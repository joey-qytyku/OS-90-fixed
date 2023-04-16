# Note

Page table entries have a dirty bit. Use this for virtual devices.

# Paging and Virtual Address Spaces

Userspace programs have access to no more than 256 megabytes of virtual addressing space. The kernel is mapped to the 2/3 mark of the virtual address space at all times, which is at 0xC0000000.

## Page Frame Allocation

All memory is in 16K blocks starting from zero.

## Global Page Directory

We do not need to keep a whole page directory for each process. This would be very wasteful and make process control blocks oversized, because 2/3 of it will be the exact same. The solution is to store only the user section of the page directory and copy it to a global main page directory. 64 PD entries must be copied into the PD on every context switch, but the overhead is quite minimal. The page directory entries of the PCB is called the user page directory, or UPD. The kernels actual page directory is called the KPD.

## Mapping to Specific Addresses

Our goal is to have a single function that takes the ID of a chain and maps it to a virtual address contiguously. Page tables must be mapped into memory somehow before they can be modified. The page directory is global so it does not need to be allocated. The mapper operates on a UPD or the kernel KPD and KPTs.

```c
MAP_UPD
MAP_KPD

MmMapChainToVirtualAddress(
    DWORD   flags,
    DWORD   id
    PVOID   map_to,
    DWORD   page_offset,
    PVOID   if_upd_address
);
```

```c
__ALIGN(4096)
DWORD mem_window[1024];
DWORD id;

VOID Example()
{
    id = MmAllocateChain(0, 8);

    MmMapChainToVirtualAddress(MAP_KPD, id, mem_window, 0, NULL);
    mem_window[0] = 0xDEADBEEF;
}
```

Operations to support:
* Page clean
* Mark as swappable
* Freeze pages
* Allocate blocks as contiguous (for DMA)
* Getting the physical address of a contiguous chain
* Getting the physical address of specific pages in the chain

Contiguous blocks are special and cannot be resized. Frozen is implied. Only the first entry in a chain of blocks needs to have the flags. ID numbers are 27-bit.

# Memory Allocation

## XMS

XMS is a 16-bit API. It supports the following operations on 16-bit entries:
* Freeze
* Unfreeze
* Delete
* Allocate
* Copy to/from conventional memory

UMBs will not be supported.

The XMS implementation involves a far call hook.

## DPMI

DPMI uses 32-bit handles for allocated blocks. Allocation is page granular and page aligned.

## DOS

DOS memory sometimes has to be allocated because the memory block needs to be in conventional memory. This is especially the case for ISA DMA which can only use 24-bit addressing. It is recommended that user leave as much DOS memory free as possible. The kernel runs DOS processes with allocation functions as well.

# Block Reshuffling (OLD)

Blocks are sometimes reshuffled to make more space. To determine an optimal way to do this algorithm:

Each symbol represents a different memory block. Spaces are page boundaries.

##|@@|!!|!!|!!

I delete @@.

##|  |!!|!!|!!

I then allocate @@@@. I can do something here.

@@|@@|!!|!!|!!|##|

This is the desired output. The concept is that small blocks of memory can be relocated to reduce fragmentation. Note that I can only reduce it. There is no perfect method.

Defragmentation should look to the smaller blocks (<4K) and attempt to nearly fill entire pages with them. Alignment is never garaunteed, but will probably be 32-bit for better performance.

Defragmenting is a slow process that requires copying memory, so it should happen rarely. It can happen with fixed intervals or a more complicated algorithm can be implemented.

For example, if there is a large deviation between allocation sizes, a defrag is more likely.

## Advice

See code.md for advanced tips on allocating memory.

# Dynamic structures

## Vector
