////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

//
// This file implements routines for page table manipulation:
// * Mapping chains to arbitrary addresses
// * Page permissions
// * Memory window

#include <Memory/Chain.h>
#include <Memory/Map.h>
#include <Debug.h>

#define KERNEL_VAS_SIZE_BYTES MB(16)

alignas(4096)
static U32 page_directory[1024];

// More than one page long.
static U32 kernel_page_table_entries[KERNEL_VAS_SIZE_BYTES / MB(4)];

// A chain for holding the page tables.
static CHID page_table_chain;

// BRIEF:
//      Set memory window. At the very end of the address space, a single
//      virtual block can be used to access block quantities. It is necessary
//      for transversing blocks without mapping them.
//
//      Not exposed to drivers. NEVER OPERATE ON THIS REGION.
//
static VOID SetMemoryWindow(PVOID where)
{
    const U32 winpgindex = sizeof(kernel_page_table_entries)-1;

    U32 kpindex = kernel_page_table_entries[winpgindex];
    U32 where2 = (U32)where2 & (~0xFFF);

    kernel_page_table_entries[kpindex] = (where2 << PG_SHIFT) | PG_P | PG_RW;

    #ifdef CPU_486
    for (U32 i = 0; i<MEM_BLOCK_SIZE/4096; i++)
    {
        __asm__ volatile("invlpg (%0)" ::"r" (win_base + i*4096) : "memory");
    }
    #else
    __asm__ volatile ("mov %0,%%cr3"::"r"(page_directory));
    #endif
}

//
// This should fold to a simple TEST and JNZ pair.
//
static inline BOOL Aligned(PVOID addr, U32 align)
{
    return ((U32)addr & (align-1)) == 0;
}

static VOID SetMemoryWindowToCHLocalBlock(
    CHID    id,
    U32     local_index
){
    PVOID block_addr = ChainWalk(id, local_index);

//    kernel_page_table_entries[];
}

STATUS MapChainToVirtualAddress(
    U32     attr,
    CHID    id,
    PVOID   address
){
}

STATUS KERNEL MapBlock(
    U32     attr,
    PVOID   virt,
    PVOID   phys
){
    if (Aligned(virt,MEM_BLOCK_SIZE) && Aligned(phys,MEM_BLOCK_SIZE))
        ;
    else return OS_ERROR_GENERIC;

    return OS_OK;
}

VOID InitMap(VOID)
{
}
