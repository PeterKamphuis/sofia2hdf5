# -*- coding: future_fstrings -*-
# Functions commonly used in the package
from sofia2hdf5.support.errors import RunTimeError
import os

from astropy.io import fits



def check_parameters(Variables,input_columns):
     # check that we found all parameters
    velocity ='v_app'
    for value in Variables:
        trig = False
        if value.lower() in input_columns:
            continue
        elif value.lower() == 'v_app':
            if 'v_rad' in input_columns:
                Variables[Variables.index('v_app')]='v_rad'
                velocity = 'v_rad'
                continue
            elif  'v_opt' in input_columns:
                Variables[Variables.index('v_app')]='v_opt'
                velocity = 'v_opt'
                continue
            else:
                trig = True    
        else:
            trig = True

        if trig:
           raise InputError(f'''SOFIA_CATALOGUE: We cannot find the required column for {value} in the sofia catalogue.
{"":8s}SOFIA_CATALOGUE: This can happen because a) you have tampered with the sofiainput.txt file in the Support directory,
{"":8s}SOFIA_CATALOGUE: b) you are using an updated version of SoFiA2.
''')
    return velocity    
check_parameters.__doc__ =f'''
 NAME:
    check_parameters(Variables,input_columns)

 PURPOSE:
    check wether all  variables are in the sofia catalogue
 
 CATEGORY:
    read_functions

 INPUTS:
    Configuration = Standard FAT configuration
    Variable = Reguired variable
    input_columns = found columns
    

 OPTIONAL INPUTS:



 OUTPUTS:
    velocity = the velocity found in the catalogue
 OPTIONAL OUTPUTS:

 PROCEDURES CALLED:
    Unspecified

 NOTE:

'''



def get_fitsfile(cwd,input_parameters):
    filename = f'{cwd}{input_parameters["input.data"]}'
    cube = fits.open(filename)
    return cube


def get_source_cat_name(line,input_columns,column_locations):
    '''Read out the source name from the catalogue input line'''
    if input_columns.index('name')-1 < 0.:
        start = 0
    else:
        start = column_locations[input_columns.index('name')-1]
    end = column_locations[input_columns.index('name')]
    name = line[start:end].strip()
    name = name.strip('"')
    name = '_'.join(name.split())
    return name



def read_catalog(catalog):
    if os.path.splitext(catalog)[1] == 'xml':
        catalog = read_sofia_catalogue(catalog,xml=True)
    else: 
        catalog = read_sofia_catalogue(catalog)
    return catalog

def read_parameter_file(filename=None):
  
    if filename is None:
        raise RunTimeError(f'read_input_parameter does not allow a default for filename=')

    with open(filename,'r') as tmp:
            template = tmp.readlines()
    result = {}
    counter = 0
    counter2 = 0
    # Separate the keyword names
    for line in template:
        key = str(line.split('=')[0].strip())
        if key == '':
            result[f"EMPTY{counter}"] = line
            counter += 1
        elif key[0] == '#':
            result[f"HASH{counter2}"] = line
            counter2 += 1
        else:
            result[key.lower()] = str(line.split('=')[1].strip())
    required = ['output.writeKarma', 'output.directory','output.filename',\
                'output.writeCatASCII','output.writeCatXML',\
                'output.writeCatSQL','output.writeNoise',\
                'output.writeFiltered','output.writeMask',\
                'output.writeMask2d','output.writeRawMask',\
                'output.writeMoments','output.writeCubelets',\
                'output.writePV','output.writeKarma',\
                'output.marginCubelets', 'output.thresholdMom12',
                'output.overwrite']
    for key in required:
        if not key.lower() in result:
            result[key.lower()] = False
    return result
read_parameter_file.__doc__ ='''
 NAME:
     read_input_parameters
 PURPOSE:
    Read the sofia2 input file 

 CATEGORY:
    read_functions

 INPUTS:

 OPTIONAL INPUTS:


 OUTPUTS:
    result = dictionary with the read file

 OPTIONAL OUTPUTS:

 PROCEDURES CALLED:
    split, strip, open

 NOTE:
'''



def read_sofia_catalogue(filename, variables = None,xml=False):
    '''Read a specified sofia table'''
    if variables is None:
        variables =['id','x','x_min','x_max','y','y_min','y_max','z','z_min',\
                    'z_max','ra','dec','v_app','f_sum','kin_pa','w50',\
                    'err_f_sum','err_x','err_y','err_z','rms','n_pix','name']
    sources = {}
    with open(filename) as tmp:
        for line in tmp.readlines():
            tmp =line.split()
            if line.strip() == '' or line.strip() == '#':
                pass
            elif tmp[0] == '#' and len(tmp) > 1:
                if tmp[1].strip().lower() in ['name','id']:
                    # get the present columns
                    input_columns  = [x.strip() for x in tmp[1:]]
                    #determin their location in the line
                    column_locations = []
                    for col in input_columns:
                        column_locations.append(line.find(col)+len(col))
                    # check that we found all parameters
                    velocity = check_parameters(variables,input_columns) 
                    
            else:
                name = get_source_cat_name(line,input_columns,column_locations)
                sources[name] = {}
                for col in variables:
                    if col == 'name':
                        sources[name][col] = name
                    else:
                        if input_columns.index(col) == 0:
                            start = 0
                        else:
                            start = column_locations[input_columns.index(col)-1]
                        end = column_locations[input_columns.index(col)]
                        sources[name][col] = line[start:end].strip() 
                sources[name]['v_sofia'] = sources[name][velocity]    
    return sources