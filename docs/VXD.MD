# VxD Translation Layer Whitepaper

> Work in progress and may reconsider.

There are many practical obstacles to making VxDs fully operational under OS/90. One primary problem is that some of them are designed to interract with Windows and there is a lot that could go wrong with that even if we had a windows install working.

The other problem is the massive difference between Win386 and OS/90 architecturally. There is a lot of "fooling" that needs to be done. On the other hand, OS/90 makes every effort to be capable of emulation in a very similar way.

# Settling Differences

## OS/90 Tasks and VMM Virtual Machines

VMs have a 4M mapping for the real mode section which allows for instant switching of the conventional memory with one DWORD write. OS/90 does the some thing, but it has a single shared address space. This makes it complicated to map the VM memory, which is something that a specific VMM call does.

The solution is
1. Only DOS/DPMI programs can be accessed by the translator. This is necessary because there is no other way to have a consistent definition of a "program address space" that is consistent across subsystems.
2. The single address space is of no concern whatsoever. If a VxD is trapping a DOS interrupt or something, it will know to map the real mode memory which has special tratment. If it is implementing an API for protected mode, it will calculate the address based on segment selectors. In short, we just give it a pointer to all of the mapped virtual address space.

VxDs use a very similar (in fact, they even use dual segment values! Great minds think alike!) register dump structure

## Critical Sections

##

## V86 Hooking

# API Implementation Section

EVERY API function that VxD implements according to the 3.0 documents is described here.
