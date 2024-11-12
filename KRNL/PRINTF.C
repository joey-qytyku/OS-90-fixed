#include <stdarg.h>
#include "Z_IO.H"

// typedef unsigned int  LONG;
// typedef unsigned short SHORT;
// typedef unsigned char  BYTE;

// typedef LONG   *PLONG;
// typedef SHORT *PSHORT;
// typedef BYTE  *PBYTE;

// typedef int   SIGLONG;
// typedef short SIGSHORT;
// typedef char  SIGBYTE;

// typedef const BYTE *PCSTR;

// #define VOID void
// #define PVOID void*
// #define BOOL bool
#define MAX_STR_LENGTH_OF_UINT32 10

// Reason for -2?

// Move func argument
VOID UInt32ToString2(LONG value, VOID (*func)(char))
{
	if (value == 0) {
		func('0');
		return;
	}

	int found_first_zero = 0;
	for (int i = 1000000000; i > 0; i /= 10) {
		LONG d = ((value / i) % 10);

		// We only care about zeroes if we
		// have encountered the first non-zero
		// digit (except if value is zero).

		if (d == 0 && !found_first_zero) {
			continue;
		}
		// If a non-zero is found, all zeroes after
		// matter so first condition is not true.
		if (d != 0) {
			found_first_zero = 1;
		}

		func('0' + d);
	}
}

VOID Hex32ToString(LONG value, VOID (*func)(char))
{
	static BYTE lookup[16] = {
		'0','1','2','3','4','5','6','7','8','9',
		'A','B','C','D','E','F'
	};

	for (int i = 7; i >= 0; i--) {
		func(lookup[value >> (i*4) & 0xF]);
	}
}

// Should work to make this truly compatible (except for float ofc)
void FuncPrintf(VOID (*func)(char c), const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	for (LONG i = 0; fmt[i] != 0; i++)
	{
		if (fmt[i] == '%')
			switch (fmt[i+1])
			{
				case '%':
					func('%');
					// Increment?
				continue;

				case 'u':
					UInt32ToString2(
						va_arg(args, LONG),
						func
					);
				continue;

				case 'x':
					Hex32ToString(
						va_arg(args, LONG),
						func
					);
					i++;
				continue;

				case 's': // TODO
				{
				}
				continue;
			}
		else {
			func(fmt[i]);
		}
	}
	va_end(args);
}

void pc(char c)
{
	outb(0xE9, c);
}

int main()
{
	return 0;
}
