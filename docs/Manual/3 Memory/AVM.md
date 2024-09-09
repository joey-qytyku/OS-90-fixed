## Automatic Virtual Memory

Allocating memory requires the use of multiple chains, including swap and RAM, and also requires coordinating uncommitted memory. Virtual memory regions are allocated using Virtual Memory Region Descriptors (VMRD). This may change with an update so the feature must be detected.

> The goal of this interface is DPMI compliance.

The layout of a virtual region is the following
```
Committed memory pool
```
