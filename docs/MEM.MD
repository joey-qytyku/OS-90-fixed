# Memory Manager 3

The memory manager of OS/90 contains the following components:
- Page frame allocation
- Page table mapping generation
- Swapping to disk

# Definitions

VAS: Virtual address space

RMR: Raw memory region. A flat representation of the physical RAM from zero to the last RAM byte. Located at 0x80400000.

AMR: Allocation mapping region. Where pages requested from the kernel are mapped. Excludes the first 256+16 pages of the address space.

# Address Space Layout

OS/90 can access about 2GB of physical addressing space in theory, but is limited by memory holes and other things which are not exposed directly to software.

The kernel is not mapped to `0xC000_0000` like most kernels, but to 0x8000_0000. This is because OS/90 has a Raw Memory Region, which is the entire phsyical memory mapped to a specific region.

The address space layout is as follows:
```
Conventional and UMA    0
HMA                     001_00000
AMR                     001_10000
Kernel                  8000_0000
Raw Memory Region       8040_0000
```

The HMA is always present and is critical to the operation of the system. It will be impossible for DOS programs to have a virtual HMA as of now. The remaining 16 bytes not available to DOS are also included.

The kernel takes 4 MB of the address space. Drivers do not use its space and run in the rest of the extended memory.

The allocatable region begins at the 4MB mark of the address space.

# Page Frame Allocation

A table called the PBT (physical block table) is allocated directly above the kernel to accomodate every physical page frame between the address zero and the highest possible RAM that can be addressed. Memory holes are included.

Conventional memory, the HMA, and the ISA memory hole are always unavailable.

> NOTE: We can define a prev of zero to indicate not in use, since that would be invalid. The conventional memory PBT entries can be totally ignored.

Each entry follows this structure:
```
tpkstruct
{
        WORD    rel_index;
        short   next;
        short   prev;

        // What else?
}BTE,*P_BTE;
```
This is not standard and can change.

- rel_index is the index of the entry relative to the chain.
- f_inuse indicates if this block is currently being used
- f_phys_cont indicates that this block is part of a chain that is phsyically contiguous in memory
- next,prev are linked list indices

Next and prev being the exact same indicates a free entry since such a combination is impossible.

Each BTE is eight bytes long.

The relative index is needed for implementing virtual memory. Any block can be swapped out (except for first?), so it is necessary to remember the position so that the missing blocks can be replaced.

# Chains

The PBT is used to generate chains to represent allocations. The mappings they represent need not be physically contiguous and page frames can also go backward.

next and previous are both signed values. If they are negative, they cannot be used as array indices for obvious reasons and are interpreted diferrently.

Negative values of `next` represent the number of uncommitted pages that proceed the final block when mapped to memory.

When the page fault handler detects an uncommitted page access, it has to find out what chain needs to be extended.

Uncommitted pages can only exist after committed pages. Arbitrary uncommitted pages are not possible.

Functions provided:
```
CHAIN CAlloc(LONG bytes_commit, LONG bytes_uncommit, LONG flags);
VOID  CFree(CHAIN ch);
int   CQUery(CHAIN ch, LONG flags)
PVOID CWalkRMR(CHAIN);
```

# Virtual Address Spaces

OS/90 preallocates every page table needed for the virtual address space. A 1:1024 ratio exists between page tables needed to represent and pages represented. This makes it quite viable to do this and avoids using any complicated methods of keeping track of address spaces.

The kernel has a configurable option for the extended memory page frames to virtual address space pages conversion ratio.

Preallocation means that the page tables themselves can be traversed to reserve and generate mappings.

# Page Hooking?

It would seem impossible for a single address space OS to be able to run DOS programs that directly access the video memory, but it is in fact possible.

By checking the memory manager reentrancy lock, it is possible to call a remap procedure at the right time to "bank swap" a mapping dynamically. To avoid interference with OS/90 software directly accessing VGA, it is necessary to restore the original identity mapping.

