# -*- coding: future_fstrings -*-
# Functions commonly used in the package

from sofia2hdf5.functions.read import read_parameter_file,read_catalog,\
    get_fitsfile
from sofia2hdf5.functions.variable_determination import get_cwd,\
    get_basename,check_catalogs,check_mask

from astropy.io import fits


import h5py
import os 
import numpy as np

def convert(cfg):
    #First we read the paramater file
    input_parameters = read_parameter_file(cfg.sofia_input)
    # get the directory
    working_directory = get_cwd(cfg,input_parameters)
    #the basename
    base_name =  get_basename(input_parameters)

      #Initialize the hfd5 file
    our_hdf5 = sofia_hdf5(f'{working_directory}{base_name}.hdf5',base_name)


    #the data of the cube
    fits_data = get_fitsfile(working_directory,input_parameters)

    print(f'We are adding the {input_parameters["input.data"]} to the file  {our_hdf5.hdf5name}') 
    our_hdf5.add_cube(fits_data)

 
    #Add the catalog, add only one no point in multiple catalogues Karma annotations are ignored
    if input_parameters['output.writekarma']:
        print(f'You have produced karm annotations but karma does not read hdf5 hence we are not adding them to the file {our_hdf5.hdf5name}')
    catalog_to_add = check_catalogs(working_directory,input_parameters)
    if catalog_to_add['add']:
        print(f'We are adding the {catalog_to_add["type"]} to the file  {our_hdf5.hdf5name}') 
        our_hdf5.add_catalog(catalog_to_add)
    mask_to_add = check_mask(working_directory,base_name,input_parameters)
    if mask_to_add['add']:
        mask = fits.open(mask_to_add['filename'])
        our_hdf5.add_mask(mask)
    args = None
    our_hdf5.write_cube()
    our_hdf5.write_mask()
    our_hdf5.write_catalog()

def _convert(val):
    if isinstance(val, str):
        return np.str_(val)
    return val

class sofia_hdf5:
    def __init__(self, name,base_name):
        self.hdf5name = name
        self.name = base_name
        self.overwrite = True

    def add_cube(self,cube):
        self.cube_data = cube[0].data
        self.cube_header = cube[0].header
        self.cube_little_endian_dtype = self.cube_data.dtype.newbyteorder('L')
        
    def add_catalog(self,catalog):
        catalog_full = read_catalog(catalog['filename'])
        self.catalog = catalog_full
        self.catalog_type = catalog['type']
        self.catalog_name = catalog['filename']
        
    def add_mask(self,mask):
        self.mask_data = mask[0].data
        self.mask_header = mask[0].header
        self.mask_little_endian_dtype = self.mask_data.dtype.newbyteorder('L')
    def write_header(self,group,header):
        because_carta_thinks_they_know_better = ['HISTORY','COMMENT']
        for key in header: 
            if not key in because_carta_thinks_they_know_better:
                if isinstance(header[key], str):
                    group.attrs.create(key, f'{header[key]}')
                elif isinstance(header[key], bool):
                    #because cannot inherit
                    if header[key]:
                        value = 1
                    else:
                        value= 0
                    group.attrs.create(key, value, dtype=np.uint8)
                else:
                    group.attrs.create(key, header[key])
        return group
    def write_cube(self):
        if os.path.isfile(self.hdf5name) and self.overwrite:
            print(f'We remove {self.hdf5name}')
            os.remove(self.hdf5name)
       
        with h5py.File(self.hdf5name, "w") as hdf5file:
            cube_group = hdf5file.require_group('0')
            cube_group = self.write_header(cube_group,self.cube_header)
            
            cube_group.create_dataset('DATA', data=self.cube_data, \
                    dtype=self.cube_little_endian_dtype, \
                    chunks= None)
            #hdf5file.close()
    def write_mask(self):
        with h5py.File(self.hdf5name, "a") as hdf5file:
            mask_group = hdf5file.require_group('1')
            mask_group = self.write_header(mask_group,self.mask_header)
            
            mask_group.create_dataset('MASK', data=self.mask_data, \
                    dtype=self.mask_little_endian_dtype, \
                    chunks= None)
    def write_catalog(self):
        with h5py.File(self.hdf5name, "a") as hdf5file:
            catalog_group = hdf5file.require_group('Catalogue')
            catalog_group.attrs.create('type', self.catalog_type)
            catalog_group.attrs.create('name', self.catalog_name)
            head_done = False
            for name in self.catalog:
                if not head_done:
                    catalog_group.create_dataset('Header',data=[x for x in self.catalog[name]])
                    head_done=True
                catalog_group.create_dataset(name,data=[self.catalog[name][x] for x in self.catalog[name]])
       