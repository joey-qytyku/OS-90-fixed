# Paging and Virtual Address Spaces

Userspace programs have access to no more than 256 megabytes of virtual addressing space. The kernel is mapped to the 2/3 mark of the virtual address space at all times, which is at 0xC0000000.

## Page Frame Allocation

All memory is in 16K blocks starting from zero. This option can be configured in the config header file.

This is a very simple and fast way to manage pages, but is not very space efficient.

```c
tpkstruct {
    CHID    id:12;
    BYTE    flags:4;
    WORD    next;
};
```
This strcuture is an element of the block list.

If I use bit array allocation, I can reduce the necessary bits for the ID.

The `AVAIL_IDS` is divided by 32 to get the size of a DWORD array (it must be divisible by 32). By default, we have 4096 available IDs, which should be more than enough.

## Global Page Directory

We do not need to keep a whole page directory for each process. This would be very wasteful and make process control blocks oversized, because 2/3 of it will be the exact same. The solution is to store only the user section of the page directory and copy it to a global main page directory. 64 PD entries must be copied into the PD on every context switch, but the overhead is quite minimal. The page directory entries of the PCB is called the user page directory, or UPD. The kernel's actual page directory is called the KPD.

## Mapping a Page to a Physical Page Frame

```
MmMapPhysicalToVirtualDirect
```

This function maps a contiguous block of pages to a physically contiguous block of page frames. This should be used in conjunction with virtual address space services to access MMIO directly. It operates on the KPD or a UPD.

Accessing a region directly should be avoided because drivers may currently be virtualizing IO to the address and mapping it elsewhere for a random process.

## Mapping a Chain to Specific Addresses

Our goal is to have a single function that takes the ID of a chain and maps it to a virtual address contiguously. Page tables must be mapped into memory somehow before they can be modified. The page directory is global so it does not need to be allocated. The mapper operates on a UPD or the kernel KPD and KPTs.

```c
MAP_UPD
MAP_KPD

STATUS APICALL MmMapChainToVirtualAddress(
    DWORD   flags,
    DWORD   id              // ID of chain
    PVOID   map_to,         // Virtual address to map to
    DWORD   page_offset,    // Offset within the chain, in pages
    DWORD   page_count,     // Number of pages to map
    PVOID   if_upd_address  // Address of the UPD
);
```

```c
// Do not do this. This is just an example of how the functions work

__ALIGN(4096)
DWORD mem_window[MEM_BLOCK_SIZE / sizeof(DWORD)];
DWORD id;

VOID Example()
{
    id = MmAllocateChain(0, 1);

    MmMapChainToVirtualAddress(MAP_KPD, id, mem_window, 0, NULL);
    mem_window[0] = 0xDEADBEEF;
}
```

Operations to support:
* Page clean
* Mark as swappable
* Freeze pages
* Disable cache
* Allocate blocks as contiguous (for DMA)
* Getting the physical address of a contiguous chain
* Getting the physical address of specific pages in the chain

Chains have attributes in the flags. The first entry is the only one that needs to have it.

Contiguous blocks are special and cannot be resized. Frozen is implied. Only the first entry in a chain of blocks needs to have the flags. ID numbers are 27-bit.

## Given a Page Table Entry, Finding the ID

```c
CHID MmGetChainIdOfPage(
    BOOL    use_kpd,
    DWORD   page_index
);
```

Later in this document, it will be necessary to find the ID that a page belongs to to implement uncommitted memory.

This takes a page index rather than a block index. We floor the index to the block boundary and index that block in the block list. Then we return the ID.

## Virtual Address Space Allocation

Specifying fixed addresses for all allocations is not practical, so the operating system needs a way to map memory to arbitrary locations.

Bitmap allocation is used to implement virtual address space allocation. To use these features, a special memory management structure must be specified. Virtual address nodes must be aligned at 4MB page directory entry boundaries.

The VA_NODE structure is a linked list element. The kernel walks the list when allocating virtual address spaces. The kernel comes with its own structure called `kernel_node`, which is global to all drivers.

A bitmap represents the availability of pages in the virtual address space. The spaces allocated do not need to be entirely present.

```c
// Returns the base address of the region

PVOID MmAllocateVirtualRegion(
    P_VA_NODE   vanode,
    DWORD       num_pages
);

STATUS MmReleaseVirtualRegion(
    P_VA_NODE   vanode,
    PVOID       base_addr
    DWORD       num_to_free
);

```

Here is an example of memory allocation with virtual address spaces. This is a much better way to allocate.
```c
const DWORD to_alloc = MEM_BLOCK_SIZE / 4096;

VOID Example()
{
    PBYTE vspace;
    DWORD id;

    vspace = MmAllocateVirtualRegion(&kernel_node, to_alloc, VA_PRESENT);
    id     = MmAllocateChain(0, 1);
    MmMapChainToVirtualAddress(MAP_KPD, id, vspace);

    C_strcpy(vspace, "Hello, world");

    MmReleaseVirtualRegion(&kernel_node, vspace, to_alloc);
}
```

The following is an example for direct MMIO access. This actually is required for direct access because a drvier may be virtualizing MMIO to the specified address for a process.
```
VOID Example()
{
    PWORD vspace;
    vspace = MmAllocateVirtualRegion(
        &kernel_node,
        1,              // Get one page
        VA_PRESENT | VA_NOCACHE | VA_R0
    );

    MmMapPhysicalToVirtualDirect((PVOID)0xB8000, vspace);

    vspace[0] = 0xF01; // Display smiley

    MmReleaseVirtualRegion(&kernel_node, vspace, 1);
}
```

## Uncommitted Memory

Uncommitted memory has to be supported in situations where software allocates a lot of memory that it does not need to use at once. An uncommitted page does not have a disk location of physical page frame, which means it does not exist until accessed for the first time.

Because we use pool-based allocation, a single uncommitted page is not possible.

Uncommitted memory regions may not be contiguous within one allocation of page frames. If we allocate 32K with 16K blocks and access the high blocks, the desired behavior is that only one block is actually allocated and the other one will be unused.

A solution is to allow the block list to store uncommitted block candidates.

### Relationship with DPMI

We support uncommitted pages at the level required by DPMI. When we allocate memory using INT 31H, it will all be committed. Resizing it further has the option of generating committed or uncommitted pages.

Resizing blocks and UCOMM is only supported by DPMI 1.0, but we implement it for the userspace.

### Implementation

We use a reserved bit in the page table entry to indicate a page is inside an unallocated block. Pages that form a complete block must have this bit all on or all off. If `PG_UNCOMMITTED` is set, the block represented by this page and the following ones are not allocated.

Note that `PG_UNCOMMITTED` cannot be valid with `PG_LOCKED` because memory that is locked cannot possibly be uncommitted.

Uncommitted pages can only be generated by resizing allocations.

When a page fault occurs and we find that the page table entry referrenced is uncommitted, we need to immediately map the pages in this block somewhere. We will need to reallocate the block list that this page happens to be part of. To do this, a function for getting the ID given the page table entry is called.

## Restrictions on Allocating Memory

Interrupt service routines may never call anything that modifies the page tables or other memory managment structures. The kernel API for memory managment is not reentrant and runs with IRQs enabled most of the time, so calling MM functions will not work safely in an ISR.

# DPMI

The kernel only supports the features that DPMI implements for virtual memory. The userspace memory manager is responsible for implementing heaps and other features.

We will "respond" to each feature that DPMI calls for and describe how OS/90 implements them.

## Page Locking

DPMI allows memory regions to be locked so that they are not paged out by the memory manager. The spec defines locking with referrence counting, where a page is not actually unlocked until the counter is decremented to zero.

I think this is because interrupts will probably try to lock memory before using it (though OS/90 does not require this, DOS programs will do it anyway) and that memory may already be locked. If the interrupt service routine unlocks the memory, it may not be in the state expected by the main thread of execution.

But OS/90 fakes interrupts and does not require programs to lock memory while handling them. Interrupts at the process level are simply changes in program flow. On a single-tasking DPMI host, interrupts will be called directly from a ring-0 ISR, which means that they cannot cause page faults.

Locking can be implemented using a reserved bit inside the page table entry. The problem is how we are supposed to implement the lock counter. There is no explicit specification for how many locks it needs to be able to handle and there are no ways to get the current count; the program simply needs to "know."

A solution is to make the functions calls for page locking succeed unless the ranges specified are wrong. Since page locking does not influence program behavior, there is no reason for a program to fail or not because of it. Software that locks pages will run as expected.

## Real Mode Locking

DPMI allows programs to lock memory in the 1M real mode region. The entire 1M+64K region is always present in memory. These functions will always return success because we cannot do that.

## Get Page Size

4096 is the page size.

## Allocating Linear Memory

The handles returned to the program from DPMI allocation services are 32-bit block IDs for the kernel.

## Resizing Linear Memory

This is a DPMI 1.0 feature but is supported for userspace purposes.

## Mark as Demand Paging Candidates

DPMI supports demand paging. Pages can be marked as candidates for swapping to the disk. These features do work on OS/90.

OS/90 uses a fixed-size page file called SWAP.SYS.

## XMS

XMS is a 16-bit API. The protected mode implementation involves a far call hook. UMBs will not be supported. It supports the following operations on 16-bit entries:
* Freeze
* Unfreeze
* Delete
* Allocate
* Copy to/from conventional memory

## DOS

DOS memory sometimes has to be allocated because the memory block needs to be in conventional memory. This is especially the case for ISA DMA which can only use 24-bit addressing. It is recommended that user leave as much DOS memory free as possible. The kernel runs DOS processes with allocation functions as well. Local conventional memory is not supported.

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

# LIM EMS

LIM EMS cannot be used as regular memory because it uses page-based bank switching and is incompatilble with the memory manager. An EMS card can however be used as a ramdisk or swap.

This would require bypassing the Limulator built into OS/90 to make calls to the actual driver.

I wonder if this is really worth my time. EMS cards started to fall off during the 90's when computers were shipping with easier to program extended memory and operating systems started running in protected mode.

DOS truncates all far pointers, by the way.
