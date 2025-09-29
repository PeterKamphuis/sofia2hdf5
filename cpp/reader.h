// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (reader.h) - SoFiA to HDF5 Converter                     //
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

/// @file   reader.h
/// @author Peter Kamphuis
/// @date   29/09/2025
/// @brief  File reading functionality for sofia2hdf5 converter (header).

#ifndef READER_H
#define READER_H

#include <stdbool.h>
#include <stdint.h>
#include "common.h"
#include "parameter.h"

// FITS file constants (from SoFiA-2)
#define FITS_HEADER_BLOCK_SIZE   2880  ///< Size of a single FITS header block (in bytes).
#define FITS_HEADER_LINE_SIZE      80  ///< Size of a single FITS header line (in bytes).
#define FITS_HEADER_LINES          36  ///< Number of lines in a single FITS header block.
#define FITS_HEADER_KEYWORD_SIZE    8  ///< Maximum size of a FITS header keyword (in bytes).
#define FITS_HEADER_KEY_SIZE       10  ///< Size of a FITS header key, including `=` assignment (in bytes).
#define FITS_HEADER_VALUE_SIZE     70  ///< Maximum size of a FITS header value (in bytes).

// ----------------------------------------------------------------- //
// Class 'FitsFile'                                                  //
// ----------------------------------------------------------------- //
// Structure to hold FITS file information                          //
// ----------------------------------------------------------------- //

typedef CLASS FitsFile {
    void *data;           // Raw data pointer (can be float, double, int8, int16, int32, int64)
    size_t nx, ny, nz;    // Dimensions
    int data_type;        // BITPIX value from FITS header
    size_t word_size;     // Size of each data element in bytes
    size_t data_size;     // Total number of data elements
    char *header;         // Raw FITS header
    size_t header_size;   // Size of header in bytes
    bool header_parsed;   // Whether header has been parsed into key-value pairs
    char **header_keys;   // Parsed header keys
    char **header_values; // Parsed header values
    size_t header_count;  // Number of header key-value pairs
} FitsFile;

// ----------------------------------------------------------------- //
// Class 'Catalog'                                                   //
// ----------------------------------------------------------------- //
// Structure to hold catalog information                            //
// ----------------------------------------------------------------- //

typedef CLASS CatalogSource {
    char name[MAX_STRING_LENGTH];
    int id;
    double x, y, z;
    double x_min, x_max, y_min, y_max, z_min, z_max;
    double ra, dec, v_app;
    double f_sum, err_f_sum;
    double err_x, err_y, err_z;
    double kin_pa, w50;
    double rms;
    int n_pix;
    double v_sofia;
} CatalogSource;

typedef CLASS SofiaCatalog {
    CatalogSource *sources;
    size_t size;
    size_t capacity;
    char type[MAX_STRING_LENGTH];
    char filename[MAX_PATH_LENGTH];
} SofiaCatalog;

// ----------------------------------------------------------------- //
// Class 'CatalogInfo'                                               //
// ----------------------------------------------------------------- //
// Structure to hold catalog file information                       //
// ----------------------------------------------------------------- //

typedef CLASS CatalogInfo {
    bool add;
    char filename[MAX_PATH_LENGTH];
    char type[MAX_STRING_LENGTH];
} CatalogInfo;

// ----------------------------------------------------------------- //
// Class 'MaskInfo'                                                  //
// ----------------------------------------------------------------- //
// Structure to hold mask file information                          //
// ----------------------------------------------------------------- //

typedef CLASS MaskInfo {
    bool add;
    char filename[MAX_PATH_LENGTH];
    char type[MAX_STRING_LENGTH];
} MaskInfo;

// Constructor and destructor functions
PUBLIC FitsFile *FitsFile_new(void);
PUBLIC void FitsFile_delete(FitsFile *self);
PUBLIC SofiaCatalog *SofiaCatalog_new(void);
PUBLIC void SofiaCatalog_delete(SofiaCatalog *self);

// FITS file reading functions
PUBLIC FitsFile *read_fits_file(const char *filename);
PUBLIC void parse_fits_header(FitsFile *self);
PUBLIC const char *get_fits_header_value(const FitsFile *self, const char *key);
PUBLIC long int get_fits_header_int(const FitsFile *self, const char *key);
PUBLIC double get_fits_header_flt(const FitsFile *self, const char *key);
PUBLIC bool get_fits_header_bool(const FitsFile *self, const char *key);

// Byte order functions
PUBLIC bool is_little_endian_system(void);
PUBLIC void swap_fits_byte_order(void *data, size_t word_size, size_t count);
// Reading functions
PUBLIC FitsFile *get_fitsfile(const char *directory, const Parameter *input_parameters);
PUBLIC SofiaCatalog *read_catalog(const char *filename);
PUBLIC SofiaCatalog *read_sofia_catalogue(const char *filename, bool xml);
PUBLIC CatalogInfo check_catalogs(const char *working_directory, const Parameter *input_parameters);
PUBLIC MaskInfo check_mask(const char *working_directory, const char *base_name, const Parameter *input_parameters);
PUBLIC void check_parameters(char **variables, int var_count, char **input_columns, int col_count);
PUBLIC char *get_source_cat_name(const char *line, char **input_columns, int *column_locations, int col_count);

#endif