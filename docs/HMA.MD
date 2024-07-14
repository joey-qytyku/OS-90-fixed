# High Memory Area

The high memory area is reserved for the kernel. This document is here for future reference in case the topic of the HMA comes up and it needs to be used for a certain thing.

```
== Map of the HMA ==

100000h-101FFFh Startup page tables, reclaimable (8192 bytes)
102000h-1021FFh Real mode switcher (512 bytes)
1022F0h-1025FFh Switcher stack (1024 bytes)
103000h-107FFFh SV86 default real mode stack (20KB)
10FFF0h  Kernel startup stack start (512 bytes down)
```

> Note: stack-based regions will use a stack pointer that is over the limit because ESP is decremented before pushing on 80x86.
