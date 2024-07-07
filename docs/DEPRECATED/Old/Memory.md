# Deprecation Note

The memory manager is being redesigned. This document was too convoluded to maintain and has been deprectated.

# OS/90 Memory Management

OS/90 supports:
* Virtual memory with swapping and uncommitted memory
* Per-process resizable heaps (sbrk-style)
* Dynamically allocated off-heap memory (mmap-style)
* Memory mapped IO emulation for framebuffers and Limulation

## Constructing Page Tables

The i386 processor uses 2-level paging for the MMU.

Page directory entries are never marked as not present on OS/90. Only pages are. If a page directory entry is not present, the meaning is that an associated page table does not exist. Such a page table entry will be set to read only/cache disabled/ring 0 and will map to the address 0xFFFFF000 by default. The page fault handler cannot know that the not present violation was caused by a page directory specifically, so that is why it is necessary to reserve this address.

Access to unmapped memory like this is always an illegal operation. The reason why we keep track of whether or not a page directory points to a valid page is for when we need to map virtual addresses to physical addresses, which may require the allocation of new page tables.

The page directory is always present in OS/90 and is globally shared. Task switching is accomplished by writing local page directory content to the global one and refreshing the TLB.

## Page Faults and Reentrancy

Page frame allocation functions are not reentrant like the rest of the memory manager, but they are garaunteed to not cause page faults, which means they can safely be called within a page fault handler. They will check the memory manager lock before acquiring it.

## Memory Blocks

Blocks are the single unit of physical virtual memory. Memory mappings are always based on the block size.

All physical memory is in 16K blocks starting from zero. This option can be configured in the config header file. This is a very simple and fast way to manage pages, but is not very space efficient.

```c
// 10 bytes long
tpkstruct
{
    CHID    idnum       :13;
    BYTE    f_free      :1;
    BYTE    f_phys_cont :1;
    BYTE    f_is_first  :1;
    WORD    rel_index;
    WORD    next;
    WORD    prev;
    WORD    owner_pid;
}MB,*P_MB;
```

This structure is an element of the block list. To support memory allocations going backward, as in page frames do not go in forward direction as virtual pages, a link to the next element is included. With 16K blocks, the maximum physical memory accessible is 1GB. This configuration can be modified.

This list is doubly linked so that chains can have blocks swapped out. An index is also kept for this purpose.

### Rationale for Fixed Blocks

Windows NT requires 32MB of RAM for typical use. It also allocated memory is 64KB quantities.

```
   32MB
  ------ = Number of blocks
   64KB
```

Now we can set this fraction equal to the one with our block size.

```
    32MB       X
  ------- = -------
    64K       16K

```
Solve for X, which is the size of the memory in megabytes.

64KB = 0.625167847 MB
16KB = 0.156291962 MB

0.625167847(X) = 32 * 0.156291962
0.625167847(X) = 5.001342784
X ~ 8.000000013

This means that an allocation of 64KB on our NT system hurts as much as an allocation of 16KB on a system with 8MB of RAM. This sounds bad, but Windows NT was known to be a RAM hog and OS/90 is a much simpler architecture without any of the cross-platform bloat.

Windows NT does have the ability to swap out or uncommit individual pages, which OS/90 cannot do.

### Relationship with Virtual Memory

Because we allocate using fixed blocks, it is also necessary to swap memory with the same size so that freed memory can be allocated later. Page granular operations are not very useful as a result, and OS/90 does not support them. Page-level settings (bits) always apply to an entire block. Memory mappings are also done at block boundaries.

Functions provided by the kernel API are byte granular so that the block size does not have to be obtained at run time (it can be).

This increases the amount of disk IO when swapping. Cache usage is not affected because the extra memory allocated is only fetched to cache when used.

## Virtual Address Space Allocation

### Virtual Address Space List

A linked list of structures of the type `VAHEAD` contains arrays which specify the mappings of physical blocks to virtual blocks.

Each entry is formatted like this:
```
#define VAF_FREE
#define VAF_USER

tpkstruct
{
    WORD    id;
    BYTE    flags;
}VA_ENT;
```

We do not use bit arrays for a few reasons. First of all, it does not allow memory to be deallocated on program termination. Secondly, it does not allow deallocation without knowing the size of the allocated region.

### Relationship with Swapping and Virtual Memory (OLD)

OS/90 supports evicting memory to the swap file.

The point of virtual memory is that pieces of the main memory can be copied to the disk to give the appearance of more RAM. The implementation is a bit more complicated because we need a way to actually access the RAM that has been freed up.

Originally, bit allocation was considered for representing virtual address spaces, where an on bit means the page is taken. The question is how we are supposed to implement virtual memory with swapping. The block list can only represent physical page frames. In order to increase available memory, we must free blocks from certain chains. The only way this can work is to swap the top blocks to the disk and resize the allocation. This presents a problem because something needs to be done when the freed memory is allocated.

We can make any block in the chain deallocated by severing its links and making it free. The block is no longer belonging to its chain. This would require a doubly linked list. Chain mapping must be able to generate not-present pages. Cache the chain-relative index of the block entry. The mapper will calculate a delta between the current block and the last block to determine the number of not-present pages to generate.

We need to be able to reconstruct the virtual address space. When a block is swapped out, the physical block may be allocated by another program. When it is swapped in, the __specific chain__ which it belongs to must be resized. We also need the ability to reconstruct the original address space so that the block goes exactly where it is expected.

## Uncommitted Memory

Uncommitted memory has to be supported in situations where software allocates a lot of memory that it does not need to use at once. An uncommitted page does not have a disk location of physical page frame, which means it does not exist until accessed for the first time.

Because we use pool-based allocation, a single uncommitted page is not possible.

OS/90 does not allow arbitrarily placed blocks to be uncommitted. They must be directly after committed blocks in the address space. When an uncommitted block is accessed, every block before it belonging to the same chain is automatically allocated. The algorith is that upon a page fault, we scan each block of pages for the uncommitted flag and if we find one that is not uncommitted, we can get its chain, do a resize, and finally map it to the same region.

The problem with uncommitted memory is that it causes significant performance hiccups. OS/90 predictively allocates the block after the page-fault-generating uncommitted memory if it is also uncommitted. This reduces the slowdown caused by uncommitted memory access for upward sequential access, but does cause slightly more memory to be used.

A solution is to allow the block list to store uncommitted block candidates. What?

## Restrictions on Allocating Memory

Interrupt service routines may never call anything that modifies the page tables or other memory managment structures. The kernel API for memory managment is not reentrant and runs with IRQs enabled most of the time, so calling MM functions will not work safely in an ISR.

# DPMI > Will remove

The kernel only supports the features that DPMI 1.0 supports for virtual memory. The userspace memory manager is responsible for implementing heaps and other features. While OS/90 is mostly DPMI 0.9, it supports some 1.0 features.

We will address each feature that DPMI calls for and describe how OS/90 implements them.

## Page Locking

DPMI allows memory regions to be locked so that they are not paged out by the memory manager. The spec defines locking with referrence counting, where a page is not actually unlocked until the counter is decremented to zero.

I think this is because interrupts will probably try to lock memory before using it (though OS/90 does not require this, DOS programs will do it anyway) and that memory may already be locked. If the interrupt service routine unlocks the memory, it may not be in the state expected by the main thread of execution.

But OS/90 fakes interrupts and does not require programs to lock memory while handling them. Interrupts at the process level are simply changes in program flow. On a single-tasking DPMI host, interrupts will be called directly from a ring-0 ISR, which means that they cannot cause page faults.

Locking can be implemented using a reserved bit inside the page table entry. The problem is how we are supposed to implement the lock counter. There is no explicit specification for how many locks it needs to be able to handle and there are no ways to get the current count; the program simply needs to "know."

A solution is to make the functions calls for page locking succeed unless the ranges specified are wrong. Since page locking does not influence program behavior, there is no reason for a program to fail or not because of it. Software that locks pages will run as expected.

Another solution is to actually perform the locks, but behavior may be unpredictable if we do not use a counter.

## Real Mode Locking

DPMI allows programs to lock memory in the 1M real mode region. The entire 1M+64K region is always present in memory. These functions will always return success because we cannot do that.

## Get Page Size

4096 is the page size.

## DPMI 1.0 Memory Allocation

OS/90 implements some DPMI 1.0 functions for the userspace.

DPMI 1.0 supports two types of allocators:
1. The original, which does not garauntee page alignment and only generates comitted pages
2. The new allocator which can manually specify base addresses and be resized with uncommitted memory

### Allocating Linear Memory (1.0)

The handles returned to the program from DPMI allocation services are 32-bit block IDs for the kernel.

### Resizing Linear Memory (1.0)

The update segment descriptors feature is not supported. This function maps directly to page frame allocation.

There is one minor problem, though. This function will change the base address if it makes the block larger. OS/90 uses an undefined bit to bypass this limitation. If bit 31 of the flags is set, it will resize and map to the same location, garaunteeing that the virtual base address will not change. This will allow for implementing `sbrk`.

## DPMI 0.9 Memory Allocation

The old DPMI 0.9 functions can only garauntee paragraph alignment and behave a bit more like `malloc` but with handles instead of pointers. We cannot possibly allocate in such quantities. It uses chain allocation too, but will never generate uncommitted memory.

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
