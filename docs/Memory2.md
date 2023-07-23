# Memory Management Subsystem

OS/90 has an advanced memory manager supporting all modern virtual memory functionality.

It supports:
* Allocating memory and mapping to address spaces
* Reserving and releasing virtual memory regions
* Virtual memory with disk and uncommitted RAM backing
* Emulating framebuffers through virtual MMIO
* Complete compatibility with DPMI 0.9 and 1.0 memory functions

# Terminology

MMS: Memory management subsystem.

PT: Page table

PD: Page directory

KPD: Kernel page directory

UPD: Process-local page directory entries

Block: A group of page frames

Block list: A modified linked list structure for allocating physical blocks. Also helps account for virtual memory by keeping indices.

Chain: A chain of blocks linked together in their virtually contiguous order.

Chain ID: A number indicating which chain is to be used. Exposed as a 32-bit number.

Uncommitted: Memory that is not present but is to be backed by memory and not storage. Accessing causes memory to be allocated.

VAS: Virtual address space

VML: Virtual memory list. This structure keep track of available virtual blocks in the address space.

Conventional memory: The first 640KB of memory. This is global to all processes.

Extended memory: The memory that comes after the 1MB memory limit.

High memory area: The first 65520 bytes of extended memory. Accessible from real mode with A20 gate enabled by overflowing the seg:off calculation.

XMS: Extended Memory Specification. Allows real mode programs to allocate extended memory blocks (EMBs) and transfer to/from conventional memory.

# Chains

## The ID

The ID is a 32-bit number which has no special meaning to userspace or drivers in itself.

# OS/90 is a Single Address Space Operating System

First of all, the conventional memory is shared by the whole system. This reduces multitasking abilities for DOS programs, but simplifies MM and scheduler design and makes the single address space possible.

Second of all, extended memory is allocated using APIs like DPMI and XMS. It is never accessed directly. Nearly all programs are written for DPMI 0.9 (which is what OS/90 reports despite having some 1.0 features) and they have to fix-up executable data to run at an arbitrary address space. OS/90 and DPMI 1.0 allow for selecting a fixed address, but no regular DOS program will think to use it.

The single address space means that IPC is very easy and context switching is much faster due to not having to flush TLB entries when switching modes. Page tables do not have to be allocated for each process. Process memory can also be accessed directly.

## Address Space Growth

The userspace VAS must be larger than the maximum physical memory. It is 3GB. Keeping page tables for this much memory is impractical because it would be 4MB of RAM constantly in use. Instead, OS/90 has a growing page table chain.

## Emulating MMIO

This is possible. Another section deals with the implementation and API

# Core Memory API

The following is a full description of the API implemented by the memory manager.

## Memory Info

```c
DWORD MemInfo(BYTE op);
```

Operations:
```
MEM_TOTAL   Physical RAM accessible by kernel
MEM_INUSE   Memory currently allocated and committed
MEM_KERNL   Memory occupied by kernel, includes real mode

MEM_AVSWP   Total swap space available
MEM_IUSWP   Swap space currently in use

CFG_BLOCK_SIZE      Size of block in bytes
CFG_ADDRESSABLE     Number of blocks addressable
```

This function is slow.

## Map Physical Block to Virtual Block

```c
VOID MapBlock(DWORD attr, PVOID virt, PVOID phys)
```
Addresses must be page aligned. Mostly for internal use but can be used for MMIO mapping (a better function exists for that).

This will modify the page tables and will extend the PTC if necessary. The attributes are simply what is provided by the kernel for page table entries (PG_...).

## Check Memory Manager Lock

```c
VOID KERNEL_ASYNC GetMmLock(VOID)
```

If this returns one, then absolutely no memory manager functions may be called.

## Allocate Chain


```c
CHID ChainAlloc(
    ALLOC_FLAGS flags,
    DWORD       bytes
);
```

The chain ID is a 32-bit value even though it is implemented as a more narrow value. This allows it to carry an invalid value of `-1`.

Flags:
```
ALLOC_USER      Memorize this as a userspace chain for protection
ALLOC_CONT      Physically contiguous (for DMA)
```

This should not be used like `malloc`. It is a low-level function call for page frame allocation.

## Get Size Of Chain

```c
DWORD ChainSize(CHID id);
```

returns 0 if the id is invalid.

## Resize a Chain

```c
STATUS ResizeChain(
    RESIZE_FLAGS   flags,
    CHID    id,
    DWORD   new_size
);
```
Options:
```
RSZ_UNCOMMITTED    Generate uncommitted blocks (skip indices)
RSZ_GROW           Increase relative to current size
RSZ_SHRINK         Decrease size
```

WAIT. How can I add uncommitted blocks if the chain only works with physical blocks.

If `new_size` is zero, the allocation is deleted.

Resizing the chain does not remap it. It must be mapped to the desired location with the appropriate function. Usually, the address space should be changed after resizing.

## Stain/Clean Blocks

This will mark the pages of a block as dirty.

```c
BOOL GetSetBlockDirtyStatus(
    DBIT_FLAGS  flags,
    PVOID       addr,
    DWORD   num
);
```

`do_stain` is ignored if getting.

This takes a virtual address and operated on the dirty bits of the associated pages. Input must be checked.

## Evict Blocks From Memory

```c
STATUS EvictBlockFromMem(
    CHID    id,
    DWORD   byte_off
    DWORD   byte_num
);
```

Forcefully swap out of memory to the disk. It will be removed from the linked list, but can still be accessed if the chain is remapped in order to generate not-present pages.

If the offset specified is uncommitted or not present, an error will occur.

This will only take into affect after remapping the chain.

## Replace Missing Blocks in Chain (INTERNAL) (TODO)

```c
ReplaceMissingBlocks()
```

Allocates memory for the blocks in the chain that have been evicted. This is used internally for swapping in memory.

## Map Chain to Virtual Address

```c
STATUS MapChainToVirtualAddress(
    DWORD   attr,
    CHID    id,
    PVOID   address
);
```

Map the chain to the specified address. It is a fixed location. The attributes are the same as the v2p mapper function.

The address provided must be block aligned!

## Reserve Linear Region

```c
PVOID ReserveLinearRegion(DWORD num_bytes);
```

Allocate a range of virtual blocks. This does not allocate the memory or modify page tables, but ensures the region will not be used by any other software.

Returns `NULL` if failed.

Example:
```c
VOID Example(VOID)
{
    PBYTE buffer;
    CHID  id;

    id = ChainAlloc(ALLOC_NORMAL, 1000);
    buffer = ReserveLinearRegion(1000);
    MapChainToVirtualAddress(ATR_NORMAL, id, buffer);
}
```

## Release Linear Region

```c
VOID ReleaseLinearRegion(
    PVOID   base
    DWORD   size
);
```

Oppositie of reserving. The size must be specified.

The reason why this does not "know" the size of the allocation is because we may want to reserve a range of addresses in the anticipation of growth, where some of the pages are not mapped yet.

# Page Locking

Pages are sometimes locked by userspace for performance reasons, but OS/90 would probably not benefit because it does not swap memory unless RAM is very low. For this reason, it is not currently supported for userspace through the page locking DPMI service. Pages actually do not need to be locked since local interrupts for processes can safely cause faults; they are not real interrupts.

# Block List

A block list entry:
```c
// 8 bytes long
tpkstruct
{
    WORD    rel_index   :14;
    BYTE    f_free      :1;
    BYTE    f_phys_cont :1;
    WORD    next;
    WORD    prev;
    WORD    owner_pid;
}MB,*P_MB;
```

The chain ID is not stored in the block list. It is returned as an integer but internally is a 32-bit offset to the block list pointing to the first entry of the chain.

With this configuration, OS/90 can access 1GB of physical memory minus any memory holes.

If `prev` is zero, this is the first.

A `rel_index` of zero indicates the first entry.

The `owner_pid` is relevant for userspace allocations after a program terminates. In this situation, all of its memory must be freed. A function called `DeleteProcessMemory()` is called by the scheduler. If `owner_pid` is zero, it is the kernel since that is not a valid PSP.

An `MB` represents a single block of 16K. This size can be reconfigured. The block is the basic unit of virtual memory in OS/90.

# Virtual Memory List

Memory can be allocated by the kernel and then mapped to a fixed region, but this is not very useful since we may want to reuse regions of our limited addressing space. To do this, we need to be able to allocate virtual addresses.

The kernel and user allocate these regions differently, but use the same chain to hold page tables. The first few page tables allocated there are for kernel PTs.

## Relationship With Uncommitted Memory and Swapping

It is important to know what ID a certain virtual page belongs to.

# Framebuffer/EMS Emulation API

16-bit DOS programs will just access the framebuffers. The single address space makes this a challenge because it cannot possibly allow multiple programs to have different versions of the same memory.

The solution is that each process has something called a process hook, which is a procedure called every time the process is scheduled, if a process hook is specified. This is called inside an atomic context. The process hook can remap the memory for the individual process.

If the memory manager lock is currently

# Rationale

## Fixed Blocks as the Unit of Paging

OS/90 allocates all memory in a fixed block size. Page attributes and virtual memory only work with block quantities. By default, the block size is 16KB. The size of the block does not need to be known by any kernel mode software. A different method of allocating page frames could be used and software would behave the same, since all the functions exposed by the kernel use byte counts and 32-bit addresses.

This design choice has advantages and disadvantages.

Pros:
* Memory allocation is very fast
* MM is simple
Cons:
* Memory is wasted due to internal fragmentation
* More disk IO when swapping
* Virtual MMIO is complicated

## Single Address Space

OS/90 is a SASOS, which is unconventional for 32-bit operating systems. It has several advantages as a result.

* Less memory used since we do not have to store page tables per-process
* CR3 is never changed on context switches, no TLB flushing needed
* More efficient use of cache lines and the TLB
* Reduced code complexity
* Simple IPC

The disadvantages are:
* Addressing space limited to 3GB for all processes. Ought to be enough for anybody tbh :)
* Significantly reduced stability

The address space is still three times larger than the maximum physical memory.
