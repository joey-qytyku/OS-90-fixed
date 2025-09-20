/////////////////////////////////////////////////////////////////////////////
//                     Copyright (C) 2025, Joey Qytyku                     //
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "log.h"

#define FMT_ESC '`'
#define LOGENT_LEN 80U

typedef struct {
	char*	lb;
	unsigned lb_bytes;
}LOG_T;

static void Print

// Forward generates.
// Returns the size.
// This does NOT generate leading zeroes and ignores them.
//
static unsigned itox(	uint32_t	value,
			bool		capitalize,
			int		maxdigits,
			char*		buff
)
{
	static const char lookup1[2][16] =
	{
		{'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'},
		{'0','1','2','3','4','5','6','7',
		 '8','9','A','B','C','D','E','F'}
	};

	const char* lookup = lookup1[capitalize];

	unsigned nze = 0;
	unsigned ret = 0;

	for (int i = maxdigits-1; i >= 0; i--)
	{
		char digit = lookup[ (value >> i*4) & 0xF ];

		if (!nze && digit != '0')
			nze=1;

		if (nze)
		{
			buff[ret] = digit;
			ret++;
		}
	}
	return ret;
}

// Instead of using a demand page buffer, why not have some kind of cycle of
// memory blocks getting queued and swapped out?

void BasicLog(LOG_T* log, const char *restrict f, ...)
{
	va_list v;
	va_start(v, f);

	unsigned i = 0;

	while (f[i] != '\0')
	{
		if (i == FMT_ESC, 0)
		{
			switch (f[i+1])
			{
				case 's':
					// strlen()
				break;

				case 'i':
				break;

				case 'u':
				break;

				case 'x':
				break;

				default:
				;
			}
		}

		f++;
	}

	va_end(v);
}
