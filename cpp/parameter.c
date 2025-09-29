// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (parameter.c) - SoFiA to HDF5 Converter                  //
// Copyright (C) 2025 Peter Kamphuis                                    //
// ____________________________________________________________________ //

#include "parameter.h"
#include "utils.h"
#include <ctype.h>

#define PARAMETER_INITIAL_CAPACITY 50

// ----------------------------------------------------------------- //
// Constructor and destructor                                        //
// ----------------------------------------------------------------- //

Parameter *Parameter_new(void)
{
    Parameter *self = memory_alloc(sizeof(Parameter));
    self->keys = memory_alloc(PARAMETER_INITIAL_CAPACITY * sizeof(char *));
    self->values = memory_alloc(PARAMETER_INITIAL_CAPACITY * sizeof(char *));
    self->size = 0;
    self->capacity = PARAMETER_INITIAL_CAPACITY;
    return self;
}

void Parameter_delete(Parameter *self)
{
    if (self != NULL) {
        for (size_t i = 0; i < self->size; i++) {
            memory_free(self->keys[i]);
            memory_free(self->values[i]);
        }
        memory_free(self->keys);
        memory_free(self->values);
        memory_free(self);
    }
    return;
}

// ----------------------------------------------------------------- //
// Public methods                                                    //
// ----------------------------------------------------------------- //

void Parameter_set(Parameter *self, const char *key, const char *value)
{
    check_null(self);
    check_null(key);
    check_null(value);
    
    // Check if key already exists
    size_t index = Parameter_find_index(self, key);
    if (index < self->size) {
        // Update existing key
        memory_free(self->values[index]);
        self->values[index] = string_copy(value);
        return;
    }
    
    // Add new key-value pair
    if (self->size >= self->capacity) {
        Parameter_expand_capacity(self);
    }
    
    self->keys[self->size] = string_copy(key);
    self->values[self->size] = string_copy(value);
    self->size++;
    
    return;
}

bool Parameter_exists(const Parameter *self, const char *key)
{
    check_null(self);
    check_null(key);
    
    return Parameter_find_index(self, key) < self->size;
}

const char *Parameter_get_str(const Parameter *self, const char *key)
{
    check_null(self);
    check_null(key);
    
    size_t index = Parameter_find_index(self, key);
    if (index < self->size) {
        return self->values[index];
    }
    
    return "";  // Return empty string if not found
}

bool Parameter_get_bool(const Parameter *self, const char *key)
{
    const char *value = Parameter_get_str(self, key);
    if (strlen(value) == 0) return false;
    
    char lower_value[32];
    strncpy(lower_value, value, sizeof(lower_value) - 1);
    lower_value[sizeof(lower_value) - 1] = '\0';
    
    // Convert to lowercase
    for (int i = 0; lower_value[i]; i++) {
        lower_value[i] = tolower(lower_value[i]);
    }
    
    return (strcmp(lower_value, "true") == 0 || 
            strcmp(lower_value, "yes") == 0 || 
            strcmp(lower_value, "1") == 0 ||
            strcmp(lower_value, "t") == 0);
}

void Parameter_load(Parameter *self, const char *filename)
{
    check_null(self);
    check_null(filename);
    
    if (!file_exists(filename)) {
        char error_msg[MAX_PATH_LENGTH + 100];
        snprintf(error_msg, sizeof(error_msg), "Parameter file not found: %s", filename);
        error_exit(error_msg);
    }
    
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        char error_msg[MAX_PATH_LENGTH + 100];
        snprintf(error_msg, sizeof(error_msg), "Cannot open parameter file: %s", filename);
        error_exit(error_msg);
    }
    
    char line[PARAMETER_MAX_LINE_SIZE];
    int counter = 0;
    int counter2 = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        char *trimmed = string_trim(line);
        
        // Skip empty lines
        if (strlen(trimmed) == 0) {
            char empty_key[32];
            snprintf(empty_key, sizeof(empty_key), "EMPTY%d", counter);
            Parameter_set(self, empty_key, line);
            counter++;
            continue;
        }
        
        // Skip comment lines
        if (trimmed[0] == '#') {
            char hash_key[32];
            snprintf(hash_key, sizeof(hash_key), "HASH%d", counter2);
            Parameter_set(self, hash_key, line);
            counter2++;
            continue;
        }
        
        // Parse key=value pairs
        char *equals = strchr(line, '=');
        if (equals != NULL) {
            *equals = '\0';
            char *key = string_trim(line);
            char *value = string_trim(equals + 1);
            
            // Convert key to lowercase
            for (char *p = key; *p; p++) {
                *p = tolower(*p);
            }
            
            Parameter_set(self, key, value);
        }
    }
    
    fclose(fp);
    
    // Set default values for required parameters
    Parameter_set_defaults(self);
    
    return;
}

void Parameter_set_defaults(Parameter *self)
{
    check_null(self);
    
    const char *required[] = {
        "output.writekarma", "output.directory", "output.filename",
        "output.writecatascii", "output.writecatxml", "output.writecatsql",
        "output.writenoise", "output.writefiltered", "output.writemask",
        "output.writemask2d", "output.writerawmask", "output.writemoments",
        "output.writecubelets", "output.writepv", "output.margincubelets",
        "output.thresholdmom12", "output.overwrite"
    };
    
    const int required_count = sizeof(required) / sizeof(required[0]);
    
    for (int i = 0; i < required_count; i++) {
        if (!Parameter_exists(self, required[i])) {
            Parameter_set(self, required[i], "false");
        }
    }
    
    return;
}

// ----------------------------------------------------------------- //
// Private methods                                                   //
// ----------------------------------------------------------------- //

void Parameter_expand_capacity(Parameter *self)
{
    check_null(self);
    
    self->capacity *= 2;
    self->keys = memory_realloc(self->keys, self->capacity * sizeof(char *));
    self->values = memory_realloc(self->values, self->capacity * sizeof(char *));
    
    return;
}

size_t Parameter_find_index(const Parameter *self, const char *key)
{
    check_null(self);
    check_null(key);
    
    for (size_t i = 0; i < self->size; i++) {
        if (strcmp(self->keys[i], key) == 0) {
            return i;
        }
    }
    
    return self->size;  // Return size if not found (out of range)
}