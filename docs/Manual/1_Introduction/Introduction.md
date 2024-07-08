# Introduction

This book documents the API provided by the OS/90 kernel and specifies a number of key interfaces, data structures, and behaviors that are expected from certain system components. For implementation details, consult some of the mardown documents, source code comments, or the journal in the project root directory.

## Why OS/90?

OS/90 was originally an OS designed for retro computers with i386 or better processors, but the 90 eventually lost its meaning as the focus shifted from the decade of computer hardware to the design of the OS itself and the principles underlying it. OS/90 is now a sort of paravirtualization software stack that is theoretically capable of running software for any 32-bit x86 operating system as well as various 16-bit systems. This it does with under 2MB of memory.

OS/90 is a rebellion. It spits on the ?layered design,? ACPI, and even the UNIX philosophy. It is a mutiny against decades-old tradition and technological modernity at the same time. On the other hand, it is not necessary to be part of my ?revolution? of sorts because OS/90 has some legitimate niche use cases.

OS/90 is viable for x86 embedded applications and allows for very direct control over hardware. The modular design means it can be adjusted to fit many different types of computers, including embedded systems. OS/90 is also good for

## Getting Started

The OS/90 SDK is essentially the entire source repository. Typing ?./Build.sh? will compile all drivers, kernel, and any userspace-related software; and run it inside of Bochs. The version of Bochs must have support for the port E9 hack and all the debugging features. DOSBox is also required for building because it is used to copy the generated files into the FAT filesystem.

The build system is configured for my cross-compiler and bootable disk image. Using a different disk image is a total pain. Getting the CHS parameters in DOSBox and in Bochs to both be correct is a challenge; possible, there is a discrepancy between the CHS parameters used by the Bochs BIOS and the fake geometry of a non-CHS disk.

The cross compiler must be specially build for generating kernel code and nothing else. Using the built-in system GCC will likely generate incorrect code. GCC is required for the kernel and probably is for drivers as well.

Just send me an email and I can send any necessary software, as the MS-DOS image contains proprietary software and cannot be copied publicly.
Components

The OS/90 kernel API has the following distinct components and uses a DOOM-style prefix to indicate its membership:
- Scheduler (S_)
- Memory manager (M_)
- Driver loading and communication (D_)
- SV86 (V_)
- Early text IO (E_)
- Debugging (none)
- IO Manager (IO_)
- Miscelaneous (Z_)

Finding Help
This handbook is the most authoritative and up-to-date documentation on the OS/90 kernel API. If there are any questions, email joey.qytyku@gmail.com for information.

## Application Binary Interface

The OS/90 kernel uses a 4-byte aligned stack, stdcall stack conventions, regparm for the first three arguments, and EBX and EBP are callee saved registers. When using stdcall, cdecl stack conventions are automatically used when functions are variadic. Register arguments are passed the following order: EAX, EDX, ECX. The rest go to the stack in the standard cdecl order. It was determined through experimentation that this configuration reduces code size and improves performance.
Other aspects of the ABI are reserved to the compiler to decide. The kernel will never use in a manner exposed to related software any interfaces that include anything that could lead to incompatibility between compiler versions or additional parameters beside the required ones (e.g. bit fields will never be used for a structure).

A few environment-related specifications exist. As mentioned later, kernel mode stacks are 2,048 bytes long and overflows could possibly cause fatal errors. Consider passing "fconserve-stack" to reduce stack usage.
