## Pages In Detail

### Page Flag Protocol

The bits in a page table entry are refered to as page flags.

In all functions that take page flags, the following rules are to be followed unless otherwise notes:
- Present (PG_P) is always ignored.
- Accessed and dirty can be used

The page flag modifier value is 32-bit. The high 16 represent the "bother with" flags. The low 16 represent the value to set them to, only if a 1 exists in the corresponding bother with flag.

This means that it is possible to "only bother with" certain bits while leaving the others alone.

To make all of this simple, macros are defined beginning with `PAGE_` that define the bother with flag AND the new value. For example `PAGE_READ_ONLY`

The following are supported
```
Bit On             |    Bit Off
-------------------|--------------------
PAGE_READ_WRITE    |    PAGE_READ_ONLY
PAGE_USER          |    PAGE_KERNEL
PAGE_WRITETHROGH   |    PAGE_WRITEBACK
PAGE_GLOBAL        |    PAGE_NOT_GLOBAL
PAGE_DIRTY         |    PAGE_CLEAN
PAGE_PRESENT       |    PAGE_NOT_PRESENT
```
> There are more options related to the Page Attribute Table. This is not complete.

### Page Modifier Index (PMI)

There are three available bits for all page tables. This is treated as a 3-bit integer that represents a mutually exclusive trait about the page.

The use of incompatible architectural page bits and PMIs is automatically corrected using the Page Flag protocol, but it is highly recommended that this document be consulted.

Here is a table describing all PMIs.

> "Requires" means bits that must be turned on.
Key:
- Requires: these bits must be on or off as described
- ~: Negation, bit is not on
- P: present
- ALLOC: Mapped to chain memory. Accessible by software.

|Name           |Requires       | Description
-|-|-
TRANSIENT       |P              | Mapped memory page that is NOT LOCKED and can be swapped to the disk. Page is present.
AWC             |~P             | Uncommitted memory. Contents are the chain to resize according to relative location.
HOOK            |~P             | Page that is 100% handled by the kernel directly upon page fault.
TRANSIENT_OUT   |~P             | Page that is swapped out. 20-bit value is the swap file page index.
COLAT           |P, ALLOC       | Allocated and mapped page that is not locked and is deallocated with notice when memory is out.
VAHDR           | --- | Internally used to allocate virtual address ranges.

- A PMI of zero excludes all of these.
- All pages are locked by default.
