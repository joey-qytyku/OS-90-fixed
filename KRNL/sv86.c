/*******************************************************************************
		      Copyright (C) 2022-2025, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************/

#include "sv86.h"

struct __ivt { WORD ip:16; WORD cs:16; };

__auto_type IVT = (const struct __ivt * const)0;

static HV86 v86_handlers[256];

static inline VOID Pushw(REGS PTR r, WORD v)
{
	r->SP -= 2;
	*(WORD PTR)(r->SS * 16 + r->SP) = v;
}

static inline WORD Popw(REGS PTR r)
{
	WORD v = *(WORD PTR)(r->SS*16 + r->SP);
	r->SP += 2;
	return v;
}

static inline VOID Intw(REGS PTR	r,
			WORD  		new_cs,
			WORD   		new_ip
			)
{
	Pushw(r, r->FLAGS);
	Pushw(r, r->CS);
	Pushw(r, r->IP + 2);
	r->EIP  = new_ip;
	r->CS   = new_cs;
}

static inline VOID Iretw(REGS PTR r)
{
	r->IP           = Popw(r);
	r->CS           = Popw(r);
	r->FLAGS        = Popw(r);
}

static BOOL Stub(REGS PTR r)
{
	(VOID)r;
	return 1;
}

// It is important to accurately simulate the stack because some real mode
// software modifies it intentionally.

// WARNING: goto
DWORD V86xH(BYTE v, REGS PTR r)
{
	DWORD int_caught = v;
	DWORD rval;
	DWORD level = 0;

	r->ESP = 0x8FF0+16; // This needs to go.
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
