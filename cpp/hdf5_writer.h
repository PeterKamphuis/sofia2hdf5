// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (hdf5_writer.h) - SoFiA to HDF5 Converter                //
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

/// @file   hdf5_writer.h
/// @author Peter Kamphuis
/// @date   29/09/2025
/// @brief  HDF5 writing functionality for sofia2hdf5 converter (header).

#ifndef HDF5_WRITER_H
#define HDF5_WRITER_H

#include <stdbool.h>
#include <hdf5.h>
#include "common.h"
#include "reader.h"

// ----------------------------------------------------------------- //
// Class 'SofiaHDF5'                                                 //
// ----------------------------------------------------------------- //
// Structure to handle HDF5 file creation and writing               //
// ----------------------------------------------------------------- //

typedef CLASS SofiaHDF5 {
    char hdf5name[MAX_PATH_LENGTH];
    char name[MAX_STRING_LENGTH];
    bool overwrite;
    
    // Data containers
    FitsFile *cube_data;
    FitsFile *mask_data;
    SofiaCatalog *catalog;
    
    // HDF5 file handle
    hid_t file_id;
    hid_t group_id;
} SofiaHDF5;

// Constructor and destructor
PUBLIC SofiaHDF5 *SofiaHDF5_new(const char *filename, const char *basename);
PUBLIC void SofiaHDF5_delete(SofiaHDF5 *self);

// Public methods
PUBLIC void SofiaHDF5_add_cube(SofiaHDF5 *self, FitsFile *cube);
PUBLIC void SofiaHDF5_add_catalog(SofiaHDF5 *self, SofiaCatalog *catalog);
PUBLIC void SofiaHDF5_add_mask(SofiaHDF5 *self, FitsFile *mask);

PUBLIC void SofiaHDF5_write_cube(SofiaHDF5 *self);
PUBLIC void SofiaHDF5_write_mask(SofiaHDF5 *self);
PUBLIC void SofiaHDF5_write_catalog(SofiaHDF5 *self);

// Private methods
PRIVATE void SofiaHDF5_write_header(SofiaHDF5 *self, hid_t group_id, const FitsFile *fits_data);

#endif