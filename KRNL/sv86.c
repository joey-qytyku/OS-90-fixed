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

struct __ivt { unsigned short ip:16; unsigned short cs:16; };

static struct __ivt *IVT = (struct __ivt *)0;

static HV86 v86_handlers[256];

static inline void Pushw(REGS *r, unsigned short v)
{
	r->SP -= 2;
	*(unsigned short*)(r->SS * 16 + r->SP) = v;
}

static inline unsigned short Popw(REGS *r)
{
	unsigned short v = *(unsigned short*)(r->SS*16 + r->SP);
	r->SP += 2;
	return v;
}

static inline void Intw(REGS *		r,
			unsigned short  new_cs,
			unsigned short	new_ip
			)
{
	Pushw(r, r->FLAGS);
	Pushw(r, r->CS);
	Pushw(r, r->IP + 2);
	r->EIP  = new_ip;
	r->CS   = new_cs;
}

static inline void Iretw(REGS *r)
{
	r->IP           = Popw(r);
	r->CS           = Popw(r);
	r->FLAGS        = Popw(r);
}

static int Stub(REGS *r)
{
	(void)r;
	return 1;
}

// It is important to accurately simulate the stack because some real mode
// software modifies it intentionally.

// WARNING: goto
unsigned V86xH(unsigned v, REGS *r)
{
	unsigned int_caught = v;
	unsigned rval;
	unsigned level = 0;

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

void InitV86(void)
{
	for (int i = 0; i < 256; i++) {
		v86_handlers[i] = Stub;
	}
}
