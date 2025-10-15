# main project Makefile

dummy:
	@echo "Running asm test: '$@.s'"
	@echo
	@python3 scripts/genAsm.py -t $@ -a asm
	@echo "Compiling generated C code..."
	@riscv32-unknown-linux-gnu-gcc gen-output/$@.c -o gen-output/$@

clean:
	@rm -rf gen-output