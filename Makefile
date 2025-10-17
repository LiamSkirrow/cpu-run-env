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
	@python3 scripts/simulate.py

clean:
	@rm -rf gen-output
