// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (main.c) - SoFiA to HDF5 Converter                       //
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

/// @file   main.c
/// @author Peter Kamphuis
/// @date   29/09/2025
/// @brief  Main program for sofia2hdf5 converter.

#include "common.h"
#include "config.h"
#include "parameter.h"
#include "reader.h"
#include "hdf5_writer.h"
#include "utils.h"

// ----------------------------------------------------------------- //
// Function prototypes                                               //
// ----------------------------------------------------------------- //

int convert(Config *cfg);

// ----------------------------------------------------------------- //
// Main function                                                     //
// ----------------------------------------------------------------- //

int main(int argc, char **argv)
{
    // Setup configuration from command line arguments
    Config *cfg = setup_config(argc, argv);
    if (cfg == NULL) {
        return ERR_SUCCESS;  // Help or version was printed
    }
    
    // Perform the conversion
    int result = convert(cfg);
    
    // Cleanup
    Config_delete(cfg);
    
    return result;
}

// ----------------------------------------------------------------- //
// Main conversion function                                          //
// ----------------------------------------------------------------- //

int convert(Config *cfg)
{
    check_null(cfg);
    
    if (cfg->general.verbose) {
        printf("Starting SoFiA to HDF5 conversion...\n");
        printf("Sofia input file: %s\n", cfg->sofia_input);
        printf("Working directory: %s\n", cfg->general.directory);
    }
    
    // Read the parameter file
    Parameter *input_parameters = Parameter_new();
    Parameter_load(input_parameters, cfg->sofia_input);
    
    // Get working directory and base name
    char *working_directory = get_working_directory(cfg->general.directory, input_parameters);
    char *base_name = get_basename(input_parameters);
    
    if (cfg->general.verbose) {
        printf("Working directory: %s\n", working_directory);
        printf("Base name: %s\n", base_name);
    }
    
    // Initialize the HDF5 file
    char hdf5_filename[MAX_PATH_LENGTH];
    snprintf(hdf5_filename, sizeof(hdf5_filename), "%s%s.hdf5", working_directory, base_name);
    
    SofiaHDF5 *our_hdf5 = SofiaHDF5_new(hdf5_filename, base_name);
    
    // Read the FITS data cube
    if (cfg->general.verbose) {
        const char *input_data = Parameter_get_str(input_parameters, "input.data");
        printf("Reading FITS file: %s\n", input_data);
        printf("Adding data to HDF5 file: %s\n", hdf5_filename);
    }
    
    FitsFile *fits_data = get_fitsfile(cfg->general.directory, input_parameters);
    SofiaHDF5_add_cube(our_hdf5, fits_data);
    
    // Check for catalog
    CatalogInfo catalog_to_add = check_catalogs(working_directory, input_parameters);
    if (catalog_to_add.add) {
        if (cfg->general.verbose) {
            printf("Adding %s catalog to HDF5 file: %s\n", catalog_to_add.type, catalog_to_add.filename);
        }
        
        if (file_exists(catalog_to_add.filename)) {
            SofiaCatalog *catalog = read_catalog(catalog_to_add.filename);
            SofiaHDF5_add_catalog(our_hdf5, catalog);
        } else {
            printf("Warning: Catalog file not found: %s\n", catalog_to_add.filename);
        }
    }
    
    // Check for mask
    MaskInfo mask_to_add = check_mask(working_directory, base_name, input_parameters);
    if (mask_to_add.add) {
        if (cfg->general.verbose) {
            printf("Adding %s to HDF5 file: %s\n", mask_to_add.type, mask_to_add.filename);
        }
        
        if (file_exists(mask_to_add.filename)) {
            FitsFile *mask = get_fitsfile("", input_parameters);  // Placeholder - would need proper mask reading
            SofiaHDF5_add_mask(our_hdf5, mask);
        } else {
            printf("Warning: Mask file not found: %s\n", mask_to_add.filename);
        }
    }
    
    // Check for Karma annotations warning
    if (Parameter_get_bool(input_parameters, "output.writekarma")) {
        printf("Warning: You have produced Karma annotations but Karma does not read HDF5, "
               "hence we are not adding them to the file %s\n", hdf5_filename);
    }
    
    // Write all data to HDF5 file
    if (cfg->general.verbose) {
        printf("Writing data to HDF5 file...\n");
    }
    
    SofiaHDF5_write_cube(our_hdf5);
    
    if (our_hdf5->mask_data) {
        SofiaHDF5_write_mask(our_hdf5);
    }
    
    if (our_hdf5->catalog) {
        SofiaHDF5_write_catalog(our_hdf5);
    }
    
    if (cfg->general.verbose) {
        printf("Conversion completed successfully!\n");
        printf("Output file: %s\n", hdf5_filename);
    }
    
    // Cleanup
    Parameter_delete(input_parameters);
    if (our_hdf5->catalog) SofiaCatalog_delete(our_hdf5->catalog);
    SofiaHDF5_delete(our_hdf5);
    FitsFile_delete(fits_data);
    memory_free(working_directory);
    memory_free(base_name);
    
    return ERR_SUCCESS;
}