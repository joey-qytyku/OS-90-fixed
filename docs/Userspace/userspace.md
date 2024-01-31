> This document is old.

# Overview

OS/90 has a cooperatively multitasking userspace. All programs run in the same linear address space. The userspace kernel is capable of yielding itself to other processes.

# Commands

drvgen: Convert ESS file to driver executable
drvins: Dynamically load a driver
drvrem: Uninstall driver
drvlst: Identify drivers
vdvlst: List virtual devices and emulated resources
meminf: Information about memory manager
irqown: Display owner of IRQs

## IRQOWN

Sample output:
```
 ___________________________________
|IRQ#| Type        | Owner Name
|----|-------------|----------------
| 0  | STANDARD_32 | KERNL386
| 1  | UNDEFINED   |
| 2  | STANDARD_32 | INPUT
| 3  | RECL_16     |
| 4  | RECL_16     |
| 5  |
| 6  |
| 7  |
| 8  |
| 9  |
| 10 |
| 11 |
| 12 |
| 13 |
| 14 | RECL_16
| 15 | BUS_FREE

Key:

RECL_16:      Reflected to DOS
BUS_INUSE:    32-bit handler
BUS_FREE:     No specific use
UNDEFINED:    Not to be used
```
## meminf

```
Physical memory:    2480 KB / 4096 KB
Swap:               0 KB / 1024 KB
Uncommitted:        0 KB
```
