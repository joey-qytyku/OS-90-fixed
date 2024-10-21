#include <stdarg.h>
#include "Z_IO.H"

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

__attribute__((cdecl))
VOID UInt32ToString(LONG value, PBYTE obuffer)
{
    LONG digit, digit_divisor;
    LONG buff_off = MAX_STR_LENGTH_OF_UINT32 - 1;
    LONG i;
    // Clear buffer by setting all chars to ascii NUL
    // so that they are not printed
    inline_memset(obuffer, 0, MAX_STR_LENGTH_OF_UINT32);
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

VOID Hex32ToString(LONG value, PBYTE obuffer)
{
	static BYTE lookup[16] = {
		'0','1','2','3','4','5','6','7','8','9',
		'A','B','C','D','E','F'
	};
	inline_memset(obuffer, ' ', 8);
	obuffer[9] = 0;
	for (int i = 0; i < 8; i++) {
		obuffer[7-i] = lookup[value >> (i*4) & 0xF];
	}
}

// Add support for aligned tabs
void FuncPrintf(VOID (*func)(char c), const char *fmt, ...)
{
	//
	// Position of cursor relative to start of where it is printed.
	// Used for tab alignment.
	//
	LONG ch_loc = 0;

	va_list args;

	char buff[16];

	va_start(args, fmt);

	for (LONG i = 0; fmt[i] != 0; i++, ch_loc++)
	{
		switch (fmt[i])
		{
			case '%':
				switch (fmt[i+1])
				{
					case '%':
						func('%');
					continue;

					case 'u':
						UInt32ToString(va_arg(args, LONG), buff);
						for (LONG i = 0; buff[i] != 0; i++)
							func(buff[i]);
						i++;
					continue;

					case 'x':
						Hex32ToString(va_arg(args, LONG), buff);
						for (LONG i = 0; buff[i] != 0; i++)
							func(buff[i]);
						ch_loc += 8;
						i++;
					continue;

					case 's':
					{
						const char *s = fmt+i;
						for (LONG i = 0; s[i] != 0; i++)
							func(s[i]);
					}
					continue;
				}
			break;

			case '\t': {
				unsigned int new_ch_loc = (ch_loc+7) & (~7);
				for (LONG i = 0; i < new_ch_loc - ch_loc; i++)
					func(' ');
			}
			break;

			default:
				func(fmt[i]);
		}
	}
	va_end(args);
}

void pc(char c)
{
	outb(0xE0, c);
}
