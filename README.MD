Many people still programming for DOS are focused on making modern things work on it that would never work on a machine intended to actually run DOS. For example, XMS managers that can access multiple gigabytes of memory, real mode USB drivers, SATA and UDMA support, etc. OS/90 is not this way. It is meant to run on 32-bit computers from the entire 90's generation of computers or before that as long as it has a 386.

WIP:
- GUI
- Advanced preemptive multitasking
- Reentrant and multithreaded kernel design
- Virtual memory
- Over 2GB of RAM available
- DPMI and DOS program compatibility
- 16-bit driver compatibility

# User FAQ

## What is the actual maximum RAM?

About 2 gigabytes. This includes virtual address space too.

## What version of DOS should be used?

OS/90 works on any version of DOS compatible with at least 4.0. If Windows 3.1 support is needed (not sure if it will work ATM) only MS-DOS works.

DR-DOS is also an option and may be more stable in some ways. So is PC-DOS.

# Installing

## From DOS

It is possible to install the sources into a DOS machine and compile using DJGPP, though the code generated will likely be of inferior quality to a newer modern compiler. I have not tried to compile a newer version of DJGPP for DOS yet.

Requirements:
- 32M-48M of RAM
- 512M of disk space
- CWSDPMI, NASM (both in path)
- Long file name driver or use FreeDOS
- DJGPP with GCC version 9.3.0 or later
- Make, bash, grep

Recommendations:
- Use a disk cache driver to speed it up. Set the cache size to several megabytes if you can.
- Get UPX and run `upx -d` on the `gcc`, `as`, `cc1` (found in libexec), `collect2`(also in libexec), and everything else you can find. This will massively speed up building but will increase disk usage due to decompression.

- Simply change directory to each module and run `make` and `make install-dos`.

Build and install the following modules:
- KRNL
- BOOT
- LIBC
- ATMLIB
- ATMDESK

Then enter the driver source tree within DJGPP/bash and build each necessary one.

## From OS/90

All of the above can be done from the OS/90 console window. This is useful for updating software. `make uninstall-dos` can be used to remove.

Run `make clean` when done with each to save disk space.

# For Content Creators and Critics

Send me an email if you want:
- A customized distribution with proprietary software I cannot distribute openly.
- Directions on doing things
- Please limit discussions to matters of OS/90 or any other software.

# Developer FAQ
