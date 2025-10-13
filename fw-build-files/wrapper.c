// C code to wrap up ASM in inline function

int main(int argc, char **argv){

	wrap();

	return 0;
}

void wrap(void){
	__asm__("li a4,4");
}
