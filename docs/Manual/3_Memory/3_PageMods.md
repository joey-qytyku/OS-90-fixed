# Page Modifiers

Using the three available bits on page table (not directory) entries, the kernel defines modifier attributes that can affect the behavior of the page. See mm/page.h for full documentation.

Page modifiers represent mutually exclusive states of a page and some require that certain other bits are also configured correctly.

The only ones that are not reserved for the memory manager are:
- PTE_HOOKED
- PTE_COLAT_L0, PTE_COLAT_L1

PTE_HOOKED requires that the page is not present. It will call a procedure when a page fault is generated while trying to access it. Do note that accesses that cross page boundaries will generate two calls.

> It may be useful to store some extra information in non-present PTE_HOOKED page table entries. There may be a function for that.

PTE_COLAT also use a procedure, but only for cleanup so that the deallocated and given to another chain. This is used to implement memory caches.

PTE_COLAT_Lx all require that the page is present. It also HAS to be mapped.
