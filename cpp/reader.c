// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (reader.c) - SoFiA to HDF5 Converter                     //
// Copyright (C) 2025 Peter Kamphuis                                    //
// ____________________________________________________________________ //

#include "reader.h"
#include "utils.h"
#include <ctype.h>
#include <math.h>

// For strcasecmp on some systems
#if defined(__APPLE__) || defined(__linux__)
#include <strings.h>
#endif

// Memory size constants
#define KILOBYTE       1024  ///< Size of a kilobyte (in bytes).
#define MEGABYTE    1048576  ///< Size of a megabyte (in bytes).
#define GIGABYTE 1073741824  ///< Size of a gigabyte (in bytes).

// ----------------------------------------------------------------- //
// Constructor and destructor functions                              //
// ----------------------------------------------------------------- //

FitsFile *FitsFile_new(void)
{
    FitsFile *self = memory_alloc(sizeof(FitsFile));
    self->data = NULL;
    self->nx = self->ny = self->nz = 0;
    self->data_type = 0;
    self->word_size = 0;
    self->data_size = 0;
    self->header = NULL;
    self->header_size = 0;
    self->header_parsed = false;
    self->header_keys = NULL;
    self->header_values = NULL;
    self->header_count = 0;
    return self;
}

void FitsFile_delete(FitsFile *self)
{
    if (self != NULL) {
        if (self->data) memory_free(self->data);
        if (self->header) memory_free(self->header);
        
        for (size_t i = 0; i < self->header_count; i++) {
            memory_free(self->header_keys[i]);
            memory_free(self->header_values[i]);
        }
        memory_free(self->header_keys);
        memory_free(self->header_values);
        memory_free(self);
    }
    return;
}

SofiaCatalog *SofiaCatalog_new(void)
{
    SofiaCatalog *self = memory_alloc(sizeof(SofiaCatalog));
    self->sources = NULL;
    self->size = 0;
    self->capacity = 0;
    strcpy(self->type, "");
    strcpy(self->filename, "");
    return self;
}

void SofiaCatalog_delete(SofiaCatalog *self)
{
    if (self != NULL) {
        memory_free(self->sources);
        memory_free(self);
    }
    return;
}

// ----------------------------------------------------------------- //
// Reading functions                                                 //
// ----------------------------------------------------------------- //

// ----------------------------------------------------------------- //
// FITS file reading functions                                       //
// ----------------------------------------------------------------- //

FitsFile *read_fits_file(const char *filename)
{
    check_null(filename);
    
    if (!file_exists(filename)) {
        char error_msg[MAX_PATH_LENGTH + 100];
        snprintf(error_msg, sizeof(error_msg), "FITS file not found: %s", filename);
        error_exit(error_msg);
    }
    
    printf("Opening FITS file '%s'.\n", filename);
    
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        char error_msg[MAX_PATH_LENGTH + 100];
        snprintf(error_msg, sizeof(error_msg), "Failed to open FITS file: %s", filename);
        error_exit(error_msg);
    }
    
    FitsFile *fits = FitsFile_new();
    
    // Read entire header into temporary array
    char *header = NULL;
    size_t header_size = 0;
    bool end_reached = false;
    
    while (!end_reached) {
        // (Re-)allocate memory as needed
        header = memory_realloc(header, header_size + FITS_HEADER_BLOCK_SIZE);
        
        // Read header block
        size_t bytes_read = fread(header + header_size, 1, FITS_HEADER_BLOCK_SIZE, fp);
        if (bytes_read != FITS_HEADER_BLOCK_SIZE) {
            memory_free(header);
            fclose(fp);
            FitsFile_delete(fits);
            error_exit("FITS file ended unexpectedly while reading header.");
        }
        
        // Check if we have reached the end of the header
        char *ptr = header + header_size;
        
        while (!end_reached && ptr < header + header_size + FITS_HEADER_BLOCK_SIZE) {
            if (strncmp(ptr, "END", 3) == 0) end_reached = true;
            else ptr += FITS_HEADER_LINE_SIZE;
        }
        
        header_size += FITS_HEADER_BLOCK_SIZE;
    }
    
    // Check if valid FITS file
    if (strncmp(header, "SIMPLE", 6) != 0) {
        memory_free(header);
        fclose(fp);
        FitsFile_delete(fits);
        error_exit("Missing 'SIMPLE' keyword; file does not appear to be a FITS file.");
    }
    
    // Store header in FitsFile object
    fits->header = header;
    fits->header_size = header_size;
    
    // Parse header to extract crucial elements
    parse_fits_header(fits);
    
    // Extract crucial header elements
    fits->data_type = get_fits_header_int(fits, "BITPIX");
    int dimension = get_fits_header_int(fits, "NAXIS");
    fits->nx = get_fits_header_int(fits, "NAXIS1");
    fits->ny = (dimension > 1) ? get_fits_header_int(fits, "NAXIS2") : 1;
    fits->nz = (dimension > 2) ? get_fits_header_int(fits, "NAXIS3") : 1;
    
    fits->word_size = abs(fits->data_type) / 8;  // Assumes 8 bits per byte
    fits->data_size = fits->nx * fits->ny * fits->nz;
    
    // Sanity checks
    if (!(fits->data_type == -64 || fits->data_type == -32 || fits->data_type == 8 || 
          fits->data_type == 16 || fits->data_type == 32 || fits->data_type == 64)) {
        fclose(fp);
        FitsFile_delete(fits);
        error_exit("Invalid BITPIX keyword encountered.");
    }
    
    if (dimension <= 0 || dimension > 4) {
        fclose(fp);
        FitsFile_delete(fits);
        error_exit("Only FITS files with 1-4 dimensions are supported.");
    }
    
    if (fits->data_size <= 0) {
        fclose(fp);
        FitsFile_delete(fits);
        error_exit("Invalid NAXISn keyword encountered.");
    }
    
    const double ram_needed = (double)(fits->data_size * fits->word_size);
    
    // Print status information
    printf("Reading FITS data with the following specifications:\n");
    printf("  Data type:    %d\n", fits->data_type);
    printf("  No. of axes:  %d\n", dimension);
    printf("  Axis sizes:   %zu, %zu, %zu\n", fits->nx, fits->ny, fits->nz);
    
    if (ram_needed >= GIGABYTE) {
        printf("  Memory used:  %.1f GB\n", ram_needed / GIGABYTE);
    } else if (ram_needed >= MEGABYTE) {
        printf("  Memory used:  %.1f MB\n", ram_needed / MEGABYTE);
    } else {
        printf("  Memory used:  %.1f kB\n", ram_needed / KILOBYTE);
    }
    
    // Allocate memory for data array
    fits->data = memory_alloc(fits->data_size * fits->word_size);
    
    // Read data
    size_t elements_read = fread(fits->data, fits->word_size, fits->data_size, fp);
    if (elements_read != fits->data_size) {
        fclose(fp);
        FitsFile_delete(fits);
        error_exit("FITS file ended unexpectedly while reading data.");
    }
    
    fclose(fp);
    
    // Swap byte order if required (FITS is big-endian)
    if (is_little_endian_system() && fits->word_size > 1) {
        swap_fits_byte_order(fits->data, fits->word_size, fits->data_size);
    }
    
    // Handle BSCALE and BZERO if necessary
    double bscale = get_fits_header_flt(fits, "BSCALE");
    double bzero = get_fits_header_flt(fits, "BZERO");
    
    // Set defaults if not found
    if (isnan(bscale)) bscale = 1.0;
    if (isnan(bzero)) bzero = 0.0;
    
    if (bscale != 1.0 || bzero != 0.0) {
        // Apply scaling to data - simplified version for now
        printf("Warning: BSCALE/BZERO scaling detected but not fully implemented.\n");
    }
    
    return fits;
}

void parse_fits_header(FitsFile *self)
{
    check_null(self);
    
    if (self->header_parsed || self->header == NULL || self->header_size == 0) {
        return;
    }
    
    // Count number of header lines
    size_t line_count = 0;
    for (size_t i = 0; i < self->header_size; i += FITS_HEADER_LINE_SIZE) {
        if (i + 3 < self->header_size && strncmp(self->header + i, "END", 3) == 0) {
            break;
        }
        line_count++;
    }
    
    // Allocate arrays for keys and values
    self->header_keys = memory_alloc(line_count * sizeof(char *));
    self->header_values = memory_alloc(line_count * sizeof(char *));
    self->header_count = 0;
    
    // Parse each header line
    for (size_t i = 0; i < self->header_size && self->header_count < line_count; i += FITS_HEADER_LINE_SIZE) {
        char line[FITS_HEADER_LINE_SIZE + 1];
        
        // Copy line and null-terminate
        memcpy(line, self->header + i, FITS_HEADER_LINE_SIZE);
        line[FITS_HEADER_LINE_SIZE] = '\0';
        
        // Check for END keyword
        if (strncmp(line, "END", 3) == 0) {
            break;
        }
        
        // Skip comment lines
        if (line[0] == ' ' || strncmp(line, "COMMENT", 7) == 0 || strncmp(line, "HISTORY", 7) == 0) {
            continue;
        }
        
        // Look for equals sign
        char *equals = strchr(line, '=');
        if (equals == NULL) {
            continue;  // No equals sign, skip this line
        }
        
        // Extract keyword
        size_t key_len = equals - line;
        while (key_len > 0 && line[key_len - 1] == ' ') key_len--;  // Trim trailing spaces
        
        self->header_keys[self->header_count] = memory_alloc(key_len + 1);
        strncpy(self->header_keys[self->header_count], line, key_len);
        self->header_keys[self->header_count][key_len] = '\0';
        
        // Extract value
        char *value_start = equals + 1;
        while (*value_start == ' ') value_start++;  // Skip leading spaces
        
        // Find end of value (before comment if any)
        char *value_end = strchr(value_start, '/');
        if (value_end == NULL) {
            value_end = line + FITS_HEADER_LINE_SIZE;
        }
        
        // Trim trailing spaces
        while (value_end > value_start && *(value_end - 1) == ' ') value_end--;
        
        // Remove quotes if present
        if (*value_start == '\'' && *(value_end - 1) == '\'') {
            value_start++;
            value_end--;
            // Trim spaces again
            while (*value_start == ' ') value_start++;
            while (value_end > value_start && *(value_end - 1) == ' ') value_end--;
        }
        
        size_t value_len = value_end - value_start;
        self->header_values[self->header_count] = memory_alloc(value_len + 1);
        strncpy(self->header_values[self->header_count], value_start, value_len);
        self->header_values[self->header_count][value_len] = '\0';
        
        self->header_count++;
    }
    
    self->header_parsed = true;
}

const char *get_fits_header_value(const FitsFile *self, const char *key)
{
    check_null(self);
    check_null(key);
    
    if (!self->header_parsed) return NULL;
    
    for (size_t i = 0; i < self->header_count; i++) {
        if (strcmp(self->header_keys[i], key) == 0) {
            return self->header_values[i];
        }
    }
    
    return NULL;
}

long int get_fits_header_int(const FitsFile *self, const char *key)
{
    const char *value = get_fits_header_value(self, key);
    if (value == NULL) return 0;
    
    return strtol(value, NULL, 10);
}

double get_fits_header_flt(const FitsFile *self, const char *key)
{
    const char *value = get_fits_header_value(self, key);
    if (value == NULL) return NAN;
    
    return strtod(value, NULL);
}

bool get_fits_header_bool(const FitsFile *self, const char *key)
{
    const char *value = get_fits_header_value(self, key);
    if (value == NULL) return false;
    
    return (strcmp(value, "T") == 0 || strcmp(value, "true") == 0 || strcmp(value, "True") == 0);
}

// ----------------------------------------------------------------- //
// Byte order functions                                              //
// ----------------------------------------------------------------- //

bool is_little_endian_system(void)
{
    union {
        uint32_t i;
        char c[4];
    } test = {0x01020304};
    
    return test.c[0] == 4;
}

void swap_fits_byte_order(void *data, size_t word_size, size_t count)
{
    if (word_size <= 1) return;  // No swapping needed for single bytes
    
    char *bytes = (char *)data;
    char temp;
    
    for (size_t i = 0; i < count; i++) {
        char *word = bytes + i * word_size;
        
        // Swap bytes in this word
        for (size_t j = 0; j < word_size / 2; j++) {
            temp = word[j];
            word[j] = word[word_size - 1 - j];
            word[word_size - 1 - j] = temp;
        }
    }
}

// ----------------------------------------------------------------- //
// Legacy wrapper function                                           //
// ----------------------------------------------------------------- //

FitsFile *get_fitsfile(const char *directory, const Parameter *input_parameters)
{
    check_null(directory);
    check_null(input_parameters);
    
    const char *input_data = Parameter_get_str(input_parameters, "input.data");
    if (strlen(input_data) == 0) {
        error_exit("No input data file specified");
    }
    
    char *filename = format_path(directory, input_data);
    FitsFile *fits = read_fits_file(filename);
    memory_free(filename);
    
    return fits;
}

SofiaCatalog *read_catalog(const char *filename)
{
    check_null(filename);
    
    if (!file_exists(filename)) {
        char error_msg[MAX_PATH_LENGTH + 100];
        snprintf(error_msg, sizeof(error_msg), "Catalog file not found: %s", filename);
        error_exit(error_msg);
    }
    
    // Determine if XML or ASCII based on extension
    bool is_xml = string_ends_with(filename, ".xml");
    
    return read_sofia_catalogue(filename, is_xml);
}

SofiaCatalog *read_sofia_catalogue(const char *filename, bool xml)
{
    check_null(filename);
    
    if (xml) {
        // XML reading would need a proper XML parser
        fprintf(stderr, "Warning: XML catalog reading not implemented\n");
        return SofiaCatalog_new();
    }
    
    SofiaCatalog *catalog = SofiaCatalog_new();
    strcpy(catalog->filename, filename);
    strcpy(catalog->type, "ASCII");
    
    // Variables we expect to find
    const char *required_vars[] = {
        "id", "x", "x_min", "x_max", "y", "y_min", "y_max", "z", "z_min", "z_max",
        "ra", "dec", "v_app", "f_sum", "kin_pa", "w50", "err_f_sum", 
        "err_x", "err_y", "err_z", "rms", "n_pix", "name"
    };
    const int var_count = sizeof(required_vars) / sizeof(required_vars[0]);
    
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        char error_msg[MAX_PATH_LENGTH + 100];
        snprintf(error_msg, sizeof(error_msg), "Cannot open catalog file: %s", filename);
        error_exit(error_msg);
    }
    
    char line[MAX_LINE_LENGTH];
    char **input_columns = NULL;
    int *column_locations = NULL;
    int col_count = 0;
    
    // Initial capacity for sources
    catalog->capacity = 100;
    catalog->sources = memory_alloc(catalog->capacity * sizeof(CatalogSource));
    
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        // Remove newline
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        char *trimmed = string_trim(line);
        
        if (strlen(trimmed) == 0 || strcmp(trimmed, "#") == 0) {
            continue;  // Skip empty lines and lone #
        }
        
        // Check for header line
        if (trimmed[0] == '#' && strlen(trimmed) > 1) {
            // Look for the specific column header line that contains "name" and "id"
            if (strstr(trimmed, "name") && strstr(trimmed, "id") && strstr(trimmed, "ra") && strstr(trimmed, "dec")) {
                // This is the column header line
                col_count = 0;
                
                // Count columns first - parse the header line carefully
                char temp_line[MAX_LINE_LENGTH];
                strcpy(temp_line, trimmed + 1);  // Skip the #
                
                // Split by whitespace and count non-empty tokens
                char *temp_token = strtok(temp_line, " \t");
                while (temp_token) {
                    if (strlen(temp_token) > 0) {
                        col_count++;
                    }
                    temp_token = strtok(NULL, " \t");
                }
                
                // Allocate arrays
                input_columns = memory_alloc(col_count * sizeof(char *));
                column_locations = memory_alloc(col_count * sizeof(int));
                
                // Parse columns and find their positions
                strcpy(temp_line, line);
                char *pos = temp_line + 1;  // Skip #
                int col_index = 0;
                char *token;
                
                while ((token = strtok(pos, " \t")) && col_index < col_count) {
                    input_columns[col_index] = string_copy(token);
                    
                    // Find position in original line
                    char *found = strstr(line, token);
                    if (found) {
                        column_locations[col_index] = found - line + strlen(token);
                    } else {
                        column_locations[col_index] = 0;
                    }
                    
                    col_index++;
                    pos = NULL;  // For subsequent strtok calls
                }
                
                // Check that we have all required parameters
                // This would call check_parameters() but simplified for now
                continue;
            }
        }
        
        // Data line - should start with a quoted string (source name)
        if (input_columns != NULL && col_count > 0 && trimmed[0] == '"') {
            if (catalog->size >= catalog->capacity) {
                catalog->capacity *= 2;
                catalog->sources = memory_realloc(catalog->sources, 
                    catalog->capacity * sizeof(CatalogSource));
            }
            
            CatalogSource *source = &catalog->sources[catalog->size];
            
            // Parse each column value - use a more robust approach for whitespace-separated values
            char *tokens[100];  // Max 100 columns
            int token_count = 0;
            
            // Split line into tokens, handling multiple spaces
            char temp_line[MAX_LINE_LENGTH];
            strcpy(temp_line, trimmed);
            char *start = temp_line;
            
            // Manual tokenization to handle quoted strings and multiple spaces
            while (*start && token_count < 100) {
                // Skip leading whitespace
                while (*start && (*start == ' ' || *start == '\t')) start++;
                if (!*start) break;
                
                char *end = start;
                
                // If it starts with a quote, find the closing quote
                if (*start == '"') {
                    start++;  // Skip opening quote
                    end = start;
                    while (*end && *end != '"') end++;
                    if (*end == '"') {
                        *end = '\0';  // Null-terminate before closing quote
                        tokens[token_count++] = start;
                        start = end + 1;  // Move past closing quote
                    }
                } else {
                    // Regular token - find next whitespace
                    while (*end && *end != ' ' && *end != '\t') end++;
                    if (*end) {
                        *end = '\0';
                        tokens[token_count++] = start;
                        start = end + 1;
                    } else {
                        tokens[token_count++] = start;
                        break;
                    }
                }
            }
            
            // Initialize with defaults
            source->id = catalog->size + 1;
            source->x = source->y = source->z = 0.0;
            source->x_min = source->x_max = 0.0;
            source->y_min = source->y_max = 0.0;
            source->z_min = source->z_max = 0.0;
            source->ra = source->dec = source->v_app = 0.0;
            source->f_sum = source->err_f_sum = 0.0;
            source->err_x = source->err_y = source->err_z = 0.0;
            source->kin_pa = source->w50 = 0.0;
            source->rms = 0.0;
            source->n_pix = 0;
            source->v_sofia = 0.0;
            strcpy(source->name, "");
            
            // Parse values based on column headers
            for (int i = 0; i < col_count && i < token_count; i++) {
                if (strcmp(input_columns[i], "name") == 0) {
                    // Remove quotes from name
                    char *name_token = tokens[i];
                    if (name_token[0] == '"') name_token++;
                    strcpy(source->name, name_token);
                    char *quote = strrchr(source->name, '"');
                    if (quote) *quote = '\0';
                } else if (strcmp(input_columns[i], "id") == 0) {
                    source->id = atoi(tokens[i]);
                } else if (strcmp(input_columns[i], "x") == 0) {
                    source->x = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "y") == 0) {
                    source->y = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "z") == 0) {
                    source->z = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "x_min") == 0) {
                    source->x_min = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "x_max") == 0) {
                    source->x_max = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "y_min") == 0) {
                    source->y_min = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "y_max") == 0) {
                    source->y_max = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "z_min") == 0) {
                    source->z_min = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "z_max") == 0) {
                    source->z_max = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "ra") == 0) {
                    source->ra = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "dec") == 0) {
                    source->dec = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "v_app") == 0) {
                    source->v_app = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "f_sum") == 0) {
                    source->f_sum = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "err_f_sum") == 0) {
                    source->err_f_sum = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "err_x") == 0) {
                    source->err_x = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "err_y") == 0) {
                    source->err_y = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "err_z") == 0) {
                    source->err_z = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "kin_pa") == 0) {
                    source->kin_pa = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "w50") == 0) {
                    source->w50 = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "rms") == 0) {
                    source->rms = atof(tokens[i]);
                } else if (strcmp(input_columns[i], "n_pix") == 0) {
                    source->n_pix = atoi(tokens[i]);
                }
            }
            
            catalog->size++;
        }
    }
    
    fclose(file);
    
    // Clean up
    for (int i = 0; i < col_count; i++) {
        memory_free(input_columns[i]);
    }
    memory_free(input_columns);
    memory_free(column_locations);
    
    return catalog;
}

CatalogInfo check_catalogs(const char *working_directory, const Parameter *input_parameters)
{
    check_null(working_directory);
    check_null(input_parameters);
    
    CatalogInfo catalog = {false, "", ""};
    
    char *basename = get_basename(input_parameters);
    char catalog_base[MAX_PATH_LENGTH];
    snprintf(catalog_base, sizeof(catalog_base), "%s%s_cat", working_directory, basename);
    
    // Check for ASCII catalog
    if (Parameter_get_bool(input_parameters, "output.writecatascii")) {
        snprintf(catalog.filename, sizeof(catalog.filename), "%s.txt", catalog_base);
        strcpy(catalog.type, "ASCII");
        catalog.add = true;
    }
    // Check for XML catalog
    else if (Parameter_get_bool(input_parameters, "output.writecatxml")) {
        snprintf(catalog.filename, sizeof(catalog.filename), "%s.xml", catalog_base);
        strcpy(catalog.type, "XML");
        catalog.add = true;
    }
    // Check for SQL catalog
    else if (Parameter_get_bool(input_parameters, "output.writecatsql")) {
        snprintf(catalog.filename, sizeof(catalog.filename), "%s.sql", catalog_base);
        strcpy(catalog.type, "SQL");
        catalog.add = true;
    }
    
    memory_free(basename);
    return catalog;
}

MaskInfo check_mask(const char *working_directory, const char *base_name, 
                    const Parameter *input_parameters)
{
    check_null(working_directory);
    check_null(base_name);
    check_null(input_parameters);
    
    MaskInfo mask = {false, "", ""};
    
    // Check for regular mask
    if (Parameter_get_bool(input_parameters, "output.writemask")) {
        snprintf(mask.filename, sizeof(mask.filename), "%s%s_mask.fits", 
                 working_directory, base_name);
        strcpy(mask.type, "Mask");
        mask.add = true;
    }
    // Check for 2D mask
    else if (Parameter_get_bool(input_parameters, "output.writemask2d")) {
        snprintf(mask.filename, sizeof(mask.filename), "%s%s_mask-2d.fits", 
                 working_directory, base_name);
        strcpy(mask.type, "2DMask");
        mask.add = true;
    }
    // Check for raw mask
    else if (Parameter_get_bool(input_parameters, "output.writerawmask")) {
        snprintf(mask.filename, sizeof(mask.filename), "%s%s_mask-raw.fits", 
                 working_directory, base_name);
        strcpy(mask.type, "RawMask");
        mask.add = true;
    }
    
    return mask;
}

char *get_source_cat_name(const char *line, char **input_columns, 
                          int *column_locations, int col_count)
{
    if (!line || !input_columns || !column_locations || col_count == 0) {
        return NULL;
    }
    
    // Find the 'name' column
    int name_index = -1;
    for (int i = 0; i < col_count; i++) {
        if (strcmp(input_columns[i], "name") == 0) {
            name_index = i;
            break;
        }
    }
    
    if (name_index == -1) {
        return NULL;  // No name column found
    }
    
    // Extract the name from the line
    int start = (name_index > 0) ? column_locations[name_index - 1] : 0;
    int end = column_locations[name_index];
    
    if (start >= end || start < 0 || end > (int)strlen(line)) {
        return NULL;
    }
    
    int len = end - start;
    char *name = memory_alloc(len + 1);
    strncpy(name, line + start, len);
    name[len] = '\0';
    
    // Trim and clean up the name (create a new trimmed copy)
    char *trimmed = string_trim(name);
    char *final_name = string_copy(trimmed);
    memory_free(name);
    name = final_name;
    
    // Remove quotes
    if (name[0] == '"') {
        memmove(name, name + 1, strlen(name));
    }
    char *quote = strrchr(name, '"');
    if (quote) *quote = '\0';
    
    // Replace spaces with underscores
    for (char *p = name; *p; p++) {
        if (*p == ' ') *p = '_';
    }
    
    return name;
}