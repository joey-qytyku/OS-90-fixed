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

#include <OSK/SD/sv86.h>

// #include <OSK/MC/pio.h>

#include <OSK/DB/debug.h>
#include <OSK/s_sync.h>

MM_STRUCT g_mm;

PVOID M_VaDeref(PVOID address)
{
	// Convert address to integer
	LONG va = (LONG)address;

	// This whole thing explained:
	// - Inside we take the page directory entry of the va.
	//   - To do this, shift out the offset and the page table index
	//     so that the result is the correct index.
	// - Then the PDE has its bits masked out.
	// - The PDE is now a valid pointer to a page table
	// - PDE is converted to pointer and dereferenced
	// - Result is the PTE
	// - PTE is masked too
	// - Convert result to pointer and return.

	return (PLONG)(*((PLONG)(pdir_ptr[va>>PDE_SHIFT] & PAGE_MAP_MASK)) & PAGE_MAP_MASK);
}

#pragma noreturn(GetCmosBytePair)

extern int _bss_end;


// Memory detection:
//
// OS/90 does not need to detect memory at all since HIMEM already did.
//
static LONG GetPageFrames(VOID)
{
}

VOID M_Init(VOID)
{
}
