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

#include <OSK/SD/enhmtx.h>

#define MB(x) ((x) * 1048576)
#define KB(x) ((x) * 1024)

static const PLONG pdir_ptr = ((PLONG)0x100000);

#define PDE_SHIFT 22
#define PTE_SHIFT 12

#define PAGE_MAP_MASK 0xFFFFF000

// Refactor to static inline?

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

/*******************************************************************************
 * If the value is zero, the page is unused and accessing is illegal.
 * Memory can be swapped. By default, memory is locked.
 * This is only applied to pages that are present and allocated.
 */
#define PTE_TRANSIENT (1<<9)

/*******************************************************************************
 * Page part of uncommitted block (Awaiting commit)
 * Physical page frame does not exist and cannot be applied to allocated page.
 * Doing so may cause an error.
**/
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
 * There are two levels.
**/
#define PTE_COLAT       (5<<9)

/*******************************************************************************
 * Every mapped page has a handler procedure associated with the
 * physical block table entry. When an access to a hooked page
 * is captured or the allocator decides to discard a collateral
 * page, this handler is called.
**/
typedef VOID (PAGE_PROC*)(
	LONG    page_table_entry,
	PVOID   virtual_address
);

typedef struct {
	PAGE_PROC       page_proc;
	SHORT           page_bits;
	SHORT           rel_index;
	SIGSHORT        next;
	SIGSHORT        prev;
}PFD,*P_PFD;
/*
 * The page bits are exactly the same as in 80x86, but some of the bits
 * are zero because they are meaningless: P, D, and A.
 ******************************************************************************/

typedef VOID (M_ALLOC_PROC*)(
	LONG bytes_commit,
	LONG bytes_uncommit,
	LONG page_flags
);
/*
 * OOMPROC takes the same exact arguments as M_Alloc.
 ******************************************************************************/

typedef VOID (LOPROC*)(VOID);
typedef VOID (SWAPPER*)(
	LONG chain_within,
	LONG local_page_index,
	LONG page_count
);

typedef struct {
	ENHMTX  mm_lock;

	LONG    page_frames_avail;
	LONG    page_frames_registered;

	LONG    chains_allocated;

	LONG    swap_file_pages_avail;
	LONG    swap_file_pages_registered;

	// Kernel physical address where it allocates. Page aligned.
	LONG    alloc_region_phys_base_intptr;

	// Where the kernel keeps page tables
	PVOID   ptables_region;

	// Where the physical memory allocation table is
	P_PFD   pmat_region;

	// Number of page table entires available. Initialized on startup.
	LONG    ptents_avail;

	// Number of page mappings that the system can handle.
	LONG    ptents_max;

	SHORT   vas_low_threshold_prcnt;
	SHORT   phys_low_threshold_prcnt;

	LOPROC  vas_low_handler;
	LOPROC  phys_low_handler;

	// Allocate
	M_ALLOC_PROC alloc_report;

	M_ALLOC_PROC oom_handler;
	SWAPPER swap_proc;
}MM_STRUCT;

// The structure is interesting, but I probably can leave some of these static.
// Maybe the alignment issues after linking are better.

extern MM_STRUCT g_mm;

#endif /* MM_H */