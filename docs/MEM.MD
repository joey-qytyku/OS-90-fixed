# Memory Manager 3

The memory manager of OS/90 contains the following components:
- Page frame allocation
- Page table mapping generation
- Swapping to disk


# Address Space Layout

OS/90 can access about 2GB of physical addressing space in theory, but is limited by memory holes and other things which are not exposed directly to software.

The kernel is not mapped to `0xC000_0000` like most kernels, but to 0x8000_0000. This is because OS/90 has a Raw Memory Region, which is the entire phsyical memory mapped to a specific region.

The address space layout is as follows:
```
Conventional and UMA    0
HMA                     0x100000


Kernel                  8000_0000
Raw Memory Region       8040_0000

```

The HMA is always present and is critical to the operation of the system. It will be impossible for DOS programs to have a virtual HMA as of now. The remaining 16 bytes not available to DOS are also included.

The kernel takes 4 MB of the address space. Drivers do not use its space and run in the rest of the extended memory.

The allocatable region begins at the 4MB mark of the address space.

# Page Frame Allocation

A table called the PBT (physical block table) is allocated directly above the kernel to accomodate every physical page frame between the address zero and the highest possible RAM that can be addressed. Memory holes are included.

Conventional memory, the HMA, and the ISA memory hole are always unavailable.

Each entry follows this structure:
```
tpkstruct
{
        WORD    rel_index   :14;
        WORD    f_inuse     :1;
        WORD    f_phys_cont :1;

        short    next;
        short    prev;
}MB,*P_MB;
```
This is not standard and can change.

- rel_index is the index of the entry relative to the chain.
- f_inuse indicates if this block is currently being used
- f_phys_cont indicates that this block is part of a chain that is phsyically contiguous in memory
- next,prev are linked list indices

Each MB is eight bytes long.

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
```

# Page Tables and Virtual Address Spaces

> I may need a separate linked list implementation for this to be efficient.

The page tables needed for the allocatable region are dynamically allocated and are based on the position requested for mapping for chains. Any mapping within the ALLOC region can be requested.

OS/90 uses a 64-bit linked list to keep a list of mappings. A single mapping is a pair. The first value has the base address, some flags, and the size of the mapping. The following entry is the chain and the reference count.

OS/90 uses reference counting garbage collection for page tables which are no longer used. Each time a mapping range is used by a chain, the count is incremented. This allows one mapping to be used for more than one chain. Each chain inside it is called a resident chain. When all residents are removed, the page tables are expunged. The mapping remains, however, and the default state on creation is zero residents.

```
int ReserveMapping(PVOID address, LONG bytes, LONG flags);
int DeleteMapping(int mapindx);
```

> Is this too complicated? Also, how do we deal with page directory entries?

> DPMI 0.9 does not support mapping to a forced location. Allocations also only happen on 16-byte boundaries. I WILL support uncommitted allocations, but DPMI will not be able to use it.
