/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <ttps://www.gnu.org/licenses/>.
*/

#ifndef MEMORY_H
#define MEMORY_H

#define MB(x) (x * 1048576)
#define KB(x) (x * 1024)

#define MEM_BLOCK_SIZE KB(16)

// Make this variable?
#define KERNEL_ADDRESS_SPACE_SIZE MB(16)

// The number of page tables that will be inserted into the 0xC0000000 range
#define KERNEL_PAGE_TABLES (KERNEL_ADDRESS_SPACE_SIZE / MB(4))

#define CHECK_ALIGN(address, if_unaligned)\
if ((DWORD)address & 0xFFF != 0)\
    {goto if_unaligned ;}      /* No misleading IF warning */

#define BLOCK_ID_BITS 14
#define BLOCK_NEXT_BITS 16

#define PG_SHIFT 12

#define PG_D     (1<<6) /* Page is dirty, was accessed */
#define PG_A     (1<<5) /* The page table/dir was used */
#define PG_PCD   (1<<4) /* Page cache disable */
#define PG_PWT   (1<<3) /* Page write though cache */
#define PG_U     (1<<2) /* User */
#define PG_RW    (1<<1) /* Read/write */
#define PG_P     (1<<0) /* Present */

//
// The unused bits are used for OS/90 to implement uncommitted memory and
// locking. These only apply to page table entries.
//

#define PG_LOCK (1<<9)  /* OS/90: Memory cannot be swapped */
#define PG_UCM  (1<<10) /* OS/90: Page part of uncomitted block */

//
// This means that the page is mapped. It is a hint to the memory manager that
// the page directory entry has a page table associated with it.
//
#define PG_MAPPED (1<<11)

//
// Flags for the chain allocate function.
//

#define CH_USER        (1<<0)
#define CH_INIT_LOCKED (1<<1)
#define CH_PHYSCONT    (1<<2)

//
// For the generic memory map function.
//

#define MAP_KPD (1<<0)
#define MAP_UPD (1<<1)
#define MAP_RD  (1<<2)

// Chain ID is exposed as 32-bit for future extensibility

typedef DWORD CHID;

tpkstruct
{
    CHID    idnum       :13;
    BYTE    f_free      :1;
    BYTE    f_phys_cont :1;
    BYTE                :1;
    WORD    rel_index;
    WORD    next;
    WORD    prev;
    WORD    owner_pid;
}MB,*P_MB;

tpkstruct
{
    WORD    chain;
    WORD    index;
}VML_ENT;

typedef enum {
    ALLOC_USER = (1<<0),
    ALLOC_CONT = (1<<1)
}ALLOC_FLAGS;

// Grow/shink are a number from 1-2 when shifting the number.
typedef enum {
    RSZ_UNCOMMITTED    = (1<<0), // Generate uncommitted blocks (skip indices)
    RSZ_GROW           = (1<<1), // Increase relative to current size
    RSZ_SHRINK         = (1<<2)  // Decrease size
}RESIZE_FLAGS;

typedef enum {
    DBIT_STAIN,
    DBIT_SET
}DBIT_FLAGS;

VOID MapBlock(PVOID virt, PVOID phys);

CHID ChainAlloc(
    DWORD flags,
    DWORD bytes
);

DWORD ChainSize(CHID id);

STATUS ResizeChain(
    RESIZE_FLAGS flags,
    CHID    id,
    DWORD   new_size
);

VOID SetBlockDirtyStatus(BOOL do_stain, PVOID addr, DWORD num);

STATUS MapChainToVirtualAddress(
    DWORD   attr,
    CHID    id,
    BOOL    use_kpd,
    PVOID   address,
    PVOID   opt_upd
);


#endif /* MEMORY_H */
