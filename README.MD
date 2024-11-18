Many people still programming for DOS are focused on making modern things work on it that would never work on a machine intended to actually run DOS. For example, XMS managers that can access multiple gigabytes of memory, real mode USB drivers, SATA and UDMA support, etc. OS/90 is not this way. It is meant to run on 32-bit computers from the entire 90's generation of computers or before that as long as it has a 386.

Features:
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

## How do i get started?

For userspace software, I suggest using either DJGPP or Open Watcom to compile programs. OS/90 is DOS-compatible.

Documentation is in the docs folder, but I only suggest reading the manual folder's contents.

Drivers are complicated, but in general, there are certain types of drivers with formulas to make them by:

- Driver to extend the operating system to support another OS if DPMI is not able (ELKS, Linux)

- Driver to enable access to a device by arbitrating all requests to it.

- Driver that implements an API for controlling a bus like PCI or USB, or a driver that provides an abstract interface for some class of device drivers.

If you want to make a PCI device driver or one that supports it, the option exists.

