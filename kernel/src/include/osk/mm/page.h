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

#ifndef PAGE_H
#define PAGE_H

#define MB(x) ((x) * 1048576)
#define KB(x) ((x) * 1024)

#define PDE_SHIFT 22
#define PTE_SHIFT 12

#define PAGE_SIZE (4096)

#define PG_G     (1<<8) /* Page is global, CPU avoids TLB flush */
#define PG_D     (1<<6) /* Page is dirty, was accessed */
#define PG_A     (1<<5) /* The page table/dir was used */
#define PG_PCD   (1<<4) /* Page cache disable */
#define PG_PWT   (1<<3) /* Page write though cache */
#define PG_U     (1<<2) /* User */
#define PG_RW    (1<<1) /* Read/write */
#define PG_P     (1<<0) /* Present */

//
// PG_G is only supported on Pentium and better.
//---
// Writethough caching means that the caches can be used for this page,
// but the CPU will not perform any work until it is copied back to
// memory.

// AVL is 3-bit. A number is used to indicate these mutually-exclusive
// properties.

//
// If the value is zero, the page is unused and accessing is illegal.
//
// Memory can be swapped. By default, memory is locked.
// This is only applied to pages that are present and allocated.
//
#define PTE_TRANSIENT (1<<9)

//
// Page part of uncommitted block (Awaiting commit)
// Actual page does not exist and cannot be applied to allocated page.
// Doing so may cause an error.
//
#define PTE_AWC       (2<<9)

//
// Page is hooked. This means that a page fault should be broadcasted to
// the kernel-mode handler of the current task.
//
// This is a modifier to a non-present page. If the page is present, this
// means nothing. It is used to implement IO memory region emulation,
// especially for framebuffers.
//
#define PTE_HOOK      (3<<9)

//
// Page is transient (swappable) and is currently on the disk.
// The address field of the PTE MUST be the page granular offset on the disk.
//
// USING THIS ANYWHERE OUTSIDE OF THE MEMORY MANAGER CODE IS ALWAYS WRONG.
//
#define PTE_TRANSIENT_OUT (4<<9)

//
// Collateral page. This page is elligible to be freed after calling
// a designated procedure if a chain allocation or resize cannot be comleted.
//
// There are two levels.
//
#define PTE_COLAT       (5<<9)

#endif /* PAGE_H */
