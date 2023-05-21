# Overview

The userspace specification details the way that programs and the user interface with the system. It is highly recommended that programs follow the userspace protocols. The goal is to ensure that their are no competing standards, and that OS/90 can avoid being fragmented and cluttered, and instead be an integrated experience. OS/90 is intended to be a desktop operating system for only one user.

# Structure of Userspace

## USER.EXE

USER.EXE handles the loading of executables and libraries, as well as heap allocation. USER.EXE is a flat binary that accumulates symbols to export to other software in its own table with names. It is loaded into a reserved address space.

Despite USER.EXE being a DPMI program, it cannot be executed inside a DPMI host because it depends on the OS/90 kernel's system call interface and being loaded at a fixed address.

It may be possible to run multiple instances of USER.EXE within OS/90. Could that make OS/90 a multiuser OS?

### Yield and Periodic Yield

Should I keep periodic yields?

The YieldCPU call will switch to the next process in the list. When the calling process is returned, it will execute as if nothing happened. This is used to implement cooperative multitasking. USER.EXE can yield itself because it nothing more than a special client. If a client made any API requests, it will block the process and execute any unblocked process.

## Clients

The programs that a user interracts with directly are cooperatively tasked and are called CLIENTS. All clients share the same linear address space and can access each other's memory without obstruction.

The PE/COFF format is used for userspace software. The PE loader is entirely userspace-implemented and the kernel knows nothing of it. In fact, the kernel only understands DOS EXE and COM files.

### Implementation of Task Switching

A list of process control blocks is created for each program under the control of USER.EXE. The PCB will also contain the FPU

# User API

This API is low-level and controls memory allocation, tasking, files, and other features. There is no need to access DOS directly as it is done internally by USER. No function provided by the userspace API matches with a C standard library name.

## Data Types

The same data types are used by the API as the kernel, but some extra definitions irrelevant to the user are omitted.

## Defines

```
INVALID_CLID
```

## Program Control

```
ChangeClientControl(CLID cl)
```
This procedure will switch to the client specified. Fails silently and program continues like nothing happened when returned to.

```
VOID YieldCPU(VOID);
```
Relinquish control of the CPU to any client the kernel prefers.

```
CLID CreateClient(IMUSTR exec_path, DWORD flags);
```
This will execute a program.

```
VOID EnableYieldPeriodic(VOID);
```

Possible flags include:
* CREATE_RUNNOW

## File Control

# Language and Toolchain Support

GCC is used ot compile all the C code in OS/90, and that includes USER.EXE. DJGPP could be used since it is basically just a port of GCC to DOS.

C++ is currently not supported because try/catch, new/delete, and other features require runtime support. It is not impossible,

## C/C++ Standard Libraries

I doubt I will have enough time to port an entire C or C++ library. It would be very time consuming and with very little gain. It could happen in the future, though.

# Directory Structure

```
OS90/
    KERNL386.EXE
    OS90.COM
    USER.EXE
