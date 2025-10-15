# incorporate the functions of GNU readelf to read the ELF's symbol section
# to determine the start and end labels for the user asm (WRAPSTART/WRAPEND)
# and splice out the corresponding chunk of machine code

from elftools.elf.elffile import ELFFile
from argparse import ArgumentParser

# handle input args
parser = ArgumentParser()
parser.add_argument("-e", "--exename", required=True, type=str, help="Give the name of the executable, used to lookup the relevant exe")

args = parser.parse_args()

exe_name = args.exename
exe_path = 'gen-output/' + exe_name
print(exe_path)

# elfFile = ELFFile(open(exe_path, 'r', encoding='latin-1'))
elfFile = ELFFile(open(exe_path, 'rb'))
print('Success?...')
