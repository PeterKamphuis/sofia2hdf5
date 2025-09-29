// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (parameter.h) - SoFiA to HDF5 Converter                  //
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

/// @file   parameter.h
/// @author Peter Kamphuis
/// @date   29/09/2025
/// @brief  Parameter file handling for sofia2hdf5 converter (header).

#ifndef PARAMETER_H
#define PARAMETER_H

#include <stdbool.h>
#include "common.h"

#define PARAMETER_MAX_LINE_SIZE 1024

// ----------------------------------------------------------------- //
// Class 'Parameter'                                                 //
// ----------------------------------------------------------------- //
// Structure to handle SoFiA parameter settings                     //
// ----------------------------------------------------------------- //

typedef CLASS Parameter {
    char **keys;
    char **values;
    size_t size;
    size_t capacity;
} Parameter;

// Constructor and destructor
PUBLIC Parameter *Parameter_new(void);
PUBLIC void Parameter_delete(Parameter *self);

// Public methods
PUBLIC void Parameter_set(Parameter *self, const char *key, const char *value);
PUBLIC bool Parameter_exists(const Parameter *self, const char *key);
PUBLIC const char *Parameter_get_str(const Parameter *self, const char *key);
PUBLIC bool Parameter_get_bool(const Parameter *self, const char *key);
PUBLIC void Parameter_load(Parameter *self, const char *filename);
PUBLIC void Parameter_set_defaults(Parameter *self);

// Private methods
PRIVATE void Parameter_expand_capacity(Parameter *self);
PRIVATE size_t Parameter_find_index(const Parameter *self, const char *key);

#endif