# main project Makefile

# SRC=rtl/
# TB=tb/
# CONF=config-files/
# CC=verilator
# ARGS=--trace-max-array 33 --trace-max-width 32

dummy:
	@echo "Compiling asm test: '$@.s'"
	@echo
	python3 scripts/genAsm.py -t $@ -a asm