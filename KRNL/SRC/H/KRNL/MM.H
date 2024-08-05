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

#ifndef MM_H
#define MM_H

#include <OSK/SD/SYNC.H>

#define MB(x) ((x) * 1048576)
#define KB(x) ((x) * 1024)

static const PLONG pdir_ptr = ((PLONG)0x100000);

#define ALIGN_VAL(V, A) ((V+(A-1)) & (-(A))

// This is a macro. We want this to be inlined as much as possible.
#define ROUNDUP_BYTES2UNITS(V, A) ( (V+(A)) & (-(A)) )

#define PDE_SHIFT 22
#define PTE_SHIFT 12

#define PAGE_MAP_MASK 0xFFFFF000

#define PAGE_SIZE (4096)

#define PG_G     (1<<8) /* Page is global, CPU avoids TLB flush */
#define PG_D     (1<<6) /* Page is dirty, was accessed */
#define PG_A     (1<<5) /* The page table/dir was used */
#define PG_PCD   (1<<4) /* Page cache disable */
#define PG_PWT   (1<<3) /* Page write though cache */
#define PG_U     (1<<2) /* User */
#define PG_RW    (1<<1) /* Read/write */
#define PG_P     (1<<0) /* Present */

/*
PG_G is only supported on Pentium and better.

Writethough means that any writes to memory go "through" the cache, which
means that RAM is always accessed on writes. This is essentially "read cache"
since it only uses the cache when reading.

AVL is 3-bit. An ordinal number is used to indicate these mutually-exclusive
properties.
*/

//
// If the ENTIRE page (and certainly the modifier) are zero, the page is
// not allocated.
//

/*******************************************************************************
 * Memory can be swapped. By default, memory is locked.
 * This is only applied to pages that are present and allocated.
 */
#define PTE_TRANSIENT (1<<9)

////////////////////////////////////////////////////////////////////////////////
// Page part of uncommitted block (Awaiting commit)
// Physical page frame does not exist and cannot be applied to allocated page.
// Doing so may cause an error.
//
// The address field of the PTE is a chain ID.
//
#define PTE_AWC       (2<<9)

/*******************************************************************************
 * Page is hooked. This means that a page fault should be broadcasted to
 * the kernel-mode handler of the current task.
 *
 * This is a modifier to a non-present page. If the page is present, this
 * means nothing. It is used to implement IO memory region emulation,
 * especially for framebuffers.
**/
#define PTE_HOOK      (3<<9)

/*******************************************************************************
 * Page is transient (swappable) and is currently on the disk.
 * The address field of the PTE MUST be the page granular offset on the disk.
 *
 * USING THIS ANYWHERE OUTSIDE OF THE MEMORY MANAGER CODE IS ALWAYS WRONG.
**/
#define PTE_TRANSIENT_OUT (4<<9)

/*******************************************************************************
 * Collateral page. This page is elligible to be freed after calling
 * a designated procedure if a chain allocation or resize cannot be comleted.
 *
 * Implied to be locked and hence mutually exclusive.
**/
#define PTE_COLAT       (5<<9)

/*******************************************************************************
 * Every mapped page has a handler procedure associated with the
 * physical block table entry. When an access to a hooked page
 * is captured or the allocator decides to discard a collateral
 * page, this handler is called.
**/

// This indicates the start of a virtual address space header. It precedes
// the actual mapped region and contains back and front links.
//
// There are two not-present pages used. The first is the back link.
// The second is the front link.
//
// 20-bit address field is simply an index for the page tables.
//
#define PTE_VAHDR       (6<<9)

// Why not add a BSS page type?

#define PTE_MOD_MASK    (0b111<<9)

typedef VOID (PAGE_PROC*)(
	LONG    page_table_entry,
	PVOID   virtual_address
);

#define HIGHEST_CHAIN_ID ((1<<20)-1)

typedef _Packed struct {
	PAGE_PROC       proc;
	PVOID           next;
	PVOID           prev;
	SHORT           rel_index;
}PFD,*P_PFD;

// Separate public and private?

typedef struct {
	ENHMTX  mm_lock;

	LONG    page_frames_free;
	LONG    page_frames_total;

	LONG    chains_allocated;

	LONG    swap_file_pages_free;
	LONG    swap_file_pages_total;


	// Number of page table entires available. Initialized on startup.
	LONG    ptents_free;

	// Number of page mappings that the system can handle.
	LONG    ptents_max;

	SHORT   vas_low_threshold_prcnt;
	SHORT   phys_low_threshold_prcnt;

	LOPROC  vas_low_handler;
	LOPROC  phys_low_handler;

	//////////////////////////////////////////

	// Where the kernel keeps page tables
	PLONG   ptables_region;

	// Where the physical memory allocation table is
	P_PFD   pmat_region;

	// Kernel physical address where it allocates. Page aligned.
	LONG    alloc_region_phys_base_intptr;
}MM_STRUCT;

// The structure is interesting, but I probably can leave some of these static.
// Maybe the alignment issues after linking are better.

extern MM_STRUCT g_mm;

#endif /* MM_H */