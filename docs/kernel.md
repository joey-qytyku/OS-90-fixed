# Kernel Overview

This document describes the purpose of the kernel, which is called KERNL386.EXE.

The kernel binary is position independent and runs at the virtual address of C0000000.

# Design Concept

The idea is that all requests to hardware from the userspace will be handled by a component on a stack of software following the layered model. Because DOS is a fully featured operating system, its interface is used, but it is extended to allow the 32-bit software to control it or pass it down the stack. Almost every request to the system is able devolve to a DOS driver/API call or BIOS function, ensuring high compatibility at the expense of slight overhead.

32-bit software can also function as an abstraction layer on the stack, e.g. USB host and USB device.
```
High Level
[  User Request  ]=\\
[  OS/90 Kernel  ]-||
[ Driver |   Bus ]-||
[   DOS Kernel   ]-||
[  PC BIOS traps ] \/
Low level
```
# What Type of Kernel?

# Fileystem and Disk Access

Upon startup, filesystem and disk access is immediately possible through the V86 mode interface. Drivers can trap real mode interrupts and IRQs to implement 32-bit disk access.

[Service1] -> [V86] -> [DOS]
[Service2] -> [DRV]

# Exiting to DOS

Exiting to DOS is impossible is OS/90. I could not figure it out, and do not see a huge advantage.

# Rationale For DOS Emulation

There are different ways that a DOS-compatible and DOS-initiated OS can be designed.

## Replace Kernel Entirely

Instead of starting the regular DOS kernel, a replacement kernel sets up the entire system on startup.

### Analysis

This design makes legacy compatibility difficult because of the isolation between DOS VMs. THe entire DOS kernel would have to be emulated, including the filesystem and the BIOS. The advantage would be more consistent design with less bugs. To avoid difficulties, the backup DOS kernel could be loaded in a virtual machine and get restricted access to hardware resources.

One question wuld be how devices can be accessed by DOS. Emulation could be possible, but in cases where the real device needs to use a DOS driver, the kernel would have to differentiate between supervisory virtual machines and regular machines, with an organized method of passing interrupts. It could also have to "lie" to DOS-based drivers about direct hardware access. This would be quite complicated. To what extent will a DOS driver be able to integrate itself in the system?

Overall, this is a clean concept, but way too difficult to design and not worth the time.

## Device Driver Bootstrap

A device driver can be installed that loads the 32-bit kernel and enters it. The kernel can access DOS services right away witout any emulation. AUTOEXEC runs automatically in a special process.

### Analysis

The issue with this design is that the TSRs and other programs executed in the AUTOEXEC process must be given special rights, somehow. Should programs forked by this initial DOS VM be split into separate processes? Probably.

## The OS/90

DOS is accessible to the kernel and all DOS VMs. Programs have a userspace API and interrupt call interface to access the 32-bit kernel services similar to unistd.h. Drivers can capture DOS interrupts and form an interrupt chain for sharing multiple sub-functions.

There are two types of drivers, bus and device. Bus drivers can request IO, DMA, IRQs, and MMIO from the kernel (a bus itself) and allow a device driver to control it.

### Analysis

I chose this design because it offered compatibility and ease of programing at the cost of overall design cleanliness. It also allows for easier user troubleshooting when drivers malfunction.

# Executables

## MZ

The relocation table is a list of seg:off pointers. The value pointed to must be added with the target code segment of the program.
