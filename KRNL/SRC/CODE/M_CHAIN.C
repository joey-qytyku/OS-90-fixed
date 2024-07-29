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

LONG API_CALL M_Alloc(
	LONG    bytes_commit,
	LONG    bytes_uncommit,
	LONG    page_bits
){
	if ( (bytes_commit == 0) || (page_bits & PG_P != 0) )
		return 0;

	P_PFD mtab = g_mm.pmat_region;

	const LONG pages_alloc = (bytes_commit+4095) & (~4095);
	const LONG pages_nocom = (bytes_uncommit+4095) & (~4095);

	// Can the request be fulfilled with the existing committed pool?
	if (g_mm.page_frames_free+pages_alloc > g_mm.page_frames_total) {
		return -1;
	}

	// If we reached here, the allocation is valid and page frames exist.
	// Now we must find free entries in the list.
	// This is somewhat slow since it requires a linear search of the
	// entire array
	LONG    already_alloced = 0;
	LONG    curr_indx = 0;
	PFD     bogus;
	P_PFD   last_descriptor = &bogus;
	LONG    last_descriptor_index = 0;

	LONG retval = 0;

	// last_descriptor_index of zero is correct for first entry.

	while (1)
	{
		// Can either be transient mapped or locked mapped,
		// zero never makes sense for a chain since unmapped
		// pages are never generated.
		if ((mtab[curr_indx].page_bits >> 9) & 0b111 == 0) {
			// Is it the first we found?
			if (unlikely(already_alloced == 0))
				retval = curr_indx;

			if (unlikely(allocated == blocks_alloc))
				break;

			// Otherwise, create the PFD normally.
			// It is assumed that this entry will be the last
			// and the uncommitted count is always stored.
			// This reduces duplicated code.

			mtab[curr_indx].page_bits       = page_bits;
			mtab[curr_indx].rel_index       = already_alloced;
			mtab[curr_indx].next            = -(pages_nocom);
			mtab[curr_indx].prev            = last_descriptor_index;
			last_descriptor->next           = curr_indx;

			// The index of the previous descriptor will not be the
			// current one on the next iteration.
			last_descriptor_index = curr_indx;

			// Pointer to previous descriptor set for next time.
			last_descriptor = &mtab[curr_indx];

			// Indicate that one more was allocated.
			already_alloced++;
		}

		// Advance to the next descriptor.
		curr_indx++;
	}

	// Report that the page frames were allocated.
	g_mm.page_frames_free -= pages_alloc;
	return 0; // RETURN THE CHAIN ID!
}

// Could I just reserve a page entry to creat some sort of allocation
// header for mappings?

STAT M_Map(PVOID base, LONG chain)
{
	PLONG const pd  = g_mm.pdir_ptr;
	PLONG const pts = g_mm.ptables_region;

	for (LONG i = 0; ; i++)
	{
		if (i <= g_mm.ptents_max) {
			// We should NOT have reached the end. Error.
			return OS_ERR;
		}

		if (pts[i] & PTE_MOD_MASK == 0) {
			//
		}
	}
}

