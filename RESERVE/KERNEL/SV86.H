/*******************************************************************************
		      Copyright (C) 2022-2024, Joey Qytyku

  This file is part of OS/90.

  OS/90 is free software. You may distribute and/or modify it under
  the terms of the GNU General Public License as published by the
  Free Software Foundation, either version two of the license or a later
  version if you choose.

  A copy of this license should be included with OS/90.
  If not, it can be found at <https://www.gnu.org/licenses/>
*******************************************************************************/

#ifndef SV86_H
#define SV86_H

#include "stdregs.h"
#include "int.h"

#define V86R_INIT { .ESP = 0 }

/*
	If the call is consumed, returns NULL. Otherwise, it returns
	the next INT call to try. Can be anything, really, but is usually the
	last in the chain.
*/
typedef PVOID (*V86HND)(PSTDREGS);

// Indicates to SV86 that there are no more procedures to run
#define SV86_RET_CONSUMED NULL

// Not recommended, but will signal an immediate reflection to SV86.
#define SV86_RET_REFLECT  ((PVOID)(1))

VOID HookINTxH(BYTE vector, V86HND hnew, V86HND *out_prev);

/*
	General purpose V86 INT service routine.
	If the stack in regparm is zero, the built in HMA SV86 stack
	is used.

	The register dump is modified after the call.

	This does NOT reflect interrupts to real DOS but can be used to call
	DOS services for DPMI-level support.

	Can be used to call a fake IRQ handler for a V86 user process. The task
	should be scheduled appropriately (exclusive tasking if needed) if
	direct hardware access is needed.

	Can be used with interrupts disabled, but not in an ISR.

	Preemption is blocked within SV86 if it actually reflects to real mode.
*/
VOID V_INTxH(BYTE vector, PSTDREGS regparm);

#endif /* SV86_H */
