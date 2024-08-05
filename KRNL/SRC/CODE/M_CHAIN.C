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

#include <OSK/MM/mm.h>

static BOOL ChainValid(LONG c) { return g_mm.pmat_region[c].prev == 0; }

LONG M_GetChainSize(LONG chain)
{
	LONG pages = 0;
	P_PFD pfd;
	for (pfd=g_mm.pmat_region[chain]; pfd->next != NULL; pfd = pfd->next)
		pages++;

	return pages * 4096;
}

PVOID GetChainEntryPhysicalAddress(INT chain, INT index)
{

}

STAT M_SetChainPageBits(
	INT     chain,
	INT     byte_offset,
	INT     byte_count)
{
}

INT M_Alloc(
	INT    bytes_commit,
	INT    bytes_uncommit,
	INT    page_bits)
{
	if (bytes_commit == 0)
		return 0;

	const INT pages_alloc = ROUNDUP_BYTES2UNITS(bytes_commit, 4096);
	const INT pages_nocom = ROUNDUP_BYTES2UNITS(bytes_uncommit, 4096);

	// Can the physical memory be allocated?
	// The reason an int cast is used is to generate a simpler sequence
	// where a signed comparison is involved.
	// This is safe because there will never be 2,147,483,648 page
	// frames.
	if ((SIGINT)g_mm.page_frames_free - (SIGINT)pages_alloc < 0)
	{
		return -1;
	}

	// If we reached here, the allocation is valid and page frames exist.
	// Now we must find free entries in the list.
	// This is somewhat slow since it requires a linear search of the
	// entire array
	INT     already_alloced = 0;
	P_PFD   cur = g_mm.pmat_region;
	P_PFD   last_descriptor = NULL;

	INT retval = 0;

	// last_descriptor_index of zero is correct for first entry.
	// Do we need current index?
	while (1)
	{
		// Can either be transient mapped or locked mapped,
		// zero never makes sense for a chain since unmapped
		// pages are never generated.
		if ((cur->page_bits >> 9) == 0)
		{
			// Is this the first found?
			if (unlikely(already_alloced == 0))
			{
				retval = ((INT)cur + (INT)g_mm.pmat_region)/sizeof(PFD);
				cur->prev = NULL;
				// TACTICAL GOTO, INCOMING!
				goto skip_for_first_point;
			}

			if (unlikely(already_alloced == pages_alloc))
				break;

			// These should not run for the first PFD in chain
			cur->prev = last_descriptor;
			last_descriptor->next = cur;

			skip_for_first_point:

			cur->page_bits = page_bits;
			cur->rel_index = already_alloced;
			// mtab[curr_indx].next            = -(pages_nocom);

			// FOR NEXT ITER
			last_descriptor = cur;
			already_alloced++;
		}

		// Advance to the next descriptor.
		cur++;
	}

	// Report that the page frames were allocated.
	g_mm.page_frames_free -= pages_alloc;
	return 0; // RETURN THE CHAIN ID!
}

VOID DoFlushTLB_386(PVOID addr, INT bytes)
{
	ASM(
		"mov %%cr3,%%eax;"
		"mov %%eax,%%cr3;"
		:::"memory","eax"
	);
}

VOID DoFlushTLB_486(PVOID addr, INT bytes)
{
	if (bytes >= MB())
		DoFlushTLB_386();
	else
		for (INT i = 0; i < ROUNDUP_BYTES2UNITS(bytes, 4096); i++)
			ASM("invlpg %0"::"r"(addr+i):"memory");
}

VOID (DoFlushTLB*)(PVOID, INT) = DoFlashTLB_386;

VOID M_FlushTLB(PVOID addr, INT bytes) { DoFlashTLB(); }

PVOID M_ReserveMapping(LONG bytes, PINT out_bytes_alloc)
{
	PINT pts = g_mm.ptables_region;
}

STAT M_ReleaseMapping(PVOID addr)
{}

// REMEMBER, THIS DOES NOT "ALLOCATE" ADDRESSES, ONLY MAPS!
STAT M_Map(
	LONG    chain,
	PVOID   baseaddr,
	INT    start,
	INT    len,
	INT    attr)
{
	if (!ChainValid(chain) || len == 0)
		return OS_ERR;
}

VOID M_SetAttr(PVOID addr, INT bytes, INT attr)
{}
