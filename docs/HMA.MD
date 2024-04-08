# High Memory Area

The high memory area is reserved for the kernel. This document is here for future reference in case the topic of the HMA comes up and it needs to be used for a certain thing.

Map of the HMA
```
100000h-1003FFh Startup page tables (1024 bytes)
100400h-1005FFh Real mode switcher
100600h-1009FFh Switcher stack (may need more. BIOS does have its own stack?)

10FDF0h-10FFF0h  Init stack (512 bytes)
```

