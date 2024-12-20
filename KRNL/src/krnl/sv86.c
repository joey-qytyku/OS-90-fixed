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

static LONG     stack_alloc = 0xFFFF0000;
static PVOID    stacks_base = (PVOID)0x105000;

static const LONG stack_size = 1024;

static LONG AllocateV86Stack(VOID)
{
	if (stack_alloc == 0xFFFFFFFF) {
		// No stack available
		return (LONG)-1;
	}
	else {
		register int index __asm__("eax") = 0;
		__asm__ volatile ("bsfl %0,%1"
			:"=r"(index)
			:"b"(stack_alloc)
		);
		return index;
	}
}

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

static inline VOID Intw(PREGS   r,
			SHORT   new_cs,
			SHORT   new_ip)
{
	Pushw(r, r->FLAGS);
	Pushw(r, r->CS);
	Pushw(r, r->IP + 2);
	r->EIP  = new_ip;
	r->CS   = new_cs;
}

static inline VOID Iretw(PREGS r)
{
	r->IP           = Popw(r);
	r->CS           = Popw(r);
	r->FLAGS        = Popw(r);
}

// It is important to accurately simulate the stack because some real mode
// software modifies it intentionally.

// WARNING: goto
LONG INTxH(BYTE v, PREGS r)
{
	LONG int_caught = v;
	LONG rval;
	LONG level = 0;

	r->ESP = 0x8FF0+16;
	r->SS  = 0xFFFF;

	DoInt:	Intw(r, IVT[int_caught].cs, IVT[int_caught].ip);
		level++;

	Resume:	int_caught = EnterV86(r);

	if (int_caught == 0xFFFFFFFF) {
		level--;
		Iretw(r);
		goto *(level==0 ? &&End : &&Resume);
	}
	else {
		goto DoInt;
	}
	End:	return r->EAX & 0xFFFF;
}

VOID InitV86(VOID)
{
	for (int i = 0; i < 256; i++) {
		v86_handlers[i] = Stub;
	}
}
