////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef MMDEFS_H
#define MMDEFS_H

#include <Type.h>

// Maybe rename
#define MB(x) (x * 1048576)
#define KB(x) (x * 1024)

#define MEM_BLOCK_SIZE KB(4)

#define INVALID_CHAIN 0xFFFFFFFF

#define PDE_SHIFT 22
#define PTE_SHIFT 12
#define PDE_MASK 0b1111111111
#define PTE_MASK 0b1111111111

#define PAGE_SIZE (4096)

#define PG_D     (1<<6) /* Page is dirty, was accessed */
#define PG_A     (1<<5) /* The page table/dir was used */
#define PG_PCD   (1<<4) /* Page cache disable */
#define PG_PWT   (1<<3) /* Page write though cache */
#define PG_U     (1<<2) /* User */
#define PG_RW    (1<<1) /* Read/write */
#define PG_P     (1<<0) /* Present */

// The unused bits are used for OS/90 to implement uncommitted memory and
// locking. These only apply to page table entries.

#define PG_LOCK (1<<9)  /* OS/90: Memory cannot be swapped */
#define PG_UCM  (1<<10) /* OS/90: Page part of uncomitted block */

// Chain ID is exposed as 32-bit for future extensibility
// In the current implementation, it is an index to the block list.
// Chains are operated on by all parts of MM, so it is defined here.
typedef U32 CHID;

API_DECL(BOOL, MM_Reent_Stat, VOID);

#endif /* MMDEFS_H */
