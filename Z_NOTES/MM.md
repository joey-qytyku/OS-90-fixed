# Memory Manager 4th Edition

Updated July 19, 2025

/*
	XMS is supposed to allocate all available memory for the kernel and
	everything else. This linear block is safe to use by OS/90.
	XMS is never supposed to allocate over the ISA memory hole because then
	we lose a bunch of memory in between the HMA and the hole due to
	allocating a massive chunk that cannot fit in there.

	As a result, memory must be "correctly" detected using the BIOS
	interface.

	The following interfaces exist:
	- INT 15h,AH=88
		This returns the number of 1K blocks after the real mode memory.
		The only problem with this is that the size reported may not
		actually reflect the true amount. Some BIOSes do not correctly
		report the actual size and are capped to 15M.

		Also recieves data from the CMOS ram, which means it can all be
		done without calling anything.
	- INT 15h,AH=0C7h
		This returns a structure which describes a bunch of memory
		information.
		Windows 95 does not support this function even if available.
	- INT 15h,AX=0E801h
	https://fd.lod.bz/rbil/interrup/bios_vendor/15e801.html#sect-1776
		- Tells the amount of 64K blocks above the hole
		- Used by most OSes if E820 is not supported.
	- INT 15h,AX=0E820h
		- Every ACPI BIOS has this. Phoenix BIOS v4 did it first and is
		used on some 486 boards.
		- One way to do this is to pick out the top-2 largest memory
		regions and go with those: the pre-memory hole region and the
		above-hole region.
*/

## Terminology

CM - Conventional memory. The RAM starting from address zero reported by `INT 12H`. No larger than 640K. It IS allowed to be less than that.

PCM - Physical conventional memory. The memory used by the physical range of CM. Part of the PCM is mapped at all times for DOS support.

VCM - Virtual conventional memory. OS/90 uses extended memory for DOS programs by swapping out the VCM mappings.

UMA - Upper memory area. The address space above CM and within 1M.

EBDA - Extended BIOS data area. Detectable in the BDA and takes space after the officially reported CM.

ECM - Extended conventional memory. The use of non-standard address ranges in CM allocation that allows the virtual UMA to be transparently allocated.

MCB - Memory control block. Used by DOS and not OS/90.

Page frame (PF) - A physical page.

Page - Usually refers to a virtual page in the address space



## Initialization

### Use of XMS

OS/90 must load in extended memory before the ISA hole, all within the 24-bit addressable range. This means the XMS driver must not be allowed to access more than 15M using the appropriate option.

OS/90 allocates the largest available block that fits in the first 15M. We assume that the HIMEM driver is properly configured but extra checks could be added.

> FreeCOM allocates extended memory to swap inactive programs and keep copies of itself out of conventional memory. This is done very aggressively, and may create problems for multitasking.
> In fact, this is totally a waste of effort because most likely the conventional memory IS extended memory.

> Is the physical CM used better with multiple CM addr spaces

An interface provided by HIMEM.SYS and FDXMS is used to get the internal control blocks for the XMS block, which contains the size and base address. This is undocumented and not part of the far call interface.

https://www.ctyme.com/intr/rb-4767.htm

### Detecting Memory

Memory detection has to be properly done for any additional memory above 15M. It is assumed the XMS driver properly detected the low extended memory.

EISA introduced a method very similar to the old one. INT 15h,AX=0E801h tells how many 64K blocks exist above the ISA memory hole, although it is supported by other systems with different buses.

The memory hole is assumed to exist.

The approach is:
- Standard detection mechanism for all XM under 15M
	- If the value returned is greater than 15M, we ignore it
	- If value less than 15M, that is used.

- "EISA" detection for memory above ISA memory hole

> E820 is currently not supported because only 2 zones currently exist.

## Virtual address space and page tables

The kernel has an address space at 0xC0000000. This leaves 1GB of addressable memory. Everything else, including drivers, heaps, user memory, etc. are allocated in a virtual range at the first 4M and extends all the way to the largest size the kernel is configured to use.

OS/90 is a mostly shared address space system, although there are protection mechanisms in userspace and the kernel to avoid common errors.

The page tables are allocated at boot time and cannot change size. They are side-by-side and can be accessed as an array.

> The virtual address space only needs to be as large as the disk backed store and the main memory. This is the default behavior. Allocating 500M on a system with 4M makes no sense unless there is somewhere to actually store it.
> Fragmentation in the allocation method may necessitate a larger address space, however.

### Page table extended attributes

The page tables use the extra bits available in a PTE for various mutually exclusive properties. There are 3 bits available allowing for 8 possibilities.

> Remember that multiple programs can wait on the same page, especially when the page is ring-0 and owned by a driver.

`PG_UNC` means the page is uncommitted and is to be RAM backed. It may still be disk backed ultimately.

It is also marked not present. Bits [8:1] and [31:12] are merged into a single 28-bit value. *TODO DEFINE*

`PG_FBP` means file backed page used for demand paging. Also not present. The 28-bit value is split into an 8-bit compressed DOS file handle (that is all DOS needs to access the SFT), and a 20-bit 4K block index to the file data. This allows for accessing a 4G file, which is the FAT limit. This is used for swapping when pages are evicted.

`PG_FBC` means file backed page cache.

`PG_VMH` is virtual memory header. Access is illegal in all modes. The 28-bit value has an 18-bit value for the next index and a 2-bit field for the index within the header for validation purposes. There are three of these entries.

`PG_VMF` is virtual memory footer. Access is illegal in all modes. Contains no metadata.

### Allocation

Virtual memory regions have a header and a footer pair within the page table entries. These are special entries which are illegal to access in ring-0 and ring-3. These are used for basic allocation.

## Physical memory allocation

Allocation adheres to the following design constraints:
- Does not waste a single page. All extended memory is allocatable even with an added cost.
- Fast average allocation
- Provides all features required by DPMI 0.9

### Zones

Physical memory is separated into two zones, the DMA addressible zone and the above ISA memory hole zone. Additional memory is not supported.

Drivers that may interact with ISA DMA should allocate a buffer in the low zone. If it is found the buffer is not needed later, the memory can be deallocated. This must be done in early boot and can sometimes fail.

The swap space is also considered a zone.

> Allocations may span multiple zones

### Page frame allocation

Each allocation is a mapping of possibly discontiguous page frames to a single flat virtual address range. It is not possible to map multiple allocations to the same virtual memory region. Allocations are identified by the address of the first byte.

Allocations can be partially uncommitted and UCPs can either be at the top or bottom depending on the configuration of the allocation. Uncommitted pages are not supported by DPMI 0.9 and OS/90 does not have all the capabilities specified by DPMI 1.0.

An array similar to the DOS MCB format is used to represent groups of page frames. It is similar to a linked list, but uses a size rather than a pointer to get to the next entry.

```
typedef struct {
	UINT	count:20;
}MRANGE;
```

### Uncommitted Memory

UCMs cannot be made uncommitted after they are committed.
UCMs cannot be locked becuase they are not in memory.

UCM is used transparently to reduce memory use.

## Real mode memory

The real mode memory is partially shared for basic compatibility and also has a pool allocated for "bank switching" between DOS programs. The same exact program segment prefix address is passed to DOS even though a different program is running, which negates having to allocate a real PSP.

Swapping the DOS memory is accomplished by changing a single PDE to the page of a different process to avoid a large copy.

OS/90 transparently allocates memory in the Upper Memory Area and the HMA, allowing for IBM PC incompatible memory configurations. Potentially 700K can be available to a DOS program using standard methods, but this can be disabled.

### Emulation of DOS allocator

The List of Lists structure is not updated for the current MCB chain, so MCBs are not accurately emulated.

This means MEM.EXE gives wrong results and may also fatally malfunction. Instead, it is captured by the monitor and the relevant information is revealed there.

