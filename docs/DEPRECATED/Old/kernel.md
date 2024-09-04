> TODO: We should make it so that the capture chain goes in reverse so that we can alter the inputs of the previous handler instead of going up the chain and potentially never reaching it. Instead, we will provide a function for getting the current handler and saving it. No need for a linked list.

# Introduction

OS/90 features a preemptible, reentrant, multitasking microkernel.

The kernel is designed to be minimal and only deals with communication between servers and memory management.

Servers are cooperatively scheduled userspace programs that can communicate with user programs and handle events.


## Comparison With Win386

VMM32 can schedule tasks while in kernel mode, but is still considered non-reentrant. I believe it uses an event system to schedule event handling for an opportune time. OS/90 is 100% reentrant and uses mutex locks.


# Index

[Code Rules](#code-rules)

[Scheduler and V86](#scheduler)

[Memory Management](#memory-management)

[Drivers](#drivers)

[Kernel API Overview](#kernel-api-overview)

[Kernel API Index](#kernel-api-index)


# Code Rules

## File Names

Source files and folder should be capitalized and follow DOS naming rules. Documents must also follow this rule. Files related to the build process

## Include Files

Includes files have the extension `.INC`. Includes are restricted to defining structures and equates.

Equates should be in MACRO_CASE and may NOT use dots as seperators.

## Style Rules

Procedures should be This_Cased. Local labels should be snake_cased. Variables use a Hungarian-like naming protocol.

```
A = array of ...
P = pointer to ...

== Only make sense for pointers ==

V = void (for pointers)
F = function (for pointers)

== Others ==

J = jump location

STR  = Exactly the same meaning and intended semantics as a `const char*` in C

== Data Types ==

U/I{8,16,32,64}

Structure instances simply use their name.

== Examples ==

AU8_myArray
APF_ArrayOfPointersToFunctions
U64_GDTEntry
```

All code should be written for viewing on 80x25 screens. Spaces should be used and indent size is 8.

## Comment Rules

Code should be sectioned using double lines (=) for the top level and single lines (-) for procedure- or variable-related comments. `.section` declarations should be spaced to the center.
```
;=============================
;       S e c t i o n
;=============================

;=============================
          .section
;=============================
```

Procedures can be commented like this:
```
;----------------------------------
; BRIEF:
;       ...
; Argument(can be register) := ...
;
; WARNINGS:
;
```
Text should be aligned with spaces.

# Scheduler

Scheduler means more than just "the part of the kernel that decides what runs and when." It should be thought of instead as a __context manager__.

## Context Types

## Processes

### Process Control Block Structure (INTERNAL)

A PCB is 4096 bytes large and is aligned at a 4K boundary. It is an internal structure that drivers do not deal with.

16 page directory entries are stored, which are 64 bytes large. These are copied into global page directory when scheduling a process. This gives 64MB of addressing space. In future versions, the address space for userspace.

A process can have no more than the (maximum address space / 4MB) pages.

The register dump structure applies to both kernel and user mode. If the thread switched from user to kernel, the userspace registers are saved onto the stack, which is part of the PCB.

```
    STRUC   PCB

RES RD_registerDump, U8, RD.size
RES AU32_localPDEs, U32, 16
RES PF_eventHandler, U32
RES PF_posthook, U32
RES PF_prehook, U32

    ; Rest is stack space.
    ENDSTRUC

```

Event handlers are local to each process, unlike V86 chains. This allows for greater concurrency.

Prehook and posthook are procedures that run when the process

## Virtual 8086 Mode

### Supervisor V86

It is often necessary for kernel-mode software to access BIOS services or to use any feature that is not implemented in 32-bits.

The issue is that the 1MB of real mode addressing space could be mapped in any way whatsoever by running processes. A special memory manager function is used to identity map the real mode memory.

### Trap Hooks

The kernel provides the basic functionality of capturing INT calls from programs, a feature needed by most drivers. The emulation behavior of instructions is the business of a dedicated driver.

UV86 INT capturing is not automatically done. A special API call is used to dispatch to a V86 handler from the event handler.

SV86 hooks are required, however, and a global chain is used for this.

The hook procedure recieves an RD structure in register EBX. If the carry flag is zero

> Setting a V86 handler requires doing so for __both__ SV86 and UV86 to ensure there is a proper handler for both.

## Protected Mode

### Trap Hooks

Protected mode traps can be hooked. A global array of handlers is kept and they follow the same protocol as V86, but there is nothing related to SV86.

## Scheduling Policy

OS/90 uses a round robin scheduler at the moment.

# Memory Management

The features that will be implemented include:
- Per-process virtual address spaces
- Virtual real mode memory
- Uncommitted memory
- Page locking/staining
- Shared memory and emulation buffers
- Heap allocation for kernel
- Paging

Local real mode memory is built in and is not provided by a driver (but can it be?). The first process is configured not to use it.

> Make it possible to implement DPMI in a driver.

## MEM16.DRV

> We need a way of keeping track of pages that are being monitored by a driver to prevent conflicts. Or they can just share. `PG_MONITOR`.

MEM16 is a driver that implements local DOS memory for all processes. Memory is allocated with page granularity, so there are 160 pages. This requires 20 bytes to store a bitmap of available blocks. Actual memory control blocks are not used and only the PSP segment is stored before the data region.



> We need the notion of scheduling events for VMs and have a kernel-mode procedure to handle them. Calling V86 on behalf of another process requires something like this.
> What if we have a local handler chain for each process?

> How will you handle events in a T2?

## Structures and Algorithms

### Page Frame Table (PFT)

# Drivers

Drivers have the file extension .DRV and use a special executable format.

## Executable Format

Drivers are flat binaries but with a special relocation table added in. They have no external symbol linking abilities.

The output of `readelf -r` is used to generate a relocation table that is terminated by a magic number of 0xDEADBEEF at the end of the file.

The binary has no sections and all allocated sections are required to be joined into .text. The only relocation table that matters is `.rel.text`.

R_386_PC32 requires an addend of the load point minus the relative program counter. Procedure calls (which use relative 32-bit addresses)
R_386_32 simply adds a number to the base.

> Think about this

Both are needed for successful relocation.

The entry point is just before the magic number and is near called.

## Environment

The initialization function is executed using a kernel startup stack or a process-local kernel stack. The size of the stack can be assumed to be under 4096 bytes. Avoid recursion and large stack allocations.

# Kernel API Overview

The OS/90 kernel API uses an assembly-only ABI that requires thunks to work with C. Any procedure that is exposed through the kernel API must respect the ABI.

INT 0CFh is used to access this API.

> Memory allocator with relocation event handler?

## Calling Conventions

The following registers are used in this order: EAX, EBX, ECX, EDX, ESI. Any registers not specified as outputs to a function call will not be clobbered.

The macro OSC is used to make kernel API calls from drivers in assembly code.

AX is the function code and EAX>>16 is the driver code, where zero is the kernel. This is for implementing dynamic linking. The assembly macro OSC automatically makes a list of calls to INT 41h with addresses (that are relocated on load). It is possible to import symbols from other drivers and pass the base address of the API relocation table.

This still requires ordinals to call driver or API functions, but the driver name can be linked.

```
OSC Logf, "Hello, world! Also a number: %i\n", 10
```

The carry flag is the universal error code indicator.

If a function is not supported, EAX will be set to all zeroes.

## Function Codes

# Kernel API Index
