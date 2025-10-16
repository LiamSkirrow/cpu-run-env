# incorporate the functions of GNU readelf to read the ELF's symbol section
# to determine the start and end labels for the user asm (WRAPSTART/WRAPEND)
# and splice out the corresponding chunk of machine code

from elftools.elf.elffile import ELFFile
from argparse import ArgumentParser
from pprint import pprint
import os

# handle input args
parser = ArgumentParser()
parser.add_argument("-e", "--exename", required=True, type=str, help="Give the name of the executable, used to lookup the relevant exe")

args = parser.parse_args()

exe_name = args.exename
exe_path = 'gen-output/' + exe_name
output_gen_bin_path = exe_path + '.genbin'

byteList = []

# open the ELF file for parsing
elfFile = ELFFile(open(exe_path, 'rb'))
print('ELF file belongs to architecture: ' + elfFile.get_machine_arch())
# obtain the symbol table (as an instance of class SymbolTableIndexSection(Section)), containing the addrs of WRAPSTART/WRAPEND
if(elfFile.has_section('.text')):
    textSection = elfFile.get_section_by_name('.text')
else:
    print('ELF File does not include .text section... Exiting')
    exit(0)

if(elfFile.has_section('.symtab')):
    symTabSection = elfFile.get_section_by_name('.symtab')
    # grab the Symbols we're interested in, representing the user-asm begin/end addresses
    startSymbol = symTabSection.get_symbol_by_name('.WRAPSTART')
    endSymbol   = symTabSection.get_symbol_by_name('.WRAPEND')
    # optional debugging output, the field we want is -> 'st_value'
    # pprint(vars(startSymbol.__getitem__(0)))
    # pprint(vars(endSymbol.__getitem__(0)))
    userAsmStartAddr = startSymbol.__getitem__(0).entry.st_value
    userAsmEndAddr   = endSymbol.__getitem__(0).entry.st_value
else:
    print('ELF File does not include .symtab section... Exiting')
    exit(0)
    
# we now have the user start/end addresses, time to splice out the user's machine code from the .text section

# find the starting offset of the .text section, and subtract the userAsmStartAddr/userAsmEndAddr offsets
textSectionStart = textSection.header.sh_addr

# print(textSection.data().index(userAsmStartAddr - textSectionStart))
print('Main code start addr: '  + str(textSectionStart))
print('ASM code start addr: '   + str(userAsmStartAddr))
print('ASM code end addr  : '   + str(userAsmEndAddr))
print('Adjusted start offset: ' + str(userAsmStartAddr - textSectionStart))
print('Total user code size (bytes): '  + str(userAsmEndAddr - userAsmStartAddr))

# farout... this was harder than it should've been...
# https://stackoverflow.com/questions/14267452/iterate-over-individual-bytes-in-python-3
userBinaryByteArray = [textSection.data()[i:i+1] for i in range(len(textSection.data()))]

# splice out user asm and dump in separate raw binary output file
os.makedirs(os.path.dirname(output_gen_bin_path), exist_ok=True) # make dir if doesn't already exist
with open(output_gen_bin_path, 'wb') as outputGenFile:
    print('Writing generated raw binary file to path: ' + output_gen_bin_path)
    for i in range(0, userAsmEndAddr - userAsmStartAddr):
        outputGenFile.write(userBinaryByteArray[userAsmStartAddr - textSectionStart + i])
