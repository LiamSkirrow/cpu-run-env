# CPU Runtime Environment
An interactive and modular runtime and debug environment for CPU cores.

The idea is that you can easily instantiate an RTL CPU core in the debug harness, write an asm code snippet, and simulate the program on the core whilst being given the ability to single-step and place breakpoints at various points in the program.
### Current State of Project
Currently, the RISC-V asm snippet at the path `./tests-asm/dummy.s` will be compiled using the `riscv32-unknown-elf-gcc` compiler, generating a 32-bit ELF. The `readelf.py` script will parse the ELF, creating a 'stripped down' version of the executable code, only keeping the machine code corresponding to the `./tests-asm/dummy.s` without all the compiler boilerplate. For this reason, compiling C code is not currently supported, [however is planned for the future](https://github.com/LiamSkirrow/cpu-run-env/issues/1). After the code is assembled and the simplified binary is generated, we're presented with a prompt that allows us to either `run`, `stepi` (instruction step) or `stepc` (clock cycle step) the binary on the simulated CPU core. Typing `exit` will close the socket connection between the Python env and the Verilator env, as well as saving the FST waveform file, to be opened ideally with GTKWave.

### Dependencies
- Verilator (v???)
- riscv-gnu-toolchain (specifically the `riscv32-unknown-elf-gcc` compiler since the default instantiated CPU is my 32 bit RV toy core)
- Python3, load the venv with `pip install -r requirements.txt` to load [the 3rd party Python library](https://github.com/eliben/pyelftools) that parses the ELF 

# Development Notes

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
- When running on an FPGA, there should be a top-level module that instantiates the CPU itself
- The top-level reads from a UART block, which has commands sent to it from the Python environment, and single-steps or free-runs the CPU accordingly
- The top-level should be able to snoop the CPU registers, and takeover the memory access buses in order to report back to the Python env the state of the CPU and memory. This will allow for us to monitor the internal state of the CPU as we step through instructions one-by-one.

## Feature Ideas
- Insert breakpoints at arbitrary locations within the ASM. This would be after the standalone executable has been reached. (can this simply be achieved with BREAK/EBREAK instructions???)
- Include option in config yaml to give hpaths to custom registers to be displayed in the single-step data dump display. Can monitor arbitrarily many regs, not just CPU regs
- Include option to store a Python buffer of register values in single-step mode. Can then show a matplotlib plot of this buffer. Would be useful if implementing things like software/digital filters, where you can show the input/output samples on a plot that updates with each subsequent step through clock cycles/instructions. 

### Misc
To generate a text file consisting of RISCV machine code, generated from a C file (probably needing to use 'inline' assembly, with no compiler optimisation)
- riscv32-unknown-linux-gnu-gcc test.c                                 # compile the C code, generating the ELF file
- objcopy -j .text -O binary -I elf32-little a.out example_binary.txt  # generate the text file of the entirety of the user code
- nm -gC a.out | grep test_func                                        # get the address of the asm wrapper function
- riscv32-unknown-linux-gnu-readelf --hex-dump=.text a.out             # dump the .text section of the ELFs, look at the machine code starting at the address found above
