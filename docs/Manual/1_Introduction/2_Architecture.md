# Architecture

## The Kernel

The kernel is the `OS90.DAT` file. It is a physical load address-independent executable that is loaded by a DOS program into extended memory. The kernel is monolithic and modular, and uses DOS services.

## Drivers

Drivers are executables of a custom format with the file extension `.90` which are loaded into the allocatable region of memory (they do not share with the kernel). They are able to reference all API funtion calls by symbol name.
