#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

extern char * asm_v(unsigned val, char *end);

int main()
{
	// Try to test using a sentinel byte also.
	char b[16];
	for (int i = 0; i < sizeof(b); i++) {
		b[i] = '=';
	}

	char *c = asm_v( 0, b+sizeof(b)-1);

	for (int i = 0; i < sizeof b; i++)
	{
		putchar(b[i]);
	}
	putchar('\n');
	putchar(*c);
}
