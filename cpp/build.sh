#!/bin/bash

# Build script for sofia2hdf5 C implementation
# This script checks for dependencies and builds the program

echo "SoFiA2HDF5 C Build Script"
echo "========================="

# Check for required libraries
echo "Checking dependencies..."

# Check for HDF5
if ! pkg-config --exists hdf5 2>/dev/null; then
    echo "Warning: HDF5 development libraries not found via pkg-config"
    echo "Please install HDF5 development package:"
    echo "  Ubuntu/Debian: sudo apt-get install libhdf5-dev"
    echo "  macOS: brew install hdf5"
    echo "  CentOS/RHEL: sudo yum install hdf5-devel"
fi

# Check for CFITSIO
# Note: Not needed anymore as we use native FITS reading from SoFiA-2
# if ! pkg-config --exists cfitsio 2>/dev/null; then
#     echo "Warning: CFITSIO development libraries not found via pkg-config"
#     echo "Please install CFITSIO development package:"
#     echo "  Ubuntu/Debian: sudo apt-get install libcfitsio-dev"
#     echo "  macOS: brew install cfitsio" 
#     echo "  CentOS/RHEL: sudo yum install cfitsio-devel"
# fi

# Check for GCC
if ! command -v gcc &> /dev/null; then
    echo "Error: GCC compiler not found"
    echo "Please install GCC:"
    echo "  Ubuntu/Debian: sudo apt-get install gcc"
    echo "  macOS: xcode-select --install"
    echo "  CentOS/RHEL: sudo yum install gcc"
    exit 1
fi

echo "Building sofia2hdf5..."

# Build with make
if make; then
    echo ""
    echo "Build successful!"
    echo "Executable created: ./sofia2hdf5"
    echo ""
    echo "Usage: ./sofia2hdf5 sofia_input=your_parameter_file.par"
else
    echo ""
    echo "Build failed. Please check error messages above."
    exit 1
fi