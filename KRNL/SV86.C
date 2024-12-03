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
	// FuncPrintf(putE9, "Pushed %x\n", v);
}

static inline SHORT Popw(PREGS r)
{
	SHORT v = *(PSHORT)(r->SS*16 + r->SP);
	r->SP += 2;
	// FuncPrintf(putE9, "Popped %x\n", v);
	return v;
}

static inline VOID Intw(PREGS   r,
			SHORT   new_cs,
			SHORT   new_ip)
{
	Pushw(r, r->FLAGS);
	Pushw(r, r->CS);
	Pushw(r, r->IP + 2);
	r->EIP   = new_ip;
	r->CS   = new_cs;
	// FuncPrintf(putE9, "\n");
}

static inline VOID Iretw(PREGS r)
{
	r->IP           = Popw(r);
	r->CS           = Popw(r);
	r->FLAGS        = Popw(r);
	// FuncPrintf(putE9, "\n");
}

// It is important o accurately simulate the stack because some real mode
// software modifies it intentionally.

LONG INTxH(BYTE v, PREGS r)
{
	LONG int_caught = v;
	LONG rval;
	LONG level = 0;


	DoInt:	Intw(r, IVT[int_caught].cs, IVT[int_caught].ip);
		level++;

	Resume:	int_caught = EnterV86(r);

	if (int_caught == 0xFFFFFFFF) {
		level--;
		Iretw(r);
		if (level == 0) {
			goto End;
		}
		else {
			// Resume execution without pushing to the stack.
			goto Resume;
		}
	}
	else {
		goto DoInt;
	}
	End:	return r->EAX & 0xFFFF;
}

VOID InitV86(VOID)
{
	for (int i = 0; i < 256; i++)
		v86_handlers[i] = Stub;
}
