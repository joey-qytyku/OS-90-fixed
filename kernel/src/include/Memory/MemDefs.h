/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef MMDEFS_H
#define MMDEFS_H

#include <Type.h>

#define MB(x) (x * 1048576)
#define KB(x) (x * 1024)

#define MEM_BLOCK_SIZE KB(16)

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

API_DECL(BOOL, MmReentStat, VOID);

#endif /* MMDEFS_H */
