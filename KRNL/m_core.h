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

#ifndef MM_H
#define MM_H

#define MB(x) ((x) * 1048576U)
#define KB(x) ((x) * 1024U)

#define ALIGN_VAL(V, A) ((V+(A-1)) & (-(A))

#define PDE_SHIFT 22
#define PTE_SHIFT 12

#define PAGE_MAP_MASK 0xFFFFF000

#define PAGE_SIZE (4096)

/*
	These are page bit masks.
*/

#define PG_PAT   (1<<12) /* Use PAT */
#define PG_G     (1<<8) /* Page is global, CPU avoids TLB flush */
#define PG_D     (1<<6) /* Page is dirty, was accessed */
#define PG_A     (1<<5) /* The page table/dir was used */
#define PG_PCD   (1<<4) /* Page cache disable */
#define PG_PWT   (1<<3) /* Page write though cache */
#define PG_U     (1<<2) /* User */
#define PG_RW    (1<<1) /* Read/write */
#define PG_P     (1<<0) /* Present */

#define _pset(PG) (PG<<16 | PG)
#define _pclr(PG) (PG)

// Example: _pset(PG_G) | _pclr(PG_RW)

// Write combine using PAT entry zero
#define PO_WC (_pclr(PG_PWT) | _pclr(PG_PCD) | _pset(PG_PAT))

// Writethrough cache makes no sense if cache is disabled.
// Writethough means that any writes to memory go "through" the cache, which
// means that RAM is always accessed on writes. This is essentially "read cache"
// since it only uses the cache when reading.
#define PO_WT (_pset(PG_PWT) | _pclr(PG_PCD))

#define PO_RO()

#define PO_RW


#endif /* MM_H */
