// C code to wrap up ASM in inline function

void wrap(void);

int main(int argc, char **argv){

	wrap();

	return 0;
}

void wrap(void){
	__asm__ __volatile__(".WRAPSTART:\n\t"
			     "li a4,255\n\t"
			     "li a5,256\n\t");
}
