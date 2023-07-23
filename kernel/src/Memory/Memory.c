/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <https://www.gnu.org/licenses/>.
*/

#include <Scheduler/V86M.h>
#include <Memory/Memory.h>
#include <Misc/BitArray.h>
#include <Misc/Linker.h>
#include <Type.h>

// The ISA memory hole is a 1M space that is sometimes
// reserved for ISA cards. It is at F00000-FFFFFF. PCs that do not need it may
// have it anyway or an option to enable it. The target platform
// (PCs from 1990-1999) certainly have an ISA memory hole
// so it is assumed to be present.

static DWORD memory_after_15M; // Extended memory after the hole
static DWORD memory_after_1M;  // Extended memory between hole and 1M

// A single page directory is used for the entire the system
// Processes only use 384 entries in the PD, so we save memory by giving
// the PCBs unaligned parts of it, which are copied to the real PD

static ALIGN(4096) DWORD global_page_directory[1024];

//
// The kernel page tables are always added to the KPD at the same location
// and are always present and locked.
//
static ALIGN(4096) DWORD kernel_page_tables[KERNEL_PAGE_TABLES];

static DWORD block_id_avail_map[1<<(BLOCK_ID_BITS-3)];
static const block_id_avail_map_bnd = (1<<(BLOCK_ID_BITS-3)) - 1;

static DWORD  num_total_blocks = 0;
static P_MB   block_list       = NULL;

// For allocating contiguous blocks, we need to know where the top of the list
// is. It makes things much faster.
static DWORD  top_index = 0;

static DWORD AddrAlign(PVOID addr, DWORD bound)
{
    return ((DWORD)addr + bound - 1) & ~(bound-1);
}

// BRIEF:
//      Gets a unique ID for a new chain.
// ARGS:
//      None
// RETURN:
//      0xFFFFFFFF if failed to get an ID. Otherwise, the ID obtained.
//
static CHID GetUniqueChainID(VOID)
{
    CHID ret_id;
    return AllocateOneBit(
        block_id_avail_map,
        block_id_avail_map_bnd,
        &ret_id
    )
    == OS_OK ? ret_id : -1;
}

// BRIEF:
//      Allow a previously used ID to be reused.
//
static VOID FreeChainID(CHID id)
{
    KeDisableBitArrayEntry(block_id_avail_map, id);
}

// BRIEF:
//      Allocate a chain of blocks.
//
// EXPLAINATION:
//
// INPUTS:
//      num_pages:  Automatically converted to blocks. The program does not
//                  need to know how large a block is.
//
// WARNINGS:
//      A lot of GOTO.
//
// FLAGS:
//
//  The following flags are provided for this function that decide attributes
//  about the block.
//
//  CH_USER             When we map this memory, remember to make user pages.
//                      Do not permit userspace to modify if CH_USER not passed.
//
//  CH_PHYSCONT         Represented in the list as contiguous. Can be used for
//                      DMA or anything that requires physical memory
//                      to be contiguous.
//
// The privilege of the chain matters because a userspace program cannot
// manipulate chains that belong to the kernel.
//
// RETURN:
//      OS_OUT_OF_MEMORY if no memory
//
STATUS KERNEL MmAllocateChain(
    DWORD flags,
    DWORD num_pages
){
    // This is a value that we write to when allocating the first block.
    // A pointer is used to access the front link of the previous block,
    // but there is no special case for the first one except for the
    // first flag, so we need a place to put the irrelevant loop iterator
    // value.
    const WORD      nonsense_word = 0;
    PWORD           prev_blk_link = &nonsense_word;

    const BOOL user        = FLAG_PARAM_ON(flags, CH_USER);
    const BOOL physcont    = FLAG_PARAM_ON(flags, CH_PHYSCONT);
    const CHID new_id      = GetUniqueChainID();
    DWORD i;
    DWORD base;

    if (!~new_id)
        goto no_id;

    // These conditions will esure that there is no OOM that does not return
    // an error.
    if (physcont)
    {
        if (top_index == num_total_blocks)
            goto oom;
        i = top_index;
    }
    else { i = 0; }
    base = i;
    // i will change, base will reflect the first block

    for (;; i++)
    {
        if (i == num_total_blocks)
            goto oom;

        if (block_list[i].f_free == 1)
        {
            block_list[i].f_is_first  = (i == base);
            block_list[i].f_phys_cont = physcont;
            block_list[i].idnum       = new_id;
            block_list[i].f_free      = 0;
            *prev_blk_link = i;
        }
        // Set for next iteration
        prev_blk_link = &block_list[i].next;
    }
oom:
    return OS_OUT_OF_MEMORY;
no_id:
}

STATUS MmResizeChain()
{}

//
// Cache disable bit?
//
STATUS MmAllocContiguousChain()
{}

//
// Returns 1 if a page in the chain has been accessed before.
// Returns 0 if the range is not valid
//
BOOL MmCheckPagesDirty(
    DWORD chain_id,
    DWORD page_offset,
    DWORD num_pages
){}

STATUS KERNEL MmMapChainToVirtualAddress(
    DWORD   flags,          // Flags for operation
    CHID    id,             // ID of chain
    PVOID   map_to,         // Virtual address to map to
    DWORD   page_offset,    // Offset within the chain, in pages
    DWORD   page_count,     // Number of pages to map
    PVOID   if_upd_address  // Address of the UPD
){}

// The block list is stored directly after the kernel. It is aligned to a
// page boundary.
static PVOID LocateBlockListLocation()
{
    DWORD location = &LKR_END;

}

static VOID InitBlockListDefaults()
{}

// Depends on V86 being ready
VOID InitPFrameAlloc(VOID)
{
}
