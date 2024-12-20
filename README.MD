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

# Developer FAQ

## How do I build OS/90?

Read the comments on build.sh and edit the vars file in the project root. Install the dependencies before running because there is no checking.

## How do I add packages to the build system?

A file named 1_pkg.sh must exist.

The first argument must be:
- build         Build all source files and the final executable
- install       Add to disk image
- uninstall     Remove from disk image totally
- clean         Delete all object files and any executables

If a project uses a makefile it can invoke that on its own.
