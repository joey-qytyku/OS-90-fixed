# Architecture

## The Kernel

The kernel is the `OS90.DAT` file. It is a physical load address-independent executable that is loaded by a DOS program into extended memory. The kernel is monolithic and modular, and uses DOS services.

## Drivers

Drivers are executables of a custom format with the file extension `.90` which are loaded into the allocatable region of memory (they do not share with the kernel). They are able to reference all API funtion calls.

## Comparison With VxD and Win386

Differences:
- OS/90 has a reentrant kernel while VMM relies on various tricks to signal the safety of reentrance.
- VMM supports multiple address spaces while OS/90 only has limited support.
- OS/90 and VMM have drastically different memory managers
- OS/90 can simulate various operating systems while VMM is designed for DOS and Windows
- OS/90 is written in C and assembly while VMM is (presumably) written entirely in assembly.

Similarities:
- Both have a concept of a virtual device driver
- Both are centered around emulation
- Both are multitasking and use 1MS-granular time slices
- Both are intended to be as binary-compatible with 16-bit DOS as possible
- Both can use real mode legacy drivers

