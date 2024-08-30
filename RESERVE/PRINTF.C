#include <stdarg.h>
#include <string.h>
#include <alloca.h>
#include <stdio.h>

typedef unsigned long  LONG;
typedef unsigned short SHORT;
typedef unsigned char  BYTE;

typedef LONG   *PLONG;
typedef SHORT *PSHORT;
typedef BYTE  *PBYTE;

typedef int   SIGLONG;
typedef short SIGSHORT;
typedef char  SIGBYTE;

typedef const BYTE *PCSTR;

#define VOID void
#define PVOID void*
#define BOOL bool
#define MAX_STR_LENGTH_OF_UINT32 10

// Reason for -2?
VOID __cdecl Hex32ToString(LONG value, PBYTE obuffer)
{
    LONG digit, digit_divisor;
    LONG buff_off = MAX_STR_LENGTH_OF_UINT32 - 2;
    LONG i;
    // Clear buffer by setting all chars to ascii NUL
    // so that they are not printed
    memset(obuffer, 0, MAX_STR_LENGTH_OF_UINT32);
    // The following loops through each  *digit
    // an then copies them to the buffer in reverse order
    // the integer digit is then converted to a character
    // It looks complicated, but dividing by 1, 10, 100, etc.
    // is like bitwise shifting but for decimal digits, not binary.
    // Modulus is like an AND operation, getting the remainder
    // or "offset" in ComSci terms. Together, it is a sort of shift/and loop
    for (i=0, digit_divisor=1; i<MAX_STR_LENGTH_OF_UINT32; i++)
    {
        digit = (value / digit_divisor) % 10;
        obuffer[buff_off] = '0' + digit;
        digit_divisor *= 10;
        buff_off--;
    }
}
// Add support for aligned tabs
void FuncPrintf(VOID (*func)(char c), const char *fmt, ...)
{
	int i;

	//
	// Position of cursor relative to start of where it is printed.
	// Used for tab alignment.
	//
	unsigned int ch_loc = 0;

	va_list args;

	char *buff = alloca(16);

	va_start(args, fmt);

	for (i = 0; fmt[i] != 0; i++, ch_loc++) {
		switch (fmt[i])
		{
			case '%':
				switch (fmt[i])
				{
					case '%':
						func('%');
					break;

					case 'i':
						// No idea
					break;

					case 'u':
						Hex32ToString(va_arg(SIGLONG), buff);
					break;

					case 's':
					{
						int i;
						const char *s = fmt+i;
						for (i=0;s[i]!=0;i++)
							func(s[i]);
					}
					break;

				}
			break;

			case '\t': {
				unsigned int new_ch_loc += (ch_loc+7) & (~7);
				int i;
				for (i=0;i<new_ch_loc-ch_loc;i++)
					func(' ');
			}
			break;

			default:
				func(fmt[i]);
		}
	}

	va_end(args);
}

int main()
{
	BYTE *b = alloca(32);
	int i;
	Hex32ToString(8086, b);
	printf("%s\n", b);
	puts("Buffer contents:");
	for (i = 0; i < 32; i++) {
		printf("%x\n", b[i]);
	}
	return 0;
}
