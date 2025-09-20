WIP:
- GUI
- Advanced preemptive multitasking
- Reentrant and multithreaded kernel design
- Virtual memory
- 1GB of RAM available
- DPMI and DOS program compatibility
- 16-bit driver compatibility

Many people still programming for DOS are focused on making modern things work on it that would never work on a machine intended to actually run DOS. For example, XMS managers that can access multiple gigabytes of memory, real mode USB drivers, SATA and UDMA support, etc. OS/90 is not this way. It is meant to run on 32-bit computers from the entire 90's generation of computers or before that as long as it has a 386.

# Minimum Requirements

Miniumum:
- 386SX
- 2M of RAM
- No FPU needed
- SVGA video adapter, 640x480 and 8-bit color or better
- AT-compatible keyboard
- Pointing device

Suggested:
- i386DX
- 3M of RAM

High-end (and supported):
- PCI
- Disk with UDMA support
- CDROM drive

Version of DOS: 6.22. FreeDOS recommended.

OS/90 can run on modern hardware, but there are many challenges involved with this, such as buggy USB legacy support.

# FAQ

## What type of kernel does OS/90 use? What is the architecture like?

Monolithic, modular, reentrant, preemptively multitasking, virtualizing kernel.

OS/90 is based on DOS interfaces which are hooked to allow safe and extended access from user programs and system software, as well as using undocumented features to make DOS multitask safely.

Drivers also have a global event system which is used to perform tasks like incompatibility detections, display arbitration, and many other things, in a generic way. It acts like a neurotransmitter in the nerve center of kernel mode.

Processes are virtual DOS sessions and they can have multiple threads.

## Can you add support for [Insert hardware/software]

Compatibility with DOS software is a top priority, and I will respond to requests to get DOS programs and games working.

Hardware is different. A standard driver set exists for the most common devices.

If you are a content creator or professional who needs certain hardware working for some reason, I MAY be able to do it. I will NEVER ask for money or release closed-source code.

For video cards, I recommend using UNIVBE or whichever TSR was packaged with the card. Cards with specialized drivers will probably not work.
