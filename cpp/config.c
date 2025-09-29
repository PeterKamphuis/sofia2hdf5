// ____________________________________________________________________ //
//                                                                      //
// sofia2hdf5 (config.c) - SoFiA to HDF5 Converter                     //
// Copyright (C) 2025 Peter Kamphuis                                    //
// ____________________________________________________________________ //

#include "config.h"
#include <unistd.h>

// ----------------------------------------------------------------- //
// Constructor and destructor                                        //
// ----------------------------------------------------------------- //

Config *Config_new(void)
{
    Config *self = memory_alloc(sizeof(Config));
    Config_set_defaults(self);
    return self;
}

void Config_delete(Config *self)
{
    if (self != NULL) {
        memory_free(self);
    }
    return;
}

// ----------------------------------------------------------------- //
// Public methods                                                    //
// ----------------------------------------------------------------- //

Config *setup_config(int argc, char **argv)
{
    Config *cfg = Config_new();
    
    // Parse command line arguments
    if (!Config_parse_args(cfg, argc, argv)) {
        Config_delete(cfg);
        return NULL;
    }
    
    // Get number of CPUs if not set
    if (cfg->general.ncpu <= 0) {
        cfg->general.ncpu = sysconf(_SC_NPROCESSORS_ONLN) - 1;
        if (cfg->general.ncpu <= 0) cfg->general.ncpu = 1;
    }
    
    // Ensure directory ends with '/'
    size_t len = strlen(cfg->general.directory);
    if (len > 0 && cfg->general.directory[len - 1] != '/') {
        if (len < MAX_PATH_LENGTH - 1) {
            strcat(cfg->general.directory, "/");
        }
    }
    
    // Check if sofia_input is provided
    if (strlen(cfg->sofia_input) == 0) {
        printf("You have to provide the input to the sofia run: ");
        if (fgets(cfg->sofia_input, MAX_PATH_LENGTH, stdin) != NULL) {
            // Remove newline if present
            char *newline = strchr(cfg->sofia_input, '\n');
            if (newline) *newline = '\0';
        }
    }
    
    return cfg;
}

void Config_set_defaults(Config *self)
{
    check_null(self);
    
    self->print_examples = false;
    strcpy(self->sofia_catalog, "");
    strcpy(self->sofia_input, "");
    strcpy(self->configuration_file, "");
    
    // Set general defaults
    self->general.verbose = true;
    self->general.ncpu = 0; // Will be determined later
    getcwd(self->general.directory, MAX_PATH_LENGTH);
    self->general.multiprocessing = true;
    
    return;
}

void Config_print_help(void)
{
    printf("\nUse sofia2hdf5 in this way:\n\n");
    printf("All config parameters can be set directly from the command line by setting the correct parameters, e.g:\n");
    printf("sofia2hdf5 sofia_input=cube.par\n\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n");
    printf("  --verbose      Enable verbose output\n");
    printf("  --ncpu=N       Set number of CPUs to use\n");
    printf("  --directory=D  Set working directory\n");
    printf("\n");
}

void Config_print_version(void)
{
    printf("This is version %s of sofia2hdf5.\n", SOFIA2HDF5_VERSION);
    printf("Created on %s\n", SOFIA2HDF5_CREATION_DATE);
}

bool Config_parse_args(Config *self, int argc, char **argv)
{
    check_null(self);
    
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            Config_print_help();
            return false;
        }
        else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
            Config_print_version();
            return false;
        }
        else if (string_starts_with(arg, "sofia_input=")) {
            strcpy(self->sofia_input, arg + 12);
        }
        else if (string_starts_with(arg, "sofia_catalog=")) {
            strcpy(self->sofia_catalog, arg + 14);
        }
        else if (string_starts_with(arg, "configuration_file=")) {
            strcpy(self->configuration_file, arg + 19);
        }
        else if (string_starts_with(arg, "general.directory=")) {
            strcpy(self->general.directory, arg + 18);
        }
        else if (string_starts_with(arg, "general.ncpu=")) {
            self->general.ncpu = atoi(arg + 13);
        }
        else if (strcmp(arg, "--verbose") == 0) {
            self->general.verbose = true;
        }
        else if (strcmp(arg, "print_examples=true") == 0) {
            self->print_examples = true;
        }
        else {
            // Check if it's a key=value pair and ignore for now
            if (strchr(arg, '=') == NULL) {
                fprintf(stderr, "Unknown argument: %s\n", arg);
                return false;
            }
        }
    }
    
    return true;
}