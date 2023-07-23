# Virtual 8086 Mode Subsystem

This document is severely outdated and is currently being updated.

## ScEnterV86

This function will enter V86 directly from a C caller. The specified context is loaded and entered. The only way out is an IRQ or a exception.

EnterV86 will save registers that cdecl expects to be callee saved and destroys the other ones. TSS.ESP0 is set to the current stack pointer. When an interrupt/exception takes place, EnterV86 will continue where it left off.

# 16-bit tasks versus Kernel Interrupt Calls

The kernel can call interrupt requests and INTx vectors using special functions. The kernel does not do this for 16-bit tasks. A task running in V86 mode does not require any function to enter. Loading the context is sufficient, which includes the EFLAGS register with the VM bit on. This distinction is important.

In both cases, the monitor is used for handling GPF exceptions to emulate ring-0 instructions.

# TSS and V86 Context Switching

## ESP0 and SS0

The task state segment contains two important fields, ESP0 and SS0. SS0 will always be set to the flat model segment of the kernel for the switch back to 32-bit mode. The value of ESP is simply what EnterV86 started with before running in protected mode. ShootdownV86 is the other function that V86 uses. When it is called, execution transfers back to the caller of EnterV86. This is normally only called in exception handlers.

## IRET Stack Frame

If the EFLAGS on the stack has the VM bit enabled, IRET will pop the data segment registers from the stack. This allows us to set the segment registers before entry.

When the V86 program is interrupted, the stack is set to ES0:ESP0 in the TSS and all the segment registers are pushed along with the usual IRET stack frame. The 32-bit segment selectors have to be restored. According to the intel documentation, they are "zeroed" (aka reference null segment). The stack segment is the same as the data segment, and CS:EIP is already set from the interrupt descriptor. This means that restoring the proper register context should be as simple as SS=>DS,ES,FS,GS.

EnterV86 continues execution in V86. Because this function is re-entrant, the stack pointer in the TSS saved must point to the register dump. See this for more info: https://stackoverflow.com/questions/54845547/problem-switching-to-v8086-mode-from-32-bit-protected-mode-by-setting-eflags-vm

The kernel allocates a stack for each program running on the system, both 16-bit and 32-bit. EnterV86 does not re-enter the caller and simply resumes execution in V86 mode. Only an interrupt or exception can stop the execution of any ring-3 code, as well as V86. When GPF is called from V86, the handler is called and ESP0 is loaded from the TSS. Interrupts and exceptions must work in V86 mode or getting out is impossible, but if ESP0 stays the same and the stack is reset upon each supervisor call, the stack of the caller is destroyed and the system will crash. To prevent this, ScEnterV86 saves ESP to the TSS.

## Other Information

Task switching never happens when the kernel or drivers are running. This is especially critical for BIOS/DOS calls from protected mode because a task switch would change the kernel stack being used.

# Critical Error, Control C, Timer Tick

DPMI programs can set their own handlers for these three using the set vector DPMI call.
```
Most software  interrupts executed  in real mode will not be
reflected to  the protected  mode interrupt hooks.  However,
some software  interrupts are  also reflected  to  protected
mode programs when they are called in real mode.  These are:

    INT            DESCRIPTION

    1Ch    BIOS timer tick interrupt
    23h    DOS Ctrl+C interrupt
    24h    DOS critical error interrupt
```

The second sentence says that if a client is in real mode, these interrupts will go to protected mode. Lets say a DPMI program catches a control C while in protected mode. It should go to a PM handler. This makes sense, but only if an actual Control+C handler was actually set using the set protected mode vector function.

Before we understand this requirement and how to implement, the desired behavior should be established. If DOS is executing, it may call the control C handler. In my tests using CWSDPMI with a test program, pressing control C cancels a get character call and the program continues normally. This means that the operating system itself is calling INT 23H, but what happened after?

The conclusion is that the default behavior is to go to the real mode control C handler. This will be local to each process. If there is no ^C handler set (because the process did not start with COMMAND.COM) the program should terminate as there is nothing else that can be done.

Having the program switch to real mode and start running the COMMAND.COM handler works. There will be separate instances of COMMAND for every task created by a window manager, so this is not a major problem. If a program is executed independently, it may crash if the handler is not set.

# DOS Semaphore

DOS has an undocumented function for getting the address of a byte that determines if DOS is safe for re-entry. This is for interrupt service routines that need to make a DOS call. If DOS is still servicing an interrupt, it should not re-enter. The first two words of the kernel reserved memory are set to this address. This must be pulled out for use.

When real mode DOS is called from the kernel, this byte will be set by DOS. If a 32-bit V86 handler takes control, this byte must be set to a non-zero value in case a RECL_16 IRQ is issued and tries to access DOS too, and this must be done while interrupts are disabled.

Really?

# DOS Idle Loop

INT 28H is the idle loop. In real mode, this is called while polling for keyboard characters. TSR programs use it to popup. When it is running, DOS calls can be safely made. The scheduler calls INT 28H for every time that a program is issued a slice.

# Interrupt Reflection and Capturing

The INT instruction is emulated by the V86 monitor. Capture chains are used to search for a proper handler for the specific function call. When INT is called, execution is passed to the CS:IP in the physical 1M.

IRET is a termination code for an ISR and a regular 16-bit V86 program. This is because normal software has no reason to use IRET, and it is impossible to tell when an ISR is trying to exit except by detecting the IRET.

The stack is not modified by virtual IRET and INT because it does not need to be and these instructions have special significance to only the monitor. This means that the monitor is incompatible with an ISR that uses the saved values for whatever reason.

## The Local Interrupt Problem

## Fake Interrupt Request and DPMI Interrupts

# A20 Gate

The A20 gate is assumed to be on. It is enabled upon boot. Any software that relies on the 8086 address wrap feature will not work on OS/90. I will never add support for enabling or disabling the A20 gate at a software level. Don't ask me to. On old windows, there is WINA20.386 which allows the A20 gate to be enabled and disabled at a per-process basis. This could be possible to implement, but as stated previously, I don't care about the A20 gate :P

# Execution of DOS Programs

All real mode software runs in physical DOS. Addresses in 16-bit mode are identity mapped. This approach reduces the available memory when multitasking. This also requires that a real mode stack are allocated by the DPMI client before initialization.

Environments are easily accessible this way.

## Executing a Process

Loading programs must be done by the 32-bit kernel rather than DOS because DOS will immediatetely execute the program until it returns. The function for executing a new process is trapped. The loading process used by OS/90 need not be exactly the same as the underlying DOS. The program segment prefix is always the allocation base of the program. Resizing the PSP of a process to zero will destroy all of its memory.

COM programs are obviously not given the entire contiguous memory. They will get no more than the file size. This can be configured by userspace for programs that need more memory.

Function 4Bh can execute a program immediately or load an overlay. When a program exits, the exit status of the subprogram is retrieved. This can be implemented inside the process control block. The exit function (4Ch) is trapped and will kill the subprocess.

The PCB stores a stack containing the return values of subprocesses and their program segment prefix segments. These are not valid OS/90 PIDs and cannot be manipulated as such.

What about the Job File Table? I guess we will use that of the last real process to run (bootloader).

DPMI programs cannot use any of these functions safely while in protected mode.

## The Program Segment Prefix

The program segment prefix segment serves as both a sort of process ID and the location where command line arguments are stored. The functions (51h and 62h, first is undocumented) are captured so that they report the PSP of the requesting process. This is aware of subprocesses.

The PSP segment is stored in the process control block.

## XMS

XMS uses 16-bit handles and memory freezing. Memory handles 0-65535 are garaunteed to be XMS reserved.

Some drivers may use XMS. When the kernel initializes, it will run the OS inside the largest possible XMS block, leaving all others blocks intact.

What if DOS is called by the kernel through a supervisory call? If such a program freezes an XMS block, how should this be handled? The XMS API must be virtualized because XMS will switch to protected mode in order to copy between extended and conventional memory. Should the previously allocated handles be probed by the kernel?

## Expanded Memory

# Direct Hardware Access

Some programs need to access IO ports directly.

## Examples

A DOS game may want to access the keyboard directly and even set its own interrupt handler.

## Virtual Devices

A device driver should register a virtual device with the DOS server if it intends to make the device available to virtual DOS programs. For example, the 32-bit 8042 keyboard driver can allow DOS programs to hook a fake IRQ and access emulated IO ports.

Virtual devices have a simple structure.

```c
typedef struct {
    PIMUSTR     name;
    WORD        irq_bmp;
    DEV_EVHND   dev_event_handler;
    WORD        io_port_base;
    BYTE        io_port_length;
    PVOID       map_to1, map_to2;

    PVOID       next;
};
```
Virtual devices are shared by all processes. It is the job of the driver to ensure that the physical device, if it exists, is properly accessed.

Port IO is emulated completely in software.

There is a difference between RECL_16 interrupts and fake IRQs. RECL_16 is for non-PnP devices with real mode drivers. BUS_INUSE is used for all fake IRQs. The handler can then fake interrupts.

### Virtual Device Memory

Some memory mapped IO regions, like framebuffers, do not need active emulation and can be modified and read however the program likes. MMIO that involves active emulation of all reads and writes is not supported. Most hardware for PCs used IO ports for almost all communications and memory mapped IO was really only used for framebuffers or EMS memory, which did not require sequential emulation.

## Post hooking and Pre-Hooking

It may be necessary to monitor or modify the input or output of a DOS call rather than replace it completely. Internally, the handler can either be 32-bit or 16-bit. A 32-bit handler can manually enter V86 for both post hooks and pre-hooks.

# DPMI

This may be the most important section in the documentation. DPMI inflences many of the kernel design choices. DPMI is a standard that provides a very low-level interface to DOS programs that is difficult, but not impossible, to virtualize.

DPMI does not specify an executable format. Loading executable data is handled by a real-mode stub.

The implementation used by OS/90 is described in the specification as a fully virtualized environment. OS/90 is not DOS and emulates or arbitrates all devices accessed by a DOS program. DPMI programs are virtualized DOS applications.

DPMI version 0.9 is the most widely used version and is implemented in OS/90 with some extra features.

## 16-bit and 32-bit DPMI

A 16-bit program can create an executable 32-bit code segment, but still remains 16-bit and some function calls are slightly different.

## Interrupts

### Interrupt Reflection Defaults

DPMI mandates that software interrupts in the protected mode environment are reflected to their 16-bit DOS counterpart by default, with INT 31H and INT 21H AH=4Ch being exceptions. That means the DOS interface will basically operate exactly the same except in protected mode.

How would IRQs work then if we HAVE to remap the PIC in order to avoid conflict with the exceptions? The answer is simple.

* The IDT allows specifying the ring that the IDT entry can be called from using INT
* All real interrupt vectors are ring 0.

This means that a user program that calls an IRQ or system exception vector will generate a protection fault, allowing the INT instruction to be emulated, and the interrupt reflection using V86 can take place.

Each process has data for every interrupt entry. Each entry indicates if the vector is to be reflected or if it has a protected mode handler assigned to it. If it has a protected mode handler, it is called.

Changing vector 31H is not allowed and will cause a critical error.

### Interrupt Hooking


### DOS Memory Services

DPMI 0.9 requires implementing DOS memory services. Allocating memory will automatically generate 64K or less LDT entries and return the base selector. I have no idea why the DPMI spec does this when there is already a way to convert segments to selectors. Perhaps because segment arithmetic does not work in protected mode and segments are <=64K on the 286.

### Local Descriptor Table

The LDT is shared by all processes. Programs using the OS/90 flat model will use the GDT user segments instead.

Selectors provided to the LDT management services must be RPL 3 and LDT or an error will occur.

#### __8.1 Allocate LDT Descriptors..........................25__

A range of selectors is allocated and the base selector is returned. This is implemented with bit array allocation.

#### __8.2 Free LDT Descriptor...............................26__

A specific descriptor is freed.

#### __8.3 Segment to Descriptor.............................27__

Returns a selector that points to a real mode segment. Limit is 64K for each segment. If the allocation is more than one segment, it will generate the needed number and the last one will have a limit of size MOD 64K

#### __8.4 Get Next Selector Increment Value.................28__

For functions that allocate multiple descriptors, this function will return the value added to a descriptor to get the next entry. It takes no inputs and does not fail. I guess it will return `1<<3`.

#### __8.5 Reserved Subfunctions.............................29__

N/A

#### __8.6 Get Segment Base Address..........................30__

#### __8.7 Set Segment Base Address..........................31__

The higher order byte of the address is ignored if in 16-bit mode.

#### __8.8 Set Segment Limit.................................32__

The high 16-bits of the limit parameter are required to be zero for 16-bit clients, so this function can safely set the limit to whatever is requested regardless of bitness.

#### __8.9 Set Descriptor Access Rights......................33__

This function does not change the extended access rights of the 386 if the program is in 16-bit DPMI mode and the value in CH is ignored.

#### __8.10 Create Code Segment Alias Descriptor.............35__

IA32 does not allow writing to code segments. This will create a descriptor with the same limit and base address as the code segment but with read/write access.

#### __8.11 Get Descriptor...................................36__

Gets the 8-byte value of a descriptor. It pretty much just copies from the LDT.

#### __8.12 Set Descriptor...................................37__

Set a descriptor manually. This obviously cannot create ring zero segments. The caller is required to only use a DPL >= to the CPL, which is only 3 in our case.

#### __8.13 Allocate Specific LDT Descriptor.................38__

Allocate specific LDT descriptor: I wonder how many programs use this feature and why. It allocates a specific descriptor to the caller. At least sixteen must be reserved at the start of the LDT for this. Some may be already in use.

Using the default bitmap allocator is not enough because we do not want free reserved entries to be allocated. A separate bitmap is used to allocate reserved specific LDT entries, which is just a single DOWRD in size.

Freeing one of these is done with free LDT descriptor.

Ring three segment is required when calling this function, or equal to caller CPL. 32-bit code and data segments with page granularity are used.

### Memory Allocation and Unified MMGR

The specification of the memory manager defines a similar interface to DPMI. Both use a handle-based memory allocation system. DPMI returns the address of the block immediately after allocating, so the kernel freezes the block as soon as it is created. LDT entries must be set to point to the newly allocated block.

For the flat model, the LDT entries point to zero. The address returned by this function is a virtual address to the data. Segments are merely offsets to the linear address. This is used for loading the executable data into usable memory. For this reason, userspace software must be linked to use addresses above 1M+64K.

OS/90 has a unified memory management interface. XMS, DPMI, and the userspace API refer to the same internal functionality. XMS handles are a subset of the DPMI handles, as mentioned previously. This means that freeing an XMS block using DPMI is theoretically possible, though not recommended.

#### INT 31H 0800H: Physical Address Mapping

This maps a physical address to an arbitrary virtual address. The memory manager provides this functionality, although it is not virtual at all. To make this work, it must first go through the MMIO of any devices using the memory, so the virtual address space has to be mapped to an appropriate location for the IO. There is little reason for a DPMI application to access any other memory using this function, so access to any other memory is forbidden and restricted to only device virtual IO. This function is byte granular. I will just implement it as page granular and hope it works. Usually, this function is used to access video memory or some other memory-mapped IO region of a physical device, or in this case, virtual.

This feature utilizes the dirty bit in page tables to check if the data was written to.

## 16-bit Procedure Calling (Translation)

If a DOS call of any sort is not handled by a process-local hook, it is passed to capture chain or real mode on fallback.

When an INT instruction is simulated with DPMI, the process must switch to real mode and execute the interrupt there. Then it must go back to protected mode. In the PCB, we will store information to go back to protected mode.

## Entering and Exiting Protected Mode

All DPMI programs start in real mode. They must obtain a far call address to enter protected mode. An INT 2FH service is used for obtaining an entry point. When the entry point address is FAR CALL'ed, the program starts running in protected mode right after the FAR CALL instruction. This is used to implement XMS and the entry to protected mode routine.

The get entry point function requests to extra conventional memory (why would it need it?). All segments selectors are set to point to what the used to point to in protected mode, except for ES, which points to the PSP.

When a program is done running in protected mode, it can call INT 21H AH=4CH.

Then it will terminate. This does terminate the program that started the DPMI session.

### Raw Switching

DPMI allows for raw switching to and from real and protected mode.
