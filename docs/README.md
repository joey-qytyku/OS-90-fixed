# Current State

Work in progress.

# What is OS/90

OS/90 is a modern operating system designed to be binary compatible with DOS.

# Minimum requirements

|Spec|Minimum|Recommended|Premium|
-|-|-|-
RAM | 4MB      | 8MB       | 16MB
CPU | i386SX   | i486DX    | Pentium CPU
PC  | PC/AT    | PC/AT     | PS/2 compatible
OS  | DOS 3.0  | -         | -
Bus | ISA      | ISA w/PnP | PCI

OS/90 is a Plug-and-play operating system. Enable this option in the BIOS. If the system does not have a PnP BIOS, it will run the same way.

The i386SX is highly discouraged because OS/90 makes heavy use of 32-bit memory access and the SX has a 16-bit bus. It will make multitasking painfully slow.

OS/90 can be booted from a floppy drive.

# Warnings

Have at least 72 KB of conventional memory free at all times.

# Editions

There are editions for computers with different hardware. The -march is for the 386, so compatibility is garaunteed regardless of the CPU being used. Processor specific instructions may be used but only after being detected in real-time.

|Edition|Compiler Tuning|Included Drivers|
-|-|-
Type C| i386      | ~
Type B| i486      | ISAPNP
Type A| Pentium   | PCI, ISAPNP

# Build from Source

A Unix-like environment is required.

The makefile needs to be modified to refference the appropriate toolchain.

The following dependencies are required:

* NASM
* DOSBox
* git
* qemu
* make
* bochs

# Installation

The install files are provided in the release ZIP file. Unzip and copy to a CDROM or a series of floppy disks. Then XCOPY the files to a directory named OS90. Add OS90 to the path.

# Uninstall

Uninstalling is easy :)

OS/90 is self-contained and does not modify the DOS system at all. Delete the files in OS90.

* CD \
* DELTREE OS90

# Update

Reinstalling OS/90 is necessary to update. User files are not stored in the OS90 directory.

# After install

See boot.md for some information on the boot process and setting up the bootloader.

# Q&A

Q: Is OS/90 a real operating system or an interface for DOS?
A: I think it is an operating system. It has a multitasking kernel and driver interface, as well as 16-bit and 32-bit userspace. It is similar to Windows 3.1 enhanced mode and 95, both of which I consider to be operating systems. Despite this, DOS is a critical component and is used for more than just booting up.

Q: Is there plug and play support?
A: Yes. The OS is designed around plug-and-play functionality. There will never be plug and play COM (serial) device PnP. Just use Windows 95 for that. 16-bit plug-and-play drivers also work.

Q: Do DOS drivers work?
A: They should, but are certainly less stable. DOS drivers will probably be the only option for the majority of expansion cards. You will have to manually configure resources like IRQ/DMA/etc.

Q: What type of kernel design is used?
A: Monolithic modular kernel. Supervisor code always runs at ring 0.

Q: I want to write software for OS/90, what resources are available?
A: Everything in DOCS\ is relevant for kernel-mode development. Source code is the most reliable documentation. If there are any questions, feel free to ask.

Q: Which version of DOS is best?
A: In theory, FreeDOS is the best because it has LFN support, but it has many extra packages that could cause compatibility issues. Do not use EMM386 or JEMMEX. Disabling disk caching software may also be a good idea.

Q: Can you make a custom version of OS/90?
A: Yes! Email me and I can make a special version.

Q: Why did you make this?
A: I thought it would be easier to make it for older computers, since portability is not a concern anymore and low-level optimization (which I like to do) is easier. I also found myself facinated with old machines and thought it would be fun to do. OS/90 turned out to be more difficult than I expected because of my choice of target platform. I cannot stop, not now that I have gone so far.

Q: What is your favorite part of the code?
A: The scheduler

Q: Will there be a GUI?

Yes. It will at least be able to run concurrent DOS windows. I may also design a GUI library.

Q: Did you use ChatGPT?

A: I __never__ use ChatGPT to generate code. I do ask it for information when Googling is not enough.

# Credits and Inspirations

## Linux

I learned a lot about preemptive multitasking and synchronization from studying the design of the Linux kernel.
