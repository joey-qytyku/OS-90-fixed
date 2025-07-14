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

#include "m_core.h"
#include "sv86.h"

// This is a macro. We want this to be inlined as much as possible.
#define ROUNDUP_BYTES2UNITS(V, A) ( (V+(A)) & (-(A)) )

static const PLONG pdir_ptr = ((PLONG)0x100000);

// First few entries correspond with the output of M_GetInfo.

// Also move this into global variables.
struct mmctrl {
	LONG    emf; // Extended memory free
	LONG    emp; // Extended memory total pages

	LONG    dbf; // Disk-backed free
	LONG    dbp; // Disk-backed total pages

	LONG    vsf; // Virtual pages free
	LONG    vsp; // Virtual pages total

	PMTE    pmt; // Pointer to memory table
	LONG    fxp; // Index of first page of extended memory
};

static struct mmctrl mm;

//
// Memory depends on interrupts, scheduler, and V86 working.
//
VOID M_Init(VOID)
{
	// Enable write protection in ring-0 if available.

	// Access the interrupt vectors used to store the memory region sizes.
	// There are two.
}

PVOID API M_XAlloc(     PVOID   force_addr,
			LONG    commit,
			LONG    uncommit,
			LONG    bits)
{

}

LONG API M_GetInfo(BYTE class)
{
}

PVOID API M_CMAlloc(LONG bytes)
{
	// Disable preemption here?
}

VOID API M_CMFree(PVOID addr)
{

}
