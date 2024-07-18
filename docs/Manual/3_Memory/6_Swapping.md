# Virtual Memory

OS/90 swaps only when it is unabled to complete a memory allocation request and commit the expected number of pages. An out of memory handler stack is used.

> I should add a way to monitor chain allocation from a driver. This could allow me to write an independent but efficient swapper.

```
MMCall(MM_ALLOC, 1000)
```
