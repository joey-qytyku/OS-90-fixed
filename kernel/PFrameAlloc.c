/*
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with OS/90. If not, see <https://www.gnu.org/licenses/>.
*/

#include <Scheduler/V86M.h>
#include <Memory/Memory.h>
#include <Misc/Linker.h>
#include <Type.h>

#define MEM_BLOCK_SIZE (16*1024)

#define CHECK_ALIGN(address, if_unaligned)\
if ((DWORD)address & 0xFFF != 0)\
    {goto if_unaligned ;}      /* No misleading IF warning */

// The ISA memory hole is a 1M space that is sometimes
// reserved for ISA cards. It is at F00000-FFFFFF. PCs that do not need it may
// have it anyway or an option to enable it. The target  platform
// (PCs from 1990-1999) certainly have an ISA memory hole
// so it is assumed to be present.

static DWORD memory_after_15M; // Extended memory after the hole
static DWORD memory_after_1M;  // Extended memory between hole and 1M

// A single page directory is used for the entire the system
// Processes only use 384 entries in the PD, so we save memory by giving
// the PCBs unaligned parts of it, which are copied to the real PD

static ALIGN(4096) DWORD global_page_directory[4096];

static DWORD  num_total_blocks = 0;
static PBLOCK block_list = NULL;
static DWORD  cur_id = 0;
static DWORD  top_index;

static DWORD AddrAlign(PVOID addr, DWORD bound)
{
    return ((DWORD)addr + bound - 1) & ~(bound-1);
}

// Get the physical address pointed to by the virtual address.
// Returns OS_OK if valid address. Anything else indicates error.
//
STATUS GetPagedPhysicalAddress(
    PDWORD page_dir,
    DWORD  virt_addr,
    PVOID *out_phys)
{
    DWORD cr3;
    __asm__ volatile ("mov %%cr0, %0":"=r"(cr3));
}

STATUS MmAllocateChain(
    DWORD flags,
    DWORD num_pages)
{
    DWORD num_left_to_allocate = num_pages / MEM_BLOCK_SIZE;

    if (num_pages < 1)
    {
        return OS_ERROR_GENERIC;
    }

    for (DWORD i = 0; num_left_to_allocate == 0; i++)
    {
        if (block_list[i].bf_free == 1)
        {
            // Is it the first in the list? If so, write the information
            if (num_left_to_allocate == num_pages)
            {
                block_list[i].bf_frozen    = (flags & CH_INIT_LOCKED)!=0;
                block_list[i].bf_user      = (flags & CH_USER)       !=0;
                block_list[i].bf_phys_cont = 0;
                block_list[i].bf_free      = 0;
                block_list[i].b_idnum      = cur_id;
                num_left_to_allocate--;
                continue;
            }

            block_list[i].bf_free = 0;
            block_list[i].b_idnum = cur_id;
            num_left_to_allocate--;
        }
        if (i > top_index)
            top_index = i+1;
    }
    cur_id++;
    return OS_OK;
}

//
// Cache disable bit?
//

STATUS MmAllocContiguousChain()
{}

// A bit called "dirty" will indicate if a page has been accessed
// before. This will reset it so that this can be determined.
// the next time it is accessed.
VOID MmCleanPages(DWORD chain_id)
{}

// Prevent the chain from being swapped out of the memory.
//
VOID MmLockPages(DWORD chain_id)
{}

//
// Returns 1 if a page in the chain has been accessed before.
// Returns 0 if the range is not valid
//
BOOL MmCheckPagesDirty(
    DWORD chain_id,
    DWORD page_offset,
    DWORD num_pages)
{}

STATUS MmMapChainToVirtualAddress(
    DWORD   flags,
    PDWORD  page_tables_array,
    PVOID   map_to_virt_addr)
{}

// Depends on V86 being ready
void InitPFrameAlloc()
{
    // Detect extended memory above 1M

    // Find the address where the kernel will store block records
}
