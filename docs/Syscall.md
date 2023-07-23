# Preface

The system call interface is not meant to implement an entire operating system interface, but to provide supplementary features to the existing DOS/DPMI API for OS/90 support.

Programs designed for OS/90 should use the userspace API. The system call API is very low level and subject to change at any time.

# Call Method

Vector 41h is reserved for OS/90.

# List of System Calls

DPMI and native programs can both access the system call interface. It is essentially just an extention on top of the existing DOS/DPMI interface.

The function code is 16-bit, where the high byte is the functional group and the low byte is the function.

Group | Name
-|-
AA | Program Control
B0 | Memory Manager
E0 | Events and Signals
|

## Installation check: INSTCK

```
Code = 0AA55h
OUT:
    EAX = Underlying DOS version code
    EBX = OS/90 version code
    CL  = Processor version
```

The function code is a reference to the PC boot sector. Versions are in BCD (e.g AH=6, AL=22). This is necessary in programs that could execute in a DOS-based DPMI server.

CL will equal 3 for 386, 4 for 486, 5 for Pentium, or 6 for Pentium pro.

## Memory Block Size MEMBLKSIZE

Future versions of OS/90 may not use pool allocation, in which case, a 4096 byte block size is reported. This value will always be a power of two.
