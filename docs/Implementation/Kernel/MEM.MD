# Memory Manager 3

OS/90 is a single address space operating system.

## Definitions

VAS: Virtual address space.

PWR: Page window range. A range of memory where individual page frames are mapped. This is almost exclusively used for mapping task blocks, but the memory manager has no knowledge.

AMR: Allocation mapping region. Where pages requested from the kernel are mapped. Excludes the first 256+16 pages of the address space.

UMA: Upper memory area. This is all of the 8086-addressable memory that is above 640K and is mostly reserved for the BIOS and VGA framebuffers.

HMA: High memory area. 65520 bytes accessable by using a real mode segment of `FFFF` and using an offset of 16 or greater. Addressable when the A20 gate is enabled. OS/90 defines it as a full 65536 bytes, including the 16 bytes not accessible in real mode.

## Address Space Layout

OS/90 can access about 2GB of physical addressing space in theory, but is limited by memory holes and other things which are not exposed directly to software.

The kernel is not mapped to `0xC000_0000` like most kernels, but to 0x8000_0000. This is because OS/90 has a Raw Memory Region, which is the entire phsyical memory mapped to a specific region.

The address space layout is as follows:
```
Conventional and UMA    0000_0000
HMA                     0010_0000
AMR                     0011_0000
Kernel                  8000_0000
Page Window Range       8040_0000
```

The HMA is always present and is critical to the operation of the system. It will be impossible for DOS programs to have a virtual HMA as of now. The remaining 16 bytes not available to DOS are also included.

The kernel takes 4 MB of the address space. Drivers do not use its space and run usng the AMR.

The allocatable region begins at the 4MB mark of the address space.

## Page Frame Allocation

## Chains

## Virtual Address Spaces

OS/90 preallocates every page table needed for the virtual address space. A 1:1024 ratio exists between page tables needed to represent and pages represented. This makes it quite viable to do this and avoids using any complicated methods of keeping track of address spaces.

The kernel has a configurable option for the extended memory page frames to virtual address space pages conversion ratio.

Preallocation means that the page tables themselves can be traversed to reserve and generate mappings.

## Page Modifiers

> Tag blocks that are cached for instant writeback?
> TODO: document collateral paging, transfer content to MD.
