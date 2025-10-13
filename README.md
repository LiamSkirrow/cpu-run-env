# CPU Runtime Environment
An interactive and modular runtime and debug environment for CPU cores

## Firmware build
- Start with a test ASM file, containing nothing but assembly code for the targeted architecture
- Run `make fw`, this will:
  - run a Python script to copy the contents of the asm code into a pregenerated C file with an 'inline' definition, sitting within a wrapper function.
  - The Makefile will run the riscv-gcc compiler on this C code to generate the executable
  - Refer to below instructions to splice out the relevant wrapper function, in terms of its label address/function start address
  - If this function appears at the very end of the executable file then it's very easy to figure out where it ends, and therefore splice out the function based on its start/end addresses. Otherwise, it's necessary to look at the addresses of the other functions in the file (if any others are present), and determine where the wrapper function ends
  - Save this spliced out asm machine code as a separate binary, this is the code that shall be loaded onto the soft-core.
 
## Simulation Environment
Now that the standalone machine code has been generated from the user-assembly. The CPU core can be simulated running this code.
- Compile the TB, instantiating the CPU core. The TB must initialise a socket connection.
- When the TB is ready, a Python script must start sending through the machine code/binary data of the machine instructions over the socket. Implementing a serial protocol like UART is probably not necessary, just send the raw binary data as-is.
- The Python script will complete the sending of the data, at which point it will indicate that it's complete and the TB will have loaded/need to load the raw binary data into a buffer emulating the code ROM to be read by the IFU of the CPU.
- The same socket connection will be used to issue commands, strictly from Python srcipt -> TB env. The TB will halt the CPU and will only commence execution when it receives a free-run or single-step command from the Python script. NOTE: it's probably not meaningful to collect waveform data during single-stepping mode since I'm pretty sure it'll simulate the dead-time between instructions... Unless I can find a way to only generate waveform data for the meaningful time in which the CPU is actually executing instructions and not sitting idle.

## Hardware Environment
- TODO

### Misc
To generate a text file consisting of RISCV machine code, generated from a C file (probably needing to use 'inline' assembly, with no compiler optimisation)
- riscv32-unknown-linux-gnu-gcc test.c                                 # compile the C code, generating the ELF file
- objcopy -j .text -O binary -I elf32-little a.out example_binary.txt  # generate the text file of the entirety of the user code
- nm -gC a.out | grep test_func                                        # get the address of the asm wrapper function
- riscv32-unknown-linux-gnu-readelf --hex-dump=.text a.out             # dump the .text section of the ELFs, look at the machine code starting at the address found above
