#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

extern char *asm_v(unsigned val, char *end);

int main()
{
	// Try to test using a sentinel byte also.
	char b[16];
	for (int i = 0; i < sizeof(b); i++) {
		b[i] = '=';
	}

	char *r = asm_v(UINT_MAX, b + sizeof(b)-1);
	printf("%16u\n", UINT_MAX);
	for (int i = 0; i < sizeof b; i++)
	{
		// putchar('[');
		putchar(b[i]);
		// putchar(']');
	}

	// printf("\n[%c]", *r);
}
