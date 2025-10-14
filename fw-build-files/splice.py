# this script identifies the user ASM code within the overall compiled executable, 
# and generates a new executable consisting of this code only

# The summary of how this works is as follow:
# - the asm code shall be written in a standalone, no frills *.s file in the tests/ dir
# - this Python script shall then grab those lines of asm and shall append the required \n\t chars to each line
#   copying them into the wrap() function within the skeleton C harness code.
# - this C code shall then be compiled using the standard riscv32 gcc port, and then readelf can be used
#   to determine the address of the WRAPSTART label, which points to the beginning of the user-code. Note,
#   the code is little endian so some adjusting will need to happen to bring them back into big endian form,
#   assuming that's the form that will be used for storing the instructions in simulation ROM
# - This script will splice out that chunk of user-code and simply copy it into a separate binary file, which
#   is the binary that will eventually be sent over the socket connection, to the Verilator testbench! Done.
# - NOTE: it may be handy to include yet another label at the end of the code called WRAPEND so that it's easy
#         to tell when the user-code ends... That label will be present in a [readelf -Ws a.out] call and can be 
#         used instead of relying on a dummy instruction to act as the escape sequence.
