This document is operational but in need of updates.

# OS/90 DOS Compliance and Multitasking Strategy

OS/90 intends to be compatible with DOS 6.22 and some 7.x features added by Windows. OS/90 is a 100% multitasking and multithreading OS except for serialized access to privileged real mode.

# DOS Virtual Machines

## Conventional Memory

> Updated July 26, 2025

OS/90 allocates all available physical real mode memory, after performing any additional reclaimation, and uses all pages in this range to bank swap DOS program into the real mode address space.

The DOS kernel, drivers, and other loaded software are reserved in the CM and are visible for compatibility. The user can allow DOS programs to use more RAM by unloading DOS software before starting OS/90.

### Incompatible CM layouts

Limulation is not supported at startup. OS/90 does not support GEMMIS or attempt to take control from an expanded memory emulator. The Booloader MUST run in real mode.

To compensate, OS/90 transparently uses upper memory blocks and can either report about 638K of free memory (-1K for IVT and -1K just to be safe) using the standard allocator, or it can report even more.

Depending on the VGA graphics options, it can go well beyond 640K.

The actual memory is allocated in such a non-standard way that MEM.EXE is actually dangerous to execute and may cause a crash. OS/90 detects any attempt to execute MEM and runs an OS/90-sepcifc version.

### Reclaimation

> The physical memory in the bankable region is also used when multitasking, as it is blotted-out before any program is ready to run.

> The bootloader and the initial COMMAND shell are terminated and all their conventional memory is reclaimed.

> FreeCOM poses a serious problem with its agressive XMS swapping. I may have to create a simple driver that lies about XMS when COMMAND is running.

## The Swappable Data Area

> Updated September 5, 2025

An undocumented data segment called the SDA exists in DOS since 3.1. OS/90 supports only the 4.0+ format.

It is a large dump of DOS's internal state which is not hookable (at least safely/correctly) or directly accessible through official means.

It exists to allow programs to reenter DOS as long as it is not in a critical section (more on that later) by saving/restoring the contents. OS/90 does not use it that way.

The relevant parts are:
- Current PSP segment
- Extended error code
- INDOS semaphore
- Program aborted by ^C flag
- Current DTA
- Current drive

These are things that have to be transfered into the SDA from the VM context. Every time a DOS interrupt (20h, 21h, 2Fh) occurs, these are copied if necessary and not if already done.

OS/90 does not swap the whole SDA and relies on the global state which is used for many operations that need to be global in scope. Other things must either NOT be replicated and may cause program failure if they were to. Time and IO things are an example.

## The DOS Data Segment

The SDA is a subset of the DOS data segment. Any data outside the SDA is accessed within a critical section by default.

The DOS data segment contains a number of important fields which still need to be correctly multitasked, such as current directory structures, pointers to drivers, and other things. Otherwise, drive selections and current directories would affect other processes and violate system integrity.

A definition of the documented fields is in DOS.H.

> The driver lfndos or any other variant will likely be unstable, especially when multitasking.

## Program Segment Prefix (needs updates)

The PSP has many components that must function as expected. It is needed for some program-local operations.

Extensions require the PSP calls to be supported because Windows uses PSPs to contain arguments and environments for multiple windows programs.

The PSP is only required to be part of the program segment if the program type is COM.

### Get/set PSP (TODO)

This is handled in 32-bits because it is just a getter/setter pair, but the SDA is correctly updated. Programs can change the current PSP which is locally stored in the VDM context.

### Sub-programs and the PSP segment value

The EXE/COM loader logic is completely 32-bit to avoid hanging and speed up loading.

### Environment Segment

The environment will usually be initialized by the command interpreter. It is nothing more than a region of memory, which in our case is isolated for each VM, that contains string data set up by `AUTOEXEC.BAT`.

AUTOEXEC.BAT or FDAUTO.BAT will likely contain the execution lines of TSR drivers and other things. By default, autoexec is not executed, but it IS read (or another file optionally) to set the variables correctly.

In cases where a program is executed without COMMAND.COM starting it up, the env block is set the same way.

> It is impossible to use a shared environment. This normally increases memory usage.

The ACTUAL environment from boot time is copied away into extended memory.

### Job File Table

> Some programs try to change the JFT though. Windows probably does.

Here is how DOS file handles work:
- The system file table keeps a linked list of every unique open file
- The address of a SFT entry cannot serve as a handle because handles can be duplicates and point to the same file while not having the same number.
- The JFT serves as a table of indices

Making the JFT actually local is not useful and makes it harder to perform IO from kernel mode drivers.

OS/90 considers the JFT to be global. It is resized to 255 handles. Creating new programs will not change the JFT references.

Each PSP has an address to the JFT and it does not have to be the default one, which keeps compatibility.

The JFT's first entries are:
- stdin (0)
- stdout (1)
- stderr (2)
- com1 (3)
- lpt (4)
- Rest are FF

> Do not confuse the JFT with the FILES=x directive in CONFIG.SYS. That actually refers to the SFT.

### Critical, ^C, and Exit Addresses

These are techically old features that should not be relied upon.

### File Control Blocks

OS/90 does not support file control blocks. This is fine since Windows 95 also does not fully support them. Filesystem drivers will block any of these functions.

### Disk Transfer Area

A DTA is provided by default and the internal DTA address is set to this location.

> The address of the DTA can reside in extended memory with extensions. Real mode and protected mode addresses are supported.

### Call 5

This interface is supported. Some programs depend on it. Call 5 simply calls INT 21H. It only works for COM programs.

### Command Line

Automatically generated by executing program.

### Internal Parent PSP Segment

DOS esentially executes programs like a stack. This is the PSP of the task that executed the current one.

This is emulated to contain the LPSP or local PSP.

### Segment of First Byte

This is the first byte of the program segment. DOS loads programs flat in memory. The PSP can be anywhere unless it is COM where it is part of 64K.

## System File Table

As stated previously, the SFT is still required to be used. Windows expects its presence.

## Handling of Subprograms

The interface for creating subprograms is controlled by the OS and the entire executable loader is reimplemented in the kernel.

DPMI 0.9 does not provide local segment descriptors, interrupt vectors, or any other structures. It explicitly states that the client must clean up everything before exiting. It acts more like a simple state transfer mechanism.

## Conventional Memory


## XMS Emulation

Each VM gets a certain number of XMS handles which are non-sharable between VMs. A simple lookup table with 24-bit compressed chain IDs is used.

## EMS Emulation

OS/90 cannot tolerate the exisitence of EMM386.

Expanded memory is a very ugly API that I have little interest in implementing. If I can adapt some existing code, it could happen.

## Context In General

OS/90 uses scheduler tasks to run DOS virtual machines. The distiction is that tasks can belong to a VM as threads of execution, while VMs are simply "instances" of the partially-simulated operating system.

DPMI 1.0 says these things about the context of clients in a single virtual machine:

```
DPMI 1.0 host handling of client termination differs from DPMI version 0.9 in the following respects:
    - In DPMI version 0.9, all clients of the same virtual machine share a single LDT and use a single IDT, and only those segment descriptors allocated by the client are deallocated when it terminates. A DPMI version 0.9 client should cleanup its own segment descriptors before its termination since some DPMI version 0.9 hosts may not free the terminating client's segment descriptors if the client is not the topmost client. In DPMI version 1.0, each client has its own LDT which is freed in its entirety at termination.
    - In DPMI version 0.9, all clients within a VM share a single IDT, and interrupt descriptors and exception handlers are not automatically freed (i.e. the IDT is unchanged when the client terminates). In DPMI version 1.0, exception handlers are deregistered automatically at termination, and each client has its own IDT which is freed in its entirety.
    - In DPMI version 0.9, real mode memory that has been unlocked by the client must be explicitly relocked by the client prior to termination. DPMI 1.0 hosts will relock real mode memory automatically.
```

OS/90 follows the DPMI 0.9 guidelines. The vIDT is not unique to each client and global to the VM. Same for the LDT. DPMI 1.0 is still compatible, but the 0.9 approach is used to use less memory.

This makes sense because the IVT is shared by clients and is used to pass calls to protected mode.

# Standard IO (TODO)

Whenever a program prints something, it needs to go somewhere. For DOS programs, there is the text mode framebuffer at B800:0000. The normal functions for this are captured.

## CON Device and INT 10H

The CON driver is normally not called, but may be used when performing redirections in the command line or something like that which involves opening a file named CON. While this level is 100% hookable, DOS is allowed to access CON through the literal driver usually in IO.SYS or the DOS kernel.



> Ability to print kernel messages through the console.

# The IDT and Entry Mechanism

The first 32 vectors are reserved on x86. This includes 0x00-0x1F, which means INT 21H and INT 31H are able to use fast entry, unlike the BIOS.

BIOS calls are dispatched by an exception handler.

# Multithreading Extensions

DOS processes are allowed to create threads while in 32-bit protected mode.

# Additional Functions

## 2Fh

Windows and DPMI implementations have some functions in INT 2Fh which are implemented on OS/90.

The extent to which these may be supported is for allowing Enhanced Mode Windows 3.1 KRNL386 to run, along with the rest of the environment. Some things do not work though. There are also various entry points which Windows software can use that have obscure documentation.

```
Int 2F/AX=1680h - MS Windows, DPMI, various - RELEASE CURRENT VIRTUAL MACHINE TIME-SLICE
Int 2F/AX=1681h - MS Windows 3+ - BEGIN CRITICAL SECTION
Int 2F/AX=1682h - MS Windows 3+ - END CRITICAL SECTION
Int 2F/AX=1683h - MS Windows 3+ - GET CURRENT VIRTUAL MACHINE ID
```

## BEGIN/END CRITICAL SECTION

This disables task switching by incrementing the preemption counter. The end critical exist too. It may be used for synchronization by Windows, so it is possibly needed.

> OS/90 does not currently have the VMM concept of a critical section. It is not per-process.

Not to be confused with the DOS calls that have the same name.

## GET CURRENT VIRTUAL MACHINE ID

A PID in OS/90 can be several different things. This is very likely used by KRNL386, and is actually supported. The ID has to be 16-bit, and is a DOS session info block pointer compressed to fit in

## Fast console output

> This is deprecated and illegal on DOS greater than 1.25.

## Absolute Disk Read (INT 25H)

This interface, along with INT 13H, is allowed so that PM software can format disks. It ends up calling INT 13H but translates to LBA first using the correct parameters.

This interface has ASE too.

## Idle loop (INT 28H)

This is used to indicate that DOS is safe to reenter while polling the keyboard. Currently not supported.

## INT 2E - Execute Command Using Base Level Command Interpreter

Basically the C `system()`. Uses command.com to execute a command.

It works the same way as the standard spawn call but only for command.com. No ASE is supported.

## Extended Error Information

# DPMI

OS/90 reports DPMI 0.9. The implementation is not planned to support 1.0 features to any extent further than being able to run Win16 or the OS/90 UI.

Some additional interfaces are also added to access unique OS/90 functionality not supported by any other OS.

This section will explain how OS/90 implements each DPMI function call and explains any extensions to or deviations from the standard.

## 32-bitness and 16-bitness

OS/90 DPMI is soft when it comes to enforcing bitness. Function calls that are normally not supported in 16-bit clients are supported under OS/90 with the expectation they are not used.

If extended registers are used to address program memory, bitness will be considered to ensure proper program behavior. The bitness of a program is constant throughout its execution and extended registers MUST be used for arguments if the client is 32-bit, even if the current code segment is 16-bit.

The general rule of thumb is that 16-bit and 32-bit clients will run "as expected" with no unusual behavior if the program does not rely on bitness-related errors, which will essentially never occur.

## Functions which are different in protected mode

This is in the standard.

## INT 2FH

Function 1680H Release Current Virtual Machine's Time Slice

### Function 1686H Get CPU Mode

Function 1687H Return Real-to-Protected Mode Switch Entry Point
Function 168AH Get Vendor-Specific API Entry Point

## LDT Services

### Allocate LDT Descriptors (0000)

Unfortunately, it is not possible to use reserved bits to determine allocation status because DPMI allows segments to be directly modified. A bit array is used instead.

### Free LDT Descriptor (0001)

### Segment to Descriptor (0002)

Allocates a descriptor and maps it to a real mode segment. Fairly simple. Multiple calls must return the same selector, but it is uncertain how many programs rely on such functionality; OS/90 does not comply in this regard.

Descriptors allocated by this are supposed to be impossible to free, but this is not a limitation on OS/90.

### Get Selector Increment Value

This is a value that when added to a range of allocated descriptors, gets it up to the next increment. This is 8.

The reason why this API call exists is because running DPMI in a different ring could cause the number to be different?

### Get/Set Descriptor (000C, 000D)

This maps almost directly to actual descriptors, but non-ring-3 descriptors are not allowed.

### Allocate Specific Descriptors

These work as expected. The first 16 LDT descriptors are reserved for specific descriptors. This means only one program can use them and others will actually fail, not stall.

## Extended Memory Services

### Get Free Memory Information (0500)

DPMI recognizes the following attributes of extended memory, reported by function `0500h`:
1. Largest available free block in bytes

The following are NOT reported, and are not required to either (only the first is):
2. Maximum unlocked page allocation in pages
3. Maximum locked page allocation in pages
4. Linear address space size in pages
5. Total number of unlocked pages
6. Total number of free pages
7. Total number of physical pages
8. Free linear address space in pages
9. Size of paging file/partition in pages

dosprogs should not allocate swap memory to avoid immediate thrashing. Only about 95% of the memory is allowed to be allocated at once by a DOS program by default, all of which is physical.

# Microsoft Extensions

OS/90 supports extensions for DOS as specified by leaked Microsoft documents, which should permit running 16-bit Windows under it. This permits protected mode to call 16-bit real mode functions and is used by the C library.

## Reflection Strategy

I want the ability to implement entire API calls using the direct context of the protected mode call. For example, a call to INT 21H AH=9 can be totally handled in PM using the segments as expected, while real mode can use a different tactic.

To do this, PM reflection and RM reflection will NOT handle PM extensions. Instead, it will be an SV86 global hook. The real mode reflection handler will pass down a PM context and the SV86 handler must capture this.

Suppose INT 21H AH=9.

- INT is received by exception handler
- Looks for local PM handler
- Finds none, passing to real mode
- No local real mode handler
- SV86 captures and finds out this is a PM context.
- If within the jurisdiction of extensions, it will be translated.
- If NOT, the call is carried out noramlly by the SV86 handler assigned

INT 21H will have an SV86 handler that is global. INT 13H will have one too since the API does support extensions.

If protected mode drivers want to hook something like INT 13H, it can work just fine, but it requires acknowledgement of extensions. The SV86 handler could be passed a protected mode context, and the only way to know is by checking the VM bit. If it was protected mode, it has to handle it differently.

The default SV86 handler for the INTs will copy the context to another one, and will do so directly using the registers of the calling task.

This makes writing drivers that use SV86 hooks kind of annoying. INTxH will automatically set the VM bit, so it is not that bad. Register-based APIs do not have to worry about handling PM.

Suppose I create an unrealistic AH=9 hook:
```
V86_HOOK INT21H_AH_9(PSTDREGS r)
{
    if (!(r->EFLAGS & FLAGS_VM)) {
        // In protected mode
        char __far *msg = MK_FP32(r->ds, r->edx);
        int i;
        for (i = 0; msg[i] != '$'; i++)
            putchar(msg[i]);
    }
}
```

This is obviously not how it should be done but illustrates the idea. Nobody would ever call INT 21H AH=9 directly in normal DPMI, and if they did, it would not work properly. On OS/90, it would. The PM call in both instances has the entire contexted passed to a real mode reflection callee. In our case, an extra layer exists that makes it work when it normally would not.

Note that this code is completely impractical. It is meant to highlight that the protected mode context can be received and detected for extensions.

## Things To Support

Most of the APIs are fairly straightforward to extend, but there are a few that need special considerations.

# DOS Memory Policy In Detail (TODO)

## Conventional Memory

This is something that should be user-configurable, and it is when starting the VM.

The recommendation is that only about 75% of the available conventional memory, but the create interface has full control.

## Environment Segment

Each program has its own environment. The size must be adjusted.

# Unsupported Interfaces

## Virtual DMA Services
> https://web.archive.org/web/20121022073637/https://support.microsoft.com/kb/93469

There are currently no plans to support VDS. Programs that use DPMI usually allocate their own memory for it, generally conventional. Windows has a VxD for simulating DMA. OS/90 has a driver for emulating DMA as well, with partially arbitrated access.

There is little need for this interface. As far as I know, few programs ever used it.

## DOS Protected Mode Services (DPMS)

This API is independent of DPMI or VCPI and can use both. It allows for resident drivers to run in protected mode. Not currently supported. Existing implementations may still be able to run on top of built-in DPMI.

