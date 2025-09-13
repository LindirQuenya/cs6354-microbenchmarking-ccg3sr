#include <sys/cdefs.h>
#include "00_function_call.h"

void function_call_baseline(void) {}

__attribute_noinline__ void function_call_v_v(void) {
	asm ("");
	return;
}

__attribute_noinline__ int function_call_i_v(void) {
	asm ("");
	return 0;
}

__attribute_noinline__ char * function_call_p_v(void) {
	asm ("");
	return 0;
}

__attribute_noinline__ char function_call_b_v(void) {
	asm ("");
	return 0;
}
