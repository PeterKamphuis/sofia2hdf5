// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (utils.c) - SoFiA to HDF5 Converter                      //
// Copyright (C) 2025 Peter Kamphuis                                    //
// ____________________________________________________________________ //

#include "utils.h"
#include <libgen.h>
#include <sys/stat.h>

// ----------------------------------------------------------------- //
// Path and filename utilities                                       //
// ----------------------------------------------------------------- //

char *get_basename(const Parameter *input_parameters)
{
    check_null(input_parameters);
    
    const char *output_filename = Parameter_get_str(input_parameters, "output.filename");
    
    if (strlen(output_filename) > 0) {
        return string_copy(output_filename);
    } else {
        const char *input_data = Parameter_get_str(input_parameters, "input.data");
        if (strlen(input_data) > 0) {
            // Extract basename and remove extension
            char *temp_path = string_copy(input_data);
            char *base = basename(temp_path);
            
            // Remove extension
            char *dot = strrchr(base, '.');
            if (dot) *dot = '\0';
            
            char *result = string_copy(base);
            memory_free(temp_path);
            return result;
        }
    }
    
    return string_copy("unknown");
}

char *get_working_directory(const char *config_directory, const Parameter *input_parameters)
{
    check_null(input_parameters);
    
    const char *output_directory = Parameter_get_str(input_parameters, "output.directory");
    
    char *dir;
    if (strlen(output_directory) > 0) {
        dir = string_copy(output_directory);
    } else {
        dir = string_copy(config_directory ? config_directory : ".");
    }
    
    // Ensure directory ends with '/'
    size_t len = strlen(dir);
    if (len > 0 && dir[len - 1] != '/') {
        dir = memory_realloc(dir, len + 2);
        strcat(dir, "/");
    }
    
    return dir;
}

// ----------------------------------------------------------------- //
// File checking utilities                                           //
// ----------------------------------------------------------------- //

bool file_exists(const char *filename)
{
    if (filename == NULL) return false;
    
    struct stat st;
    return (stat(filename, &st) == 0);
}

void ensure_directory_exists(const char *directory)
{
    if (directory == NULL) return;
    
    struct stat st = {0};
    if (stat(directory, &st) == -1) {
        mkdir(directory, 0755);
    }
    return;
}

// ----------------------------------------------------------------- //
// String parsing utilities                                          //
// ----------------------------------------------------------------- //

void parse_key_value(const char *line, char *key, char *value)
{
    check_null(line);
    check_null(key);
    check_null(value);
    
    key[0] = '\0';
    value[0] = '\0';
    
    char *equals = strchr(line, '=');
    if (equals != NULL) {
        size_t key_len = equals - line;
        if (key_len < MAX_STRING_LENGTH) {
            strncpy(key, line, key_len);
            key[key_len] = '\0';
            string_trim(key);
        }
        
        strcpy(value, equals + 1);
        string_trim(value);
    }
    
    return;
}

char *format_path(const char *directory, const char *filename)
{
    check_null(directory);
    check_null(filename);
    
    size_t dir_len = strlen(directory);
    size_t file_len = strlen(filename);
    size_t total_len = dir_len + file_len + 2;  // +2 for potential '/' and '\0'
    
    char *path = memory_alloc(total_len);
    strcpy(path, directory);
    
    // Add '/' if not present
    if (dir_len > 0 && directory[dir_len - 1] != '/') {
        strcat(path, "/");
    }
    
    strcat(path, filename);
    
    return path;
}