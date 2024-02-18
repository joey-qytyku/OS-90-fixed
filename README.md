# Introduction

OS/90 is a work-in-progress 32-bit operating system designed to be binary compatible with MS-DOS.

OS/90 is capable of advanced preemptive multitasking and virtualization of just about any 32-bit operating system on an 80386 processor with at least 1MB of memory.

# Installation and Use

OS/90 releases are ZIP files. The entire OS goes inside the OS/90 directory in the root.

## Boot Media

OS/90 can be booted from a floppy disk or a hard disk.

## Boot Options

There are several bootloaders to choose from which are inside the OS/90 directory.

- OS90.COM:    Load kernel in extended memory
- OS90LOW.COM: Load kernel in conventional memory
- OS90HMA.COM  Load kernel in high memory area

## Installing Drivers

If a DOS driver is not sufficient for proper device support, a device driver can be used. Drivers are `.DRV` files. Any drivers placed in `OS90\DEVICES` will automatically be loaded.

Adding support for different subsystems is also done using drivers, which are instead placed in `OS90/SUBSYS`.
