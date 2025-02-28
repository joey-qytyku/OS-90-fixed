#define SHARED_PRINTF_TESTING_NATIVE
#include "printf.c"


// Note: streams require callbacks to accept reads and writes. This allows
// for a correctly working stdin/stdout. I saw this in the
// standard header myself. However, it may need to be a
// special case for thread safety.

int main(void)
{
	char b[1024];

	_printf("%.4x\n", 256);
	printf("%.4x\n", 256);

	//
	// There is a difference between the DJGPP printf output and the
	// GCC/Mac one. This is not a regression or error on my part, the OTHER
	// printf is showing an abberation.
	//
	// In this case, I think DJGPP's library has a bug. It abberates from
	// a standards-compliant one.
	//
	// Should send a report.
	//
}
