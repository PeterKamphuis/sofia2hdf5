# -*- coding: future_fstrings -*-
# Functions commonly used in the package

import os


def check_catalogs(cwd,input_parameters):
    catalog = {'add':False,'filename': '', 'type':''}
    basename = f'{get_basename(input_parameters)}_cat'
    if input_parameters['output.writeCatASCII'.lower()]:
        catalog['filename'] = f'{cwd}{basename}.txt'
        catalog['type'] = 'ASCII'
        catalog['add'] = True
    elif input_parameters['output.writeCatXML'.lower()]:
        catalog['filename'] = f'{cwd}{basename}.xml'
        catalog['type'] = 'XML'
        catalog['add'] = True
    elif input_parameters['output.writeCatSQL'.lower()]:
        catalog['filename'] = f'{cwd}{basename}.sql'
        catalog['type'] = 'SQL'
        catalog['add'] = True
    return catalog


def check_mask(cwd,base_name,input_parameters):
    mask_to_add  = {'add':False,'filename': '', 'type':''}
    if input_parameters['output.writeMask'.lower()]:
        mask_to_add['filename'] = f'{cwd}{base_name}_mask.fits'
        mask_to_add['type'] = 'Mask'
        mask_to_add['add'] = True
    elif input_parameters['output.writeMask2d'.lower()]:
        mask_to_add['filename'] = f'{cwd}{base_name}_mask-2d.fits'
        mask_to_add['type'] = '2DMask'
        mask_to_add['add'] = True
    elif input_parameters['output.writeRawMask'.lower()]:
        mask_to_add['filename'] = f'{cwd}{base_name}_mask-raw.fits'
        mask_to_add['type'] = 'RawMask'
        mask_to_add['add'] = True
    return mask_to_add



def get_basename(input_parameters):
    if input_parameters['output.filename'] != '':
        return input_parameters['output.filename'] 
    else:
        return os.path.basename(os.path.splitext(input_parameters['input.data'])[0])


def get_cwd(cfg,input_parameters):
    if input_parameters['output.directory'] != '':
        dir = input_parameters['output.directory']
    else:
        dir = cfg.general.directory
    if dir[-1] != '/':
        dir = f'{dir}/'
    return dir

