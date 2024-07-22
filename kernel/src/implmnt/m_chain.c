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

	LONG blocks_alloc = (bytes_commit+4095) & (~4095);
	LONG blocks_nocom = (bytes_uncommit+4095) & (~4095);

	// Can the request be fulfilled with the existing committed pool?
	if (g_mm.page_frames_avail+blocks_alloc > g_mm.page_frames_registered) {
		// If not, we must call the OOM handler before continuing.
		// If it returns an error, the system must crash.
		// This is unlikely since subsystems can implement their own
		// OOM hooks to kill tasks.
	}

	// If we reached here, the allocation is valid and page frames exist.
	// Now we must find free entries in the list.
	// This is somewhat slow since it requires a linear search of the
	// entire array
}
