// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (utils.h) - SoFiA to HDF5 Converter                      //
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

/// @file   utils.h
/// @author Peter Kamphuis
/// @date   29/09/2025
/// @brief  Utility functions for sofia2hdf5 converter (header).

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "common.h"
#include "parameter.h"

// Forward declarations
bool file_exists(const char *filename);

// Path and filename utilities
PUBLIC char *get_basename(const Parameter *input_parameters);
PUBLIC char *get_working_directory(const char *config_directory, const Parameter *input_parameters);

// File checking utilities
PUBLIC bool file_exists(const char *filename);
PUBLIC void ensure_directory_exists(const char *directory);

// String parsing utilities
PUBLIC void parse_key_value(const char *line, char *key, char *value);
PUBLIC char *format_path(const char *directory, const char *filename);

#endif