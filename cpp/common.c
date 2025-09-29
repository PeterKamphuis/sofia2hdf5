// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (common.c) - SoFiA to HDF5 Converter                     //
// Copyright (C) 2025 Peter Kamphuis                                    //
// ____________________________________________________________________ //

#include "common.h"

// ----------------------------------------------------------------- //
// Memory allocation functions                                       //
// ----------------------------------------------------------------- //

void *memory_alloc(const size_t size)
{
    void *ptr = malloc(size);
    check_null(ptr);
    return ptr;
}

void *memory_realloc(void *ptr, const size_t size)
{
    ptr = realloc(ptr, size);
    check_null(ptr);
    return ptr;
}

void memory_free(void *ptr)
{
    if (ptr != NULL) free(ptr);
    return;
}

// ----------------------------------------------------------------- //
// String utility functions                                          //
// ----------------------------------------------------------------- //

char *string_copy(const char *str)
{
    check_null(str);
    
    size_t len = strlen(str);
    char *copy = memory_alloc(len + 1);
    strcpy(copy, str);
    return copy;
}

char *string_trim(char *str)
{
    if (str == NULL) return NULL;
    
    // Trim leading whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    
    return str;
}

bool string_starts_with(const char *str, const char *prefix)
{
    if (str == NULL || prefix == NULL) return false;
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool string_ends_with(const char *str, const char *suffix)
{
    if (str == NULL || suffix == NULL) return false;
    
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len) return false;
    
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

// ----------------------------------------------------------------- //
// Error handling functions                                          //
// ----------------------------------------------------------------- //

void check_null(const void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "Error: NULL pointer encountered\n");
        exit(ERR_NULL_PTR);
    }
    return;
}

void error_exit(const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(ERR_FAILURE);
}