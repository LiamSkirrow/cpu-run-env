# incorporate the functions of GNU readelf to read the ELF's symbol section
# to determine the start and end labels for the user asm (WRAPSTART/WRAPEND)
# and splice out the corresponding chunk of machine code

import readelf