# Current State

NOT COMPLETE. Most of the readme and documentation is dedicated to the release, and specifies the full programming interface that WILL exist.

# What is OS/90

OS/90 is a work-in-progress operating system which includes a bootloader, kernel, and some userspace utilities.

It is designed specifically for 32-bit IBM-PC compatibles, primarily from the 1990s, but can run on newer hardware. The kernel is non-portable, but APIs built on top it, including the driver model, are somewhat portable.

It will be released under a different name, but the source code will always be named "OS/90". Kind of like Windows NT.

# Minimum requirements

|Spec|Minimum|Recommended|Premium|
-|-|-|-
RAM | 4MB      | 8MB       | 16MB
CPU | i386SX   | i486DX    | Pentium CPU
PC  | PC/AT    | PC/AT     | PS/2 compatible
OS  | DOS 3.0  | -         | -
Bus | ISA      | ISA w/PnP | PCI

* OS/90 is a Plug-and-play operating system. Enable this option in the BIOS. If the system does not have a PnP BIOS, it will run the same way.
* DOS should have at a decent amount of memory free to prevent out-of-memory issues, 2KB must be free
* The i386SX is highly discouraged because OS/90 makes heavy use of 32-bit memory access.

OS/90 can be booted from a floppy drive.

# Warnings

* 32-bit Floppy disk support is experimental

This technology is infamously unreliable and I do not have computer with a floppy drive to test certain features.

* Do not use a compressed drive or 32-bit disk access will fail!

The kernel will call the 16-bit filesystem but traps disk access to perform it in protected mode for performance.

* Do not use disk cache software without disabling it
I have not tested this, but SMARTDRV will cache disk contents to memory, sometimes XMS blocks. It will not be able to flush disk contents when the OS boots. Because it optimizes the INT 13H function by hooking it, 16-bit disk access is required for something like this.

* Summary
Run DOS light for compatibility.

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

# Limitations

In the current design, the 32-bit userspace runtime has not been specified. It will be in the future. Filesystem access is 16-bit only.

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
A: I thought it would be easier to make it for older computers, since portability is not a concern anymore and low-level optimization (which I like to do) is easier. I also found myself facinated with old machines and thought it would be fun to do. OS/90 turned out to be more difficult than I expected because of my choice of target platform, but that is okay.

Q: What is your favorite part of the code?
A: The scheduler, specifically, virtual 8086

Q: Will there be a GUI?

Yes. It will at least be able to run concurrent DOS windows. I may also design a GUI library.

Q: Did you use ChatGPT?

A: I __never__ use ChatGPT to generate code. I do ask it for information when Googling is not enough.

Q: WHY U UZE GLOBAL VARIABLES REEEE

I use global variables for performance-critical parts of code. I think global variables are acceptable as long as they are well documented and relatively isolated from other subsystems. Otherwise, I avoid them.

# Credits

*will update in the future*
