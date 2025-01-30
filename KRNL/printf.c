/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2022-2024, Joey Qytyku                //
//                                                                         //
// This file is part of OS/90.                                             //
//                                                                         //
// OS/90 is free software. You may distribute and/or modify it under       //
// the terms of the GNU General Public License as published by the         //
// Free Software Foundation, either version two of the license or a later  //
// version if you chose.                                                   //
//                                                                         //
// A copy of this license should be included with OS/90.                   //
// If not, it can be found at <https://www.gnu.org/licenses/>              //
/////////////////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include "printf.h"

// This be look hard to understand, but I got it from transforming existing
// code and checking the assembler output.
//
// This prints out commas and does so fast as well.

VOID UInt32ToString2(   LONG            value,
			PUTCHAR_LIKE    func)
{
	static const int lookup[] = {
		1000000000,
		100000000,
		10000000,
		1000000,
		100000,
		10000,
		1000,
		100,
		10,
		1
	};

	static const BYTE lookup_need_comma[8] = {1,0,0,1,0,0,1};

	if (value == 0) {
		func('0');
		return;
	}

	int found_first_zero = 0;

	for (int j = 0; j < 10; j++) {
		LONG d = ((value / lookup[j]) % 10);

		// We only care about zeroes if we
		// have encountered the first non-zero
		// digit (except if value is zero).

		if (d == 0 && !found_first_zero) {
			continue;
		}
		// If a non-zero is found, all zeroes after
		// matter so first condition is not true.
		else if (d != 0) {
			found_first_zero = 1;
		}
		func('0' + d);

		if (lookup_need_comma[j]) {
		    func(',');
		}
	}
}

VOID Hex32ToString(     LONG            value,
			PUTCHAR_LIKE    func)
{
	static BYTE lookup[16] = {
		'0','1','2','3','4','5','6','7','8','9',
		'A','B','C','D','E','F'
	};

	for (int i = 7; i >= 0; i--)
		func(lookup[(value >> (i*4)) & 0xF]);
}

// Should work to make this truly compatible (except for float ofc)
void FuncPrintf(        PUTCHAR_LIKE    func,
			const char*     fmt,
			...)
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
					i++;
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

				case 'M':
					UInt32ToString2(
						va_arg(args, LONG)/(1024*1024),
						func
					);
					func('M');
				continue;

				case 'K':
					UInt32ToString2(
						va_arg(args, LONG)/1024,
						func
					);
					func('K');
				continue;

				case 's': // TODO
				{
					char *s = va_arg(args, char*);
					for (int i = 0; !s[i]; i++)
						func(s[i]);
				}
				continue;
			}
		else {
			func(fmt[i]);
		}
	}
	va_end(args);
}

VOID putE9(char c) { outb(0xE9, c); }
