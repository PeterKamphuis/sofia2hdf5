// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (config.h) - SoFiA to HDF5 Converter                     //
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

/// @file   config.h
/// @author Peter Kamphuis
/// @date   29/09/2025
/// @brief  Configuration handling for sofia2hdf5 converter (header).

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include "common.h"

// ----------------------------------------------------------------- //
// Class 'General'                                                   //
// ----------------------------------------------------------------- //
// Structure to hold general configuration settings                  //
// ----------------------------------------------------------------- //

typedef CLASS General {
    bool verbose;
    int ncpu;
    char directory[MAX_PATH_LENGTH];
    bool multiprocessing;
} General;

// ----------------------------------------------------------------- //
// Class 'Config'                                                    //
// ----------------------------------------------------------------- //
// Structure to hold all configuration parameters                    //
// ----------------------------------------------------------------- //

typedef CLASS Config {
    bool print_examples;
    char sofia_catalog[MAX_PATH_LENGTH];
    char sofia_input[MAX_PATH_LENGTH];
    char configuration_file[MAX_PATH_LENGTH];
    General general;
} Config;

// Constructor and destructor
PUBLIC Config *Config_new(void);
PUBLIC void Config_delete(Config *self);

// Public methods
PUBLIC Config *setup_config(int argc, char **argv);
PUBLIC void Config_set_defaults(Config *self);
PUBLIC void Config_print_help(void);
PUBLIC void Config_print_version(void);
PUBLIC bool Config_parse_args(Config *self, int argc, char **argv);

#endif