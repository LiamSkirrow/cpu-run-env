# main project Makefile

# TODO:
# - need to be able to tests generically, rather than replicating one block for each test name, I only
#   want one block like below
# - need to split up each test into logically distinct [firmware | simulation] sections
#   - make [test name] FW=1  # only run binary generation
#   - make [test name] SIM=1 # only run simulation without first generating binary
# - also need to run the simulate.py script as a background process...
#     - run simulate.py in the background and return its pid?
#     - run the verilator tb in some conditional block, so that when it exits (either successfully or unsuccessfully)
#       it also kills the background simulate.py process...
# - include the ability to run without debug env wrapper. Just simply simulate using Verilator, running the asm code
#   this is the only time that the trace-fst should be generated. We don't need to generate the trace if we're using the Python environment

CC=verilator
ARGS=--trace-max-array 4096 --trace-max-width 32
SRC=rtl/
TB=tb/

dummy:
	@echo "Running asm test: '$@.s'"
	@echo
	@python3 scripts/genAsm.py -t $@ -a asm
	@echo "Compiling generated C code..."
	@riscv32-unknown-linux-gnu-gcc gen-output/$@.c -o gen-output/$@
	@python3 scripts/readelf.py -e $@
	@echo
	@hd -C gen-output/$@.genbin
	@echo
	@echo ">>> Running Verilator Compilation..."
	@$(CC) -Wno-fatal --trace-fst --cc $(SRC)debug_harness.sv $(SRC)top.sv --exe $(TB)main_tb.cpp $(ARGS)
	@make -C obj_dir -f Vdebug_harness.mk Vdebug_harness
	@echo ">>> Running Verilator Executable..."
	@./obj_dir/Vdebug_harness &
	@echo ">>> Initialising Python debug environment..."
	@python3 scripts/simulate.py -t $@
	@echo

# debug:
# 	$(CC) -Wno-fatal --debug --gdb --trace-fst --cc $(SRC)debug_harness.sv $(SRC)top.sv --exe $(TB)main_tb.cpp $(ARGS)
# 	make -C obj_dir -f Vdebug_harness.mk Vdebug_harness
# 	./obj_dir/Vdebug_harness --gdb

clean:
	rm -rf gen-output
	rm -rf obj_dir
