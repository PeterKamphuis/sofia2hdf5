// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (hdf5_writer.c) - SoFiA to HDF5 Converter                //
// Copyright (C) 2025 Peter Kamphuis                                    //
// ____________________________________________________________________ //

#include "hdf5_writer.h"
#include "utils.h"
#include <unistd.h>

// ----------------------------------------------------------------- //
// Constructor and destructor                                        //
// ----------------------------------------------------------------- //

SofiaHDF5 *SofiaHDF5_new(const char *filename, const char *basename)
{
    check_null(filename);
    check_null(basename);
    
    SofiaHDF5 *self = memory_alloc(sizeof(SofiaHDF5));
    
    strcpy(self->hdf5name, filename);
    strcpy(self->name, basename);
    self->overwrite = true;
    
    self->cube_data = NULL;
    self->mask_data = NULL;
    self->catalog = NULL;
    
    self->file_id = -1;
    self->group_id = -1;
    
    return self;
}

void SofiaHDF5_delete(SofiaHDF5 *self)
{
    if (self != NULL) {
        // Close HDF5 handles if still open
        if (self->group_id >= 0) H5Gclose(self->group_id);
        if (self->file_id >= 0) H5Fclose(self->file_id);
        
        memory_free(self);
    }
    return;
}

// ----------------------------------------------------------------- //
// Public methods                                                    //
// ----------------------------------------------------------------- //

void SofiaHDF5_add_cube(SofiaHDF5 *self, FitsFile *cube)
{
    check_null(self);
    check_null(cube);
    
    self->cube_data = cube;
    return;
}

void SofiaHDF5_add_catalog(SofiaHDF5 *self, SofiaCatalog *catalog)
{
    check_null(self);
    check_null(catalog);
    
    self->catalog = catalog;
    return;
}

void SofiaHDF5_add_mask(SofiaHDF5 *self, FitsFile *mask)
{
    check_null(self);
    check_null(mask);
    
    self->mask_data = mask;
    return;
}

void SofiaHDF5_write_cube(SofiaHDF5 *self)
{
    check_null(self);
    check_null(self->cube_data);
    
    // Remove existing file if overwrite is enabled
    if (self->overwrite && file_exists(self->hdf5name)) {
        printf("Removing existing file: %s\n", self->hdf5name);
        unlink(self->hdf5name);
    }
    
    // Create HDF5 file
    self->file_id = H5Fcreate(self->hdf5name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (self->file_id < 0) {
        char error_msg[MAX_PATH_LENGTH + 100];
        snprintf(error_msg, sizeof(error_msg), "Cannot create HDF5 file: %s", self->hdf5name);
        error_exit(error_msg);
    }
    
    // Create main SoFiA group
    self->group_id = H5Gcreate2(self->file_id, "/SoFiA", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (self->group_id < 0) {
        error_exit("Cannot create SoFiA group in HDF5 file");
    }
    
    // Write header attributes
    SofiaHDF5_write_header(self, self->group_id, self->cube_data);
    
    // Create and write data dataset
    if (self->cube_data->data && self->cube_data->nx > 0 && 
        self->cube_data->ny > 0 && self->cube_data->nz > 0) {
        
        // Define dimensions (note: HDF5 uses C ordering, which is opposite of FITS)
        hsize_t dims[3] = {self->cube_data->nz, self->cube_data->ny, self->cube_data->nx};
        hid_t space_id = H5Screate_simple(3, dims, NULL);
        
        // Determine HDF5 data type based on FITS BITPIX
        hid_t h5_datatype;
        switch (self->cube_data->data_type) {
            case 8:   h5_datatype = H5T_NATIVE_INT8; break;
            case 16:  h5_datatype = H5T_NATIVE_INT16; break;
            case 32:  h5_datatype = H5T_NATIVE_INT32; break;
            case 64:  h5_datatype = H5T_NATIVE_INT64; break;
            case -32: h5_datatype = H5T_NATIVE_FLOAT; break;
            case -64: h5_datatype = H5T_NATIVE_DOUBLE; break;
            default:  h5_datatype = H5T_NATIVE_FLOAT; break;
        }
        
        // Create dataset
        hid_t dataset_id = H5Dcreate2(self->group_id, "DATA", h5_datatype, space_id,
                                      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        
        if (dataset_id >= 0) {
            // Write data
            herr_t status = H5Dwrite(dataset_id, h5_datatype, H5S_ALL, H5S_ALL,
                                     H5P_DEFAULT, self->cube_data->data);
            
            if (status < 0) {
                fprintf(stderr, "Warning: Failed to write cube data to HDF5 file\n");
            }
            
            H5Dclose(dataset_id);
        }
        
        H5Sclose(space_id);
    }
    
    // Close groups and file
    H5Gclose(self->group_id);
    H5Fclose(self->file_id);
    
    self->group_id = -1;
    self->file_id = -1;
    
    return;
}

void SofiaHDF5_write_mask(SofiaHDF5 *self)
{
    check_null(self);
    
    if (self->mask_data == NULL) {
        return;  // No mask to write
    }
    
    // Open existing file for writing
    self->file_id = H5Fopen(self->hdf5name, H5F_ACC_RDWR, H5P_DEFAULT);
    if (self->file_id < 0) {
        char error_msg[MAX_PATH_LENGTH + 100];
        snprintf(error_msg, sizeof(error_msg), "Cannot open HDF5 file for mask writing: %s", self->hdf5name);
        error_exit(error_msg);
    }
    
    // Open SoFiA group
    hid_t sofia_group = H5Gopen2(self->file_id, "/SoFiA", H5P_DEFAULT);
    if (sofia_group < 0) {
        error_exit("Cannot open SoFiA group for mask writing");
    }
    
    // Create Mask group
    hid_t mask_group = H5Gcreate2(sofia_group, "Mask", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (mask_group < 0) {
        error_exit("Cannot create Mask group");
    }
    
    // Write mask header
    SofiaHDF5_write_header(self, mask_group, self->mask_data);
    
    // Write mask data
    if (self->mask_data->data && self->mask_data->nx > 0 && 
        self->mask_data->ny > 0 && self->mask_data->nz > 0) {
        
        hsize_t dims[3] = {self->mask_data->nz, self->mask_data->ny, self->mask_data->nx};
        hid_t space_id = H5Screate_simple(3, dims, NULL);
        
        // Determine HDF5 data type based on FITS BITPIX
        hid_t h5_datatype;
        switch (self->mask_data->data_type) {
            case 8:   h5_datatype = H5T_NATIVE_INT8; break;
            case 16:  h5_datatype = H5T_NATIVE_INT16; break;
            case 32:  h5_datatype = H5T_NATIVE_INT32; break;
            case 64:  h5_datatype = H5T_NATIVE_INT64; break;
            case -32: h5_datatype = H5T_NATIVE_FLOAT; break;
            case -64: h5_datatype = H5T_NATIVE_DOUBLE; break;
            default:  h5_datatype = H5T_NATIVE_FLOAT; break;
        }
        
        hid_t dataset_id = H5Dcreate2(mask_group, "DATA", h5_datatype, space_id,
                                      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        
        if (dataset_id >= 0) {
            H5Dwrite(dataset_id, h5_datatype, H5S_ALL, H5S_ALL,
                     H5P_DEFAULT, self->mask_data->data);
            H5Dclose(dataset_id);
        }
        
        H5Sclose(space_id);
    }
    
    H5Gclose(mask_group);
    H5Gclose(sofia_group);
    H5Fclose(self->file_id);
    
    self->file_id = -1;
    
    return;
}

void SofiaHDF5_write_catalog(SofiaHDF5 *self)
{
    check_null(self);
    
    if (self->catalog == NULL || self->catalog->size == 0) {
        return;  // No catalog to write
    }
    
    // Open existing file for writing
    self->file_id = H5Fopen(self->hdf5name, H5F_ACC_RDWR, H5P_DEFAULT);
    if (self->file_id < 0) {
        char error_msg[MAX_PATH_LENGTH + 100];
        snprintf(error_msg, sizeof(error_msg), "Cannot open HDF5 file for catalog writing: %s", self->hdf5name);
        error_exit(error_msg);
    }
    
    // Open SoFiA group
    hid_t sofia_group = H5Gopen2(self->file_id, "/SoFiA", H5P_DEFAULT);
    if (sofia_group < 0) {
        error_exit("Cannot open SoFiA group for catalog writing");
    }
    
    // Create Catalogue group (note: British spelling as in original)
    hid_t catalog_group = H5Gcreate2(sofia_group, "Catalogue", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (catalog_group < 0) {
        error_exit("Cannot create Catalogue group");
    }
    
    // Write catalog metadata as attributes
    hid_t str_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(str_type, H5T_VARIABLE);
    
    hid_t attr_space = H5Screate(H5S_SCALAR);
    
    // Type attribute
    hid_t type_attr = H5Acreate2(catalog_group, "type", str_type, attr_space,
                                 H5P_DEFAULT, H5P_DEFAULT);
    if (type_attr >= 0) {
        const char *type_str = self->catalog->type;
        H5Awrite(type_attr, str_type, &type_str);
        H5Aclose(type_attr);
    }
    
    // Name attribute
    hid_t name_attr = H5Acreate2(catalog_group, "name", str_type, attr_space,
                                 H5P_DEFAULT, H5P_DEFAULT);
    if (name_attr >= 0) {
        const char *name_str = self->catalog->filename;
        H5Awrite(name_attr, str_type, &name_str);
        H5Aclose(name_attr);
    }
    
    H5Sclose(attr_space);
    H5Tclose(str_type);
    
    // Write catalog data with all fields
    if (self->catalog->size > 0) {
        hsize_t dims[1] = {self->catalog->size};
        hid_t space_id = H5Screate_simple(1, dims, NULL);
        
        // Helper macro for writing datasets
        #define WRITE_DOUBLE_DATASET(field_name, field_value) \
        { \
            hid_t dataset = H5Dcreate2(catalog_group, field_name, H5T_NATIVE_DOUBLE, space_id, \
                                       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); \
            if (dataset >= 0) { \
                double *data = memory_alloc(self->catalog->size * sizeof(double)); \
                for (size_t i = 0; i < self->catalog->size; i++) { \
                    data[i] = self->catalog->sources[i].field_value; \
                } \
                H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data); \
                memory_free(data); \
                H5Dclose(dataset); \
            } \
        }
        
        #define WRITE_INT_DATASET(field_name, field_value) \
        { \
            hid_t dataset = H5Dcreate2(catalog_group, field_name, H5T_NATIVE_INT, space_id, \
                                       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); \
            if (dataset >= 0) { \
                int *data = memory_alloc(self->catalog->size * sizeof(int)); \
                for (size_t i = 0; i < self->catalog->size; i++) { \
                    data[i] = self->catalog->sources[i].field_value; \
                } \
                H5Dwrite(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data); \
                memory_free(data); \
                H5Dclose(dataset); \
            } \
        }
        
        // Write all the catalog fields
        WRITE_INT_DATASET("id", id);
        WRITE_DOUBLE_DATASET("x", x);
        WRITE_DOUBLE_DATASET("y", y);
        WRITE_DOUBLE_DATASET("z", z);
        WRITE_DOUBLE_DATASET("x_min", x_min);
        WRITE_DOUBLE_DATASET("x_max", x_max);
        WRITE_DOUBLE_DATASET("y_min", y_min);
        WRITE_DOUBLE_DATASET("y_max", y_max);
        WRITE_DOUBLE_DATASET("z_min", z_min);
        WRITE_DOUBLE_DATASET("z_max", z_max);
        WRITE_DOUBLE_DATASET("ra", ra);
        WRITE_DOUBLE_DATASET("dec", dec);
        WRITE_DOUBLE_DATASET("v_app", v_app);
        WRITE_DOUBLE_DATASET("f_sum", f_sum);
        WRITE_DOUBLE_DATASET("err_f_sum", err_f_sum);
        WRITE_DOUBLE_DATASET("err_x", err_x);
        WRITE_DOUBLE_DATASET("err_y", err_y);
        WRITE_DOUBLE_DATASET("err_z", err_z);
        WRITE_DOUBLE_DATASET("kin_pa", kin_pa);
        WRITE_DOUBLE_DATASET("w50", w50);
        WRITE_DOUBLE_DATASET("rms", rms);
        WRITE_INT_DATASET("n_pix", n_pix);
        
        // Name dataset (requires special string handling)
        hid_t str_space = H5Screate_simple(1, dims, NULL);
        hid_t str_dtype = H5Tcopy(H5T_C_S1);
        H5Tset_size(str_dtype, MAX_STRING_LENGTH);
        
        hid_t name_dataset = H5Dcreate2(catalog_group, "name", str_dtype, str_space,
                                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (name_dataset >= 0) {
            char *names = memory_alloc(self->catalog->size * MAX_STRING_LENGTH);
            for (size_t i = 0; i < self->catalog->size; i++) {
                strcpy(names + i * MAX_STRING_LENGTH, self->catalog->sources[i].name);
            }
            H5Dwrite(name_dataset, str_dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, names);
            memory_free(names);
            H5Dclose(name_dataset);
        }
        
        H5Tclose(str_dtype);
        H5Sclose(str_space);
        H5Sclose(space_id);
        
        #undef WRITE_DOUBLE_DATASET
        #undef WRITE_INT_DATASET
    }
    
    H5Gclose(catalog_group);
    H5Gclose(sofia_group);
    H5Fclose(self->file_id);
    
    self->file_id = -1;
    
    return;
}

// ----------------------------------------------------------------- //
// Private methods                                                   //
// ----------------------------------------------------------------- //

void SofiaHDF5_write_header(SofiaHDF5 *self, hid_t group_id, const FitsFile *fits_data)
{
    check_null(self);
    check_null(fits_data);
    
    if (!fits_data->header_parsed || fits_data->header_count == 0) {
        return;  // No header to write
    }
    
    // Create string datatype for attributes
    hid_t str_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(str_type, 256);  // 256 characters as in Python version
    
    hid_t attr_space = H5Screate(H5S_SCALAR);
    
    for (size_t i = 0; i < fits_data->header_count; i++) {
        const char *key = fits_data->header_keys[i];
        const char *value = fits_data->header_values[i];
        
        // Skip HISTORY and COMMENT as in Python version
        if (strcmp(key, "HISTORY") == 0 || strcmp(key, "COMMENT") == 0) {
            continue;
        }
        
        // Check if value is a boolean
        bool is_bool = (strcmp(value, "T") == 0 || strcmp(value, "F") == 0);
        
        if (is_bool) {
            // Write as uint8
            hid_t attr_id = H5Acreate2(group_id, key, H5T_NATIVE_UINT8, attr_space,
                                       H5P_DEFAULT, H5P_DEFAULT);
            if (attr_id >= 0) {
                uint8_t bool_val = (strcmp(value, "T") == 0) ? 1 : 0;
                H5Awrite(attr_id, H5T_NATIVE_UINT8, &bool_val);
                H5Aclose(attr_id);
            }
        } else {
            // Check if value is numeric
            char *endptr;
            double numeric_val = strtod(value, &endptr);
            
            if (*endptr == '\0' && endptr != value) {
                // It's a number
                hid_t attr_id = H5Acreate2(group_id, key, H5T_NATIVE_DOUBLE, attr_space,
                                           H5P_DEFAULT, H5P_DEFAULT);
                if (attr_id >= 0) {
                    H5Awrite(attr_id, H5T_NATIVE_DOUBLE, &numeric_val);
                    H5Aclose(attr_id);
                }
            } else {
                // It's a string
                hid_t attr_id = H5Acreate2(group_id, key, str_type, attr_space,
                                           H5P_DEFAULT, H5P_DEFAULT);
                if (attr_id >= 0) {
                    H5Awrite(attr_id, str_type, value);
                    H5Aclose(attr_id);
                }
            }
        }
    }
    
    H5Sclose(attr_space);
    H5Tclose(str_type);
    
    return;
}