This book documents the API provided by the OS/90 kernel and specifies a number of key interfaces, data structures, and behaviors that are expected from certain system components. For implementation details, consult some of the mardown documents, source code comments, or the journal in the project root directory.

# Why OS/90?

OS/90 was originally an OS designed for retro computers with i386 or better processors, but the 90 eventually lost its meaning as the focus shifted from the decade of computer hardware to the design of the OS itself and the principles underlying it. OS/90 is now a sort of paravirtualization software stack that is theoretically capable of running software for any 32-bit x86 operating system as well as various 16-bit systems. This it does with under 2MB of memory.

OS/90 is a rebellion. It spits on the "layered design," ACPI, and even the UNIX philosophy. It is a mutiny against decades-old tradition and technological modernity at the same time. On the other hand, it is not necessary to be part of my "revolution" of sorts because OS/90 has some legitimate niche use cases.

OS/90 is viable for x86 embedded applications and allows for very direct control over hardware. The modular design means it can be adjusted to fit many different types of computers, including embedded systems.

# Getting Started

OS/90 uses DJGPP so that it can self-host or be compiled from DOS using the sources. Every capitalized folder in the project root is a __package__, which can be independently compiled and installed. `BUILD.SH` and `INSTALL.SH` are used for all packages to install the packages on the computer.

The entire build system assumes a self-hosting DOS environment, which must be virtualized on Unix or windows.
