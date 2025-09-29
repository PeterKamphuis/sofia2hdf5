// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (common.h) - SoFiA to HDF5 Converter                     //
// Copyright (C) 2025 Peter Kamphuis                                    //
// ____________________________________________________________________ //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program. If not, see http://www.gnu.org/licenses/.   //
// ____________________________________________________________________ //

/// @file   common.h
/// @author Peter Kamphuis
/// @date   29/09/2025
/// @brief  Common functionality for sofia2hdf5 converter (header).

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SOFIA2HDF5_VERSION "1.0.0"
#define SOFIA2HDF5_CREATION_DATE "2025-09-29"

// Define object-oriented terminology
#define CLASS struct
#define PUBLIC extern
#define PRIVATE static

// Define error codes
#define ERR_SUCCESS      0
#define ERR_FAILURE      1
#define ERR_NULL_PTR     2
#define ERR_MEM_ALLOC    3
#define ERR_INDEX_RANGE  4
#define ERR_FILE_ACCESS  5
#define ERR_USER_INPUT   7

// Maximum string lengths
#define MAX_PATH_LENGTH    1024
#define MAX_STRING_LENGTH  256
#define MAX_LINE_LENGTH    1024

// Memory allocation
void *memory_alloc(const size_t size);
void *memory_realloc(void *ptr, const size_t size);
void memory_free(void *ptr);

// String utilities
char *string_copy(const char *str);
char *string_trim(char *str);
bool string_starts_with(const char *str, const char *prefix);
bool string_ends_with(const char *str, const char *suffix);

// Error handling
void check_null(const void *ptr);
void error_exit(const char *message);

#endif