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

#include "sv86.h"
#include "printf.h"

/*

Here is how it all works.

There is an array of handlers for each V86 INT vector. This is not necessary
but is faster.

There is a chain of hooks. They return-chain by calling the previous one
and either calling the previous one or returning to "eat" the request.
The return value of each handler can be 1 to indicate (HANDLED) or 0 to
indicate (REFLECT).

>>> THIS IS WRONG ACTUALLY

There is a stub handler that automatically routes the interrupt to real mode
in that special case.

*/

// Stub handler? Yes.
static HV86 v86_handlers[256];

static BOOL Stub(PREGS r)
{
	(VOID)r;
	return 1;
}

static inline VOID Pushw(PREGS r, SHORT v)
{
	r->SP -= 2;
	*(PSHORT)(r->SS * 16 + r->SP) = v;
}

static inline SHORT Popw(PREGS r)
{
	SHORT v = *(PSHORT)(r->SS*16 + r->SP);
	r->SP += 2;
	return v;
}

static inline VOID Intw(PREGS r, SHORT new_cs, SHORT new_ip)
{
	Pushw(r, r->FLAGS);
	Pushw(r, r->CS);
	Pushw(r, r->IP);
	r->IP   = new_ip;
	r->CS   = new_cs;
}

static inline VOID Iretw(PREGS r)
{
	r->IP           = Popw(r);
	r->CS           = Popw(r);
	r->FLAGS        = Popw(r);
}

LONG V86xH(BYTE v, PREGS r)
{
	LONG int_caught = v;
	LONG rval;
	LONG level = 0;

	while (1) {
		Intw(r, IVT[int_caught].cs, IVT[int_caught].ip);
		int_caught = EnterV86(r);
		if (int_caught == 0xFFFFFFFF) {
			level--;
			if (level == 0) {
				return r->EAX & 0xFFFF;
			}
		}
	}
}

VOID InitV86(VOID)
{
	for (int i = 0; i < 256; i++)
		v86_handlers[i] = Stub;
}
