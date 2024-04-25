# OmegaConf setups

from dataclasses import dataclass, field
import psutil
from omegaconf import MISSING
from typing import List, Optional
import os


@dataclass
class General:
    verbose: bool = True
    try:
        ncpu: int = len(psutil.Process().cpu_affinity())
    except AttributeError:
        ncpu: int = psutil.cpu_count()
    directory: str = os.getcwd()
    multiprocessing: bool = True
    #font_file: str = "/usr/share/fonts/truetype/msttcorefonts/Times_New_Roman.ttf"

@dataclass
class defaults:
    print_examples: bool = False
    configuration_file: Optional[str] = None
    general: General = General()
   