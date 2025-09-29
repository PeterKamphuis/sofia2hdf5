# SoFiA2HDF5 C Implementation

This directory contains a C translation of the Python `sofia2hdf5` program, which converts SoFiA-2 outputs to HDF5 format.

## Overview

The C implementation follows the same logic as the Python version but uses C programming paradigms consistent with the SoFiA-2 codebase style found in the `src/` directory.

## Files

### Header Files (.h)
- `common.h` - Common utilities, constants, and memory management
- `config.h` - Configuration and command-line argument handling  
- `parameter.h` - SoFiA parameter file parsing
- `reader.h` - FITS file and catalog reading functionality
- `hdf5_writer.h` - HDF5 file writing functionality
- `utils.h` - Utility functions for file paths and string manipulation

### Source Files (.c)
- `main.c` - Main program entry point and conversion orchestration
- `common.c` - Implementation of common utilities
- `config.c` - Configuration management implementation
- `parameter.c` - Parameter file parsing implementation
- `reader.c` - File reading implementations
- `hdf5_writer.c` - HDF5 writing implementations
- `utils.c` - Utility function implementations

### Build System
- `Makefile` - Build configuration
- `build.sh` - Build script with dependency checking

## Dependencies

The C implementation requires the following libraries:

1. **HDF5** - For writing HDF5 output files
   - Ubuntu/Debian: `sudo apt-get install libhdf5-dev`
   - macOS: `brew install hdf5`
   - CentOS/RHEL: `sudo yum install hdf5-devel`

2. **GCC** - C compiler
   - Ubuntu/Debian: `sudo apt-get install gcc`
   - macOS: `xcode-select --install`
   - CentOS/RHEL: `sudo yum install gcc`

Note: **CFITSIO is no longer required** - this implementation uses the same native FITS reading approach as SoFiA-2, which reads FITS files directly without external libraries.

## Building

### Using the build script (recommended):
```bash
./build.sh
```

### Manual build using Make:
```bash
make
```

### Clean build:
```bash
make clean
make
```

## Usage

The C implementation uses the same command-line interface as the Python version:

```bash
# Basic usage
./sofia2hdf5 sofia_input=your_parameter_file.par

# With additional options
./sofia2hdf5 sofia_input=cube.par general.verbose=true general.directory=/path/to/data/
```

### Command Line Options

- `sofia_input=FILE` - SoFiA parameter file (required)
- `general.directory=PATH` - Working directory
- `general.verbose=true/false` - Enable verbose output
- `general.ncpu=N` - Number of CPUs to use
- `-h, --help` - Show help message
- `-v, --version` - Show version information

## Architecture

The C implementation follows object-oriented principles using structs and function pointers, similar to the SoFiA-2 codebase:

1. **Config** - Handles command-line arguments and configuration
2. **Parameter** - Manages SoFiA parameter file parsing
3. **FitsFile** - Represents FITS file data and headers
4. **SofiaCatalog** - Manages source catalog data
5. **SofiaHDF5** - Handles HDF5 file creation and writing

## Key Features

- Memory-safe implementation with proper allocation/deallocation
- Error handling with descriptive messages
- Support for ASCII, XML, and SQL catalog formats
- FITS file reading (requires CFITSIO implementation)
- HDF5 output with proper data organization
- Command-line compatibility with Python version

## Implementation Notes

### FITS File Reading
The implementation now uses the same native FITS file reading approach as SoFiA-2, which:
- Reads FITS files directly without external dependencies
- Handles header parsing line by line following FITS standards
- Supports all standard FITS data types (8, 16, 32, 64-bit integers and 32, 64-bit floats)
- Performs automatic byte-order conversion from big-endian (FITS standard) to system endianness
- Handles BSCALE/BZERO scaling and BLANK values
- Supports 1-4 dimensional data cubes
- Includes comprehensive error checking and validation

This approach eliminates the CFITSIO dependency while maintaining full FITS compatibility.

### Catalog Parsing
The catalog parser handles:
- Column detection from header lines
- Source name extraction and cleaning
- Multi-format support (ASCII, XML, SQL)
- Parameter validation

### HDF5 Structure
The HDF5 output follows the same structure as the Python version:
```
/SoFiA/
├── DATA (main data cube)
├── <header attributes>
├── Mask/
│   ├── DATA (mask data)
│   └── <header attributes>
└── Catalogue/
    ├── <metadata attributes>
    ├── id (dataset)
    ├── name (dataset)
    └── <other column datasets>
```

## Differences from Python Version

1. **Memory Management** - Explicit allocation/deallocation required
2. **Error Handling** - Uses return codes and error_exit() instead of exceptions
3. **String Handling** - Manual string manipulation and bounds checking
4. **Data Structures** - Structs instead of classes, manual capacity management
5. **Dependencies** - Requires system libraries instead of pip packages

## Future Improvements

1. ~~Complete CFITSIO integration for FITS reading~~ ✓ **COMPLETED** - Native FITS reading implemented
2. XML parsing support for XML catalogs
3. Multi-threading support for large files
4. Progress indicators for long operations
5. Unit tests and validation suite
6. Region-based FITS file reading (subregion extraction)
7. Enhanced BSCALE/BZERO scaling for integer data types

## License

This C implementation follows the same GPL-3.0+ license as the original Python code and SoFiA-2 project.