# M_PokeHole

```c
int M_PokeHole(uint page_frame, uint npages);
```

This is a function which facillitates the creation of memory holes primarily for talking to ISA devices.

It copies all page frames in the range specified to another allocated set of page frames, and then adjusts every single page table entry related to it so that whatever program which was using it does not notice.

M_PokeHole may trigger page faults or uncommitted memory allocations, making it extremely slow, along with the large transfer of pages.

This function blocks simultaneous access.

Unless using this to block access to a static resource, NEVER call this function! Report to the user where the memory hole is so they can block it off in their config (OS/90 does this with PnP support). It is also okay to write to the config.
