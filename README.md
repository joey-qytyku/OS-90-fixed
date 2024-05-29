# Introduction

OS/90 is a work-in-progress 32-bit operating system designed to be binary compatible with MS-DOS. A UI is being worked on alongside the kernel.

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

## What type of kernel is used

The kernel is monolithic and modular, with some inspiration from the exokernel in that there is a focus on "securely" arbitrating resources.

Programs run inside shared address space virtual machines and various behavior. The kernel loads subsystem drivers which act as hypervisors or translation layers that allows programs to run using the native DOS interface.

Any component of DOS or the BIOS can also be trapped to provide a 32-bit implementation. This is what makes OS/90 a true operating system.

## What does the name mean?

The name was partially inspired by IBM OS/2, although OS/90 spits on the OS/2 orthodox layered design at every opportunity (seriously).

The 90 was chosen because OS/90 was meant to be for 90's era computers. This is no longer the case, so 90 means nothing specific. (I simply found targetting an entire decade of hardware with a hobbyist operating system to be unreasonable.)

Now the name means nothing in particular, but it kind of stuck.

## What version of DOS should be used?

FreeDOS is recommended and is actually used for debug builds.

## Is OS/90 an operating system or an operating environment?

OS/90 is an operating system in the same way that Windows 95 is. It just happens to use services from another one (DOS).

## Why does the source code have strange characters?

The source code uses codepage 437 characters to make it viewable in DOS. Visual studio code supports CP437. Make sure to select it when viewing the code.

## Where can I find documentation for driver development?

The docs folder has some information about the kernel design and some API details. The best documentation is still the source code.

The documentation will at no point will be perfect or complete, so feel free to send me an email.

## What subsystems are planned?

32-bit DPMI and DOS subsystem is a priority.

## What are the Licensing Terms?

Most of the code is GPLv2 licensed. Anything else is dedicated to the public domain. See the file LICENSE for details.
