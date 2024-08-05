# Introduction

OS/90 is a work-in-progress 32-bit operating system designed to be binary compatible with MS-DOS. A UI is being worked on alongside the kernel.

# Architecture Description

OS/90 uses a kernel that is not OS-specific except for a few calls to the underlying DOS. All operations needed by operating systems are caught by the kernel as exceptions for emulation by a subsystem driver, which is also responsible for creating and terminating tasks in its domain.

# Minimum Requirements

OS/90 has no specific memory requirement, but may fail at any point if there is too little. 1MB is enough for very basic use, but at least 2MB is recommended.

There is also no CPU requirement except for a 32-bit processor, but the i386SX may not be the best option because OS/90 uses a lot of 32-bit code.

A VGA adapter is required for the UI.

# Installation and Use

OS/90 releases are ZIP files. The entire OS goes inside the `OS90` directory in the root.

Installing the OS is done by creating an OS90 directory at the root of the boot medium and extracting the installation ZIP into it. Any additional packages for the OS will also be extracted somewhere in this directory.

It is very unlikely that OS/90 install files will not fit inside a floppy disk.

# FAQ

## Does it Run DOOM?

The #1 goal of OS/90 is to be able to run DOOM. Because OS/90 is DPMI-compatible, it should be able to run anything that uses a DOS extender.

## What type of kernel is used

The kernel is monolithic and modular, with some inspiration from the exokernel in that there is a focus on "securely" arbitrating resources.

## What does the name mean?

The name was partially inspired by IBM OS/2, although OS/90 has nothing at all to do with OS/2.

The 90 was chosen because OS/90 was meant to be for 90's era computers. This is no longer the case, so 90 means nothing specific. Targetting a decade was not a great idea because the category was too broad and I could not narrow down what was actually characteristic about 90's computing to make a single OS based on it.

Now the name means nothing in particular, but it kind of stuck.

## What version of DOS should be used?

Anything newer that 3.3 should work.

FreeDOS is good but requires modifications. COMMAND.COM needs to be replaced because the built-in one swaps programs to extended memory and allocates a massive chunk for that, thus starving OS/90 significantly on startup. There is no option to disable this behavior.

The FreeDOS command prompt on the other hand does save a lot of convnetional memory when swapping.

## Is OS/90 an operating system or an operating environment?

OS/90 is an operating system in the same way that Windows 95 is. It just happens to use services from another one (DOS).

## Where can I find documentation for driver development?

There is a full API handbook in the `docs` folder.

## What are the Licensing Terms?

Most of the code is GPLv2 licensed. See the notices inside the code.

