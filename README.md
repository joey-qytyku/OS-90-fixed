# Introduction

OS/90 is a work-in-progress 32-bit operating system designed to be binary compatible with MS-DOS. A UI is being worked on alongside the kernel.

OS/90 is capable of advanced preemptive multitasking and virtualization of just about any 32-bit operating system on an 80386 processor with at least 1MB of memory.

# Minimum Requirements

OS/90 has no specific memory requirement, but may fail at any point if there is too little. 1MB is enough for very basic use, but at least 2MB is recommended.

There is also no CPU requirement, but the i386SX may not be the best option because OS/90 uses a lot of 32-bit code.

A VGA adapter is required.

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

# FAQ

## What version of DOS should be used

FreeDOS is recommended and is actually used for debug builds.

## Is OS/90 an operating system or an operating environment?

OS/90 is an operating system in the same way that Windows 95 is. It just happens to use services from another one (DOS).

## Why does the source code have strange characters?

The source code uses codepage 437 characters to make it viewable in DOS. Visual studio code supports CP437.

## Where can I find documentation for driver development?

The documentation will at no point will be perfect or complete, so feel free to send me an email.

The entire kernel API is specified in a table. The kernel source code is very commented.

## What subsystems are planned?

There will be two DOS subsystems: a 16-bit only one and a 32-bit DPMI compatible subsystem (VDMBIO16.DRV and VDMBIO32.DRV).
