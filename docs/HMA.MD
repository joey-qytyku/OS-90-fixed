# High Memory Area

The high memory area is reserved for the kernel. This document is here for future reference in case the topic of the HMA comes up and it needs to be used for a certain thing.

```
== Map of the HMA ==

100000-102FFF Startup page tables, reclaimable (8192 bytes)
103000-107FFF Real mode switcher (512 bytes)
10FFF0        Kernel startup stack start (512 bytes down)
```

- Stack-based regions will use a stack pointer that is over the limit because ESP is decremented before pushing on 80x86.
- The startup page tables continue to be used after boot for mapping the kernel. The 1M region is not available until MM is set up.
- The SV86 stack is the only part that contains ring-3 pages. It is aligned at a 4K boundary.
- The startup stack is used very briefly.
