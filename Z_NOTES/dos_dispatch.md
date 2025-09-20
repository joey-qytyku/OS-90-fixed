# DOS VM Dispatching Logic (Updated July, 2025)

This document can be grep'ed for all occurances of "## INT" to get the information.

## Overview

OS/90 must securely arbitrate all access to real mode through the INT instruction, and implement address space translations when needed.

One layer exists over the entirety of INT 21H which implements dispatching to the DOS kernel in a way that does not disrupt its global state.

An array with flags for each AH code uses bit flags to determine which "group" a function is part of.

Groups:
- Registers only
- DS:(E)DX is the memory address, no definite size
- DS:(E)DX and (E)CX is the count

Additional flag(s):
- Can fail and updates the extended error code
- Irregular and must be handled manually
- System integrity violation

## Rationale and Examples

There are DOS functions that cannot safely be called in a multitasking environment, even if reentrances are blocked.

The extended error state is not only global in DOS, but it is not even exposed in the List of Lists or any other documented structure. The way it works is after a function that is capable of failing returns, OS/90 already has preemption blocked ahead of time even before a V86 call begins, and ensures no other program can modify the error code. Then it gets the error code and puts it in a process-local variable.

Another one is the INT 13H AH=01h. This is a very similar thing. INT 13H is extended on OS/90 to allow for block device access to disks recognized by the BIOS.

> I don't think disks should be directly accessible with either the DOS LBA functions or INT 13H within userspace.
> There is a function in IOCTL that asserts a full disk lock. Win95 requires it before INT 13H is allowed.
> BTW absolute read/write can be used the main OS/90 interface.
> Actually no. It only works for very small partitions.

> As it stands, PnP is able to configure resources, but what about installing new drivers? There should be a utility to install new hardware.
