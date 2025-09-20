# Extended Memory Manager

The OS/90 memory manager supports:
* Demand paging (e.g. loading a program larger than RAM)
* Single address space
* Uncommitted memory
* malloc/free

RAM and page file space are indexed in an array called the MAT or Memory Allocation Table.

If there is no more memory, demand-paged space is allocated. A demand paging window is used to achieve orthogonality.

> Swap is an incorrect term. OS/90 does not swap.


# General Description

Memory blocks are alloated as non-resizable chunks with fixed and non-resizable virtual address ranges. Inside the page tables, there is a header table entry and a buffer break entry.

> Keep track of free and used page table entries in variable.

Uncommitted memory is memory that is to be allocated when a virtual page is accessed so that it does not all have to be reserved at once. The uncommitted status in indicated using free bits in the page tables. Using iteration, the allocation in need of extension is found, and unaligned addresses are also accounted for to improve performance.

There is a single page table for the first 1M of memory with the rest being reserved, another for the kernel, and any number for extended memory that is allocatable.

# Thread Safety

Each allocation is protected by a mutex lock inside the header. All operations must run in T2 unless otherwise specified.


# Error Codes

This can easily be stringified by calling M_ErrToStr.
