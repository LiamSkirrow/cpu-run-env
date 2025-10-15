# Python multiline strings to include the wrapper C code to wrap our inline user-assembly

c_wrapper_first_half = """
// C code to wrap up ASM in inline function. THIS FILE IS GENERATED! DO NOT MODIFY!
void wrap(void);
int main(int argc, char **argv){
	wrap();
	return 0;
}
void wrap(void){
	__asm__ __volatile__(
		".WRAPSTART:\\n\\t" """

c_wrapper_second_half = """ ".WRAPEND\\n\\t");
}"""
