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

from argparse import ArgumentParser
import ctemplate

# handle input args
parser = ArgumentParser()
parser.add_argument("-t", "--testname", required=True, type=str, help="Give the name of the test, used to lookup the relevant filename")
parser.add_argument("-a", "--asmOrC",   required=True, type=str, help="Used to determine whether the test is generated from C or asm")

args = parser.parse_args()

test_name = args.testname
asm_or_c  = args.asmOrC

test_file_path = 'tests-' + asm_or_c + '/' + test_name + ('.s' if(asm_or_c == 'asm') else '.c' )
output_gen_c_path = 'gen-output/' + test_name + '.c'

# hold the C wrapper harness code
c_wrapper_first_half  = ctemplate.c_wrapper_first_half
c_wrapper_second_half = ctemplate.c_wrapper_second_half

# add the necessary bloat overhead to be included in the C __inline_asm__ function
def addBloat(cleanAsm):
    return '"' + cleanAsm + '\\n\\t"'

# open the user-defined asm code
print('Searching for ' + asm_or_c + ' test with name: ' + test_file_path)
with open(test_file_path) as asmFile:
    for line in asmFile:
        # print(line.strip('\n'))
        line = addBloat(line.strip('\n'))
        # print('added bloat to instruction: ' + line)
        c_wrapper_first_half = c_wrapper_first_half + '\n                ' + line
    c_wrapper_total = c_wrapper_first_half + '\n               ' + c_wrapper_second_half
    # print(c_wrapper_total)

# dump generated C code output into output file, ready for compilation
with open(output_gen_c_path, 'w') as outputGenFile:
    outputGenFile.write(c_wrapper_total)



