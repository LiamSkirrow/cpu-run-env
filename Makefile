# main project Makefile

dummy:
	@echo "Compiling asm test: '$@.s'"
	@echo
	@python3 scripts/genAsm.py -t $@ -a asm

clean:
	@rm -rf gen-output