# -*- coding: future_fstrings -*-
# Functions commonly used in the package




def write_sofia(template,name):
    with open(name,'w') as file:
        for key in template:
            if key[0] == 'E' or key [0] == 'H':
                file.write(template[key])
            else:
                file.write(f"{key} = {template[key]}\n")

write_sofia.__doc__ =f'''
NAME:
sofia
PURPOSE:
write a sofia2 dictionary into file
CATEGORY:
write_functions

INPUTS:
template = sofia template
name = name of the file to write to

OPTIONAL INPUTS:

OUTPUTS:

OPTIONAL OUTPUTS:

PROCEDURES CALLED:
Unspecified

NOTE:
'''