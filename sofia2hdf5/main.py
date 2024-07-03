# -*- coding: future_fstrings -*-

from sofia2hdf5.config.config import setup_config
from sofia2hdf5.functions.overhead import convert
import sys
import traceback
import warnings
import sofia2hdf5
from multiprocessing import get_context,Manager

def warn_with_traceback(message, category, filename, lineno, file=None, line=None):
    log = file if hasattr(file,'write') else sys.stderr
    traceback.print_stack(file=log)
    log.write(warnings.formatwarning(message, category, filename, lineno, line))





def main():
    argv=sys.argv[1:]
    if '-v' in argv or '--version' in argv:
        print(f"This is version {sofia2hdf5.__version__} of the program.")
        sys.exit()

    if '-h' in argv or '--help' in argv:
        print('''
Use package_name in this way:

All config parameters can be set directly from the command line by setting the correct parameters, e.g:
create_package_name def_file=cube.fits error_generator=tirshaker 
''')
        sys.exit()

    cfg = setup_config(argv)
  
    # for some dumb reason pools have to be called from main
    # !!!!!!!!Starts your Main Here
    convert(cfg)



if __name__ =="__main__":
    main()
