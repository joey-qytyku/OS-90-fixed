> Why not have a single segment for the whole VM? Why not share the address space?

> Also, why not use DPMI 1.0 features? There are servers that implement DPMI 1.0 and OS/90 can have some features.

HDPMI also supports DOS translation, which OS/90 also will, as well as most 1.0 features. XLAT may be useful for implementing some services.

# DSL/90

The DOS Subsystem for Linux is an ambitious project to make DOS capable of executing 32-bit Linux programs inside a lightweight virtual machine.

This will be a package for OS/90 but does not require DOS.

## Requirements

This will not make a potato run Linux automatically. At least 4M of RAM is recommended for doing the most basic work.

A DPMI server with support for most 1.0 features is needed. HDPMI is recommended. DPMIONE may also work. OS/90 reports 0.9 but supports several 1.0 features and is detected appropriately.

> OS/90 is recommended over a DPMI client as it can multitasking DOS and Linux at the same time. If using OS/90, do not run the executable from the command line, as it is a full VM instance. A special interface exists.

## Scheduler

Making IRQ#0 hookable has its challenges. Can I send a tick to the VM task every time IRQ#0 fires? This makes some sense. I do not want to depend on OS/90 being a complete OS right now.

IRQ#0 could of course actually change the context of what it just entered to run the tick interrupt. Cooperation is required between several components. IRET must be emulated correctly in all modes.

Emulating IRQ#0 is a good idea. Windows standard mode will 100% try to use it while the rest go directly to real mode.

Only tasks that have a handler for it should have this done.

> Alternatively, syscalls can indicate a voluntary task switch.

## Memory Management

Null pointers must be errors or the environment will not work properly.

mmap must work totally. Luckily, it is mostly not that bad. It allocates page-granular memory. It also can map files, which is something that is difficult to do under pure DPMI.

Memory mapped files kind of require handling page faults. It is also necessary to reserve memory without allocating, which is not really possible with DPMI 0.9. 1.0 supports this and the OS/90 kernel also does.

Programs must run in a shared address range, so segmentation is needed. Shared libraries must also be possible. This can be done by extending the program segment.

The dumb solution is of course to allocate memory for the whole file and write back upon closing. The OS can then be relied upon to correctly allocate the memory without wasting too much.

If MMAP does anything like reserving addresses it can just allocate anyway.

### Memory Mapped Files In Detail

If 1.0 is supported, memory mapped files are a bit easier to implement because ranges can be reserved and then filled in as I like. This is also efficient, although not perfect.

I will probably just copy the whole thing in memory at first and work on it later.

munmap takes the size to unmap. Some software may rely on this behavior.

## Filesystem

Case sensitivity does not exist under DOS or Windows at all. The trick is to place a ^% before a captal letter. If it appears twice "^%" it counts as an escape. LFN is required.

Attrbutes of files in UNIX:
- Date and time at very high precision
- Group/user ID
- Read, write, execute
    - These are also per-user and per-group.

Users will likely have fixed limits.

- The filesystem has to be stored as a subset
- The DOS FS must be exposed through a mount point

## Users and Groups

In UNIX, there are user IDs. Users can belong to groups which can have different permissions.

Default groups exist.

Executables can have a root UID and still be executable to other users. For example, sudo.

UIDs can be of three types:
- Real
    - The UID of the process that started the process
    -
- Effective
- Saved

See more: https://www.geeksforgeeks.org/real-effective-and-saved-userid-in-linux/

The same applies to groups:
https://www.man7.org/linux/man-pages/man2/getegid.2.html

## Mount points

UNIX allows mounting valid block devices to any directory. They can even overlap in the same directory. The protocol is that the most recently mounted device gets it. This cannot be overrided without reversing/redoing the mount operations.

## DevFS

A number of devices must be supported and others are optional. Some are rather special and require the VM to make them localized (e.g. stdout/in/err).

- stdout, stdin, stdout
    - These overlap with DOS
- teletypes
    - I believe the INIT system typically initializes stdout to point to a TTY by default by force duplication.
    - Terminal emulators probably do something similar for subprocesses it executes.
    - sudo makes sure that programs running under it cannot be snooped on by others when in the TTY by reading it.
- /dev/null
- /dev/zero
- /dev/random
- /dev/urandom

Device files can be created as blockdevs or chardevs. This may not be super critical but is worth mentioning.

## Syscalls

There are a total of 335 system calls. Some are much more complicated than others. Some are quite new and dont need to be supported.

## brk

The memory layout must be formalized.

## Dynamic Libraries

## Versions

I will support all syscalls that I can. The version reported can be a choice.

## Binary Compatibility

100% compatibility is intended. All software designed for all Linux distributions should work, so long as it does not depend on kernel drivers.

## Software Package

It is not currently known what software will be able to run. A full distribution may not be practical as anything related to the kernel will probably not work.

Alpine Linux may be a good candidate since it is used for Docker. Maybe someone can install Gentoo on it?

# Technical Details

## Development Environment

DJGPP will be used. A number of operations are easier this way. I can operate on files for example using the supported UNIX calls.

## Memory Layout

A range of virtual addresses is allocated using DPMI 1.0 calls. Allocations within are handled using an abstract interface that is later used by mman.h functions.

The address space is shared within a single segment (DS=SS=ES). The first page is reserved and unmapped. Currently, a single NULL reference will crash the system. This is being worked on.

To prevent buffer overruns, breaks are inserted between allocated regions.

brk allows programs to set the size of the heap using an absolute pointer. I am not sure how a program would know where to base it from.

I think it has something to do with the stack.

## Access Control List

The ACL is stored in the root directory. It uses a binary format.

## Configuration File
