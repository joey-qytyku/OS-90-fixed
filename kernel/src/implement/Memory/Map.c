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
#include <Debug/Debug.h>

#define KERNEL_VAS_SIZE_BYTES MB(16)

// PD entries for the kernel always point to the page tables.
// Therefore, it is not necessary to manipulate PD entries
// for the kernel address space.
ALIGN(4096)
static U32 page_directory[1024];

// More than one page long.
static U32 kernel_page_table_entries[KERNEL_VAS_SIZE_BYTES / MB(4)];

// A chain for holding the page tables. Nothing fancy here. We just allocate
// far enough to have page tables available for the address available if there
// is not space already.
//
// This is NOT used for the kernel. The kernel has a fixed size address space.
//
static CHID page_table_chain;

// BRIEF:
//      Set memory window. At the very end of the address space, a single
//      virtual block can be used to access block quantities. It is necessary
//      for transversing blocks without mapping them.
//
//      Not exposed to drivers. NEVER OPERATE ON THIS REGION.
//
static VOID Set_Memory_Window(PVOID physical)
{
    // Uh... u sure?
    const U32 winpgindex = sizeof(kernel_page_table_entries)-1;

    U32 kpindex = kernel_page_table_entries[winpgindex];
    U32 where2 = (U32)physical & (~0xFFF);

    kernel_page_table_entries[kpindex] = (where2 << PTE_SHIFT) | PG_P | PG_RW;

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

//
//
// RETURN:
//      1 if okay
//      0 if index does not exist (block not mapped)
//
static BOOL Set_Memory_Window_To_Chain_Local_Block(
    CHID    id,
    U32     local_index
){
    PVOID block_addr = ChainWalk(id, local_index);

    SetMemoryWindow(block_addr);
}

// BRIEF:
//      Map a single block to a virtual address.
//
STATUS kernel Map_Block(
    U32     attr,
    PVOID   virt,
    PVOID   phys
){
    if (!Aligned(virt,MEM_BLOCK_SIZE) || !Aligned(phys,MEM_BLOCK_SIZE))
        return OS_INVALID_PARAMS;

    const U32 virt_as_int = (U32)virt,
              phys_as_int = (U32)phys;

    const U32 num_ptabs = ChainSize(page_table_chain) / PAGE_SIZE;
    const U32 page_index;

    // Is the address in the user range
    if (virt_as_int < 0xC0000000)
    {
        // It is in the user range. This means we have to use the
        // user page tables.

        // Check if the address is a valid for userspace.
        if (virt_as_int > (0xC0000000 - MEM_BLOCK_SIZE))
            return OS_INVALID_PARAMS;

        // SetMemoryWindowToChainLocalBlock(page_table_chain, );
    }
    else {
        // Kernel PD entries are always set up to point to the page tables.
        // Page table entries can therefore be modified directly.
        const U32 local_pg_index = (virt_as_int - 0xC0000000) / 4096;

        kernel_page_table_entries[local_pg_index  ] = attr | phys_as_int;
        kernel_page_table_entries[local_pg_index+1] = attr | phys_as_int+4096;
        kernel_page_table_entries[local_pg_index+2] = attr | phys_as_int+8192;
        kernel_page_table_entries[local_pg_index+3] = attr | phys_as_int+12288;

    }

    return OS_OK;
}

STATUS Map_Chain_To_Virtual_Address(
    U32     attr,
    CHID    id,
    PVOID   address
){
    if (!Aligned(address,MEM_BLOCK_SIZE))
        return OS_INVALID_PARAMS;

}

//
//
//
VOID Init_Map(VOID)
{
}
