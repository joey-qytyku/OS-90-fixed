# Introduction

OS/90 features a preemptible, reentrant, multitasking kernel capable of emulating any 32-bit operating system.

# Memory Manager

OS/90 supports (or will):
- Virtual memory and swapping
- Uncommitted memory allocations
- Direct access to physical memory from kernel space
- Dynamic arrays, mappings, and other features
- Bit arrays

Features related to virtual memory are fully in the control of the subsystem driver or a subsystem-asscociated task using whatever inteface is provided by the former.

# Scheduler

OS/90 is preemptively multitasking and allows the kernel to be preempted by tasks.

## Exception Handling

OS/90 supports reentrant exceptions. This means exception handlers can be called from multiple processes. This is necessary in order to allow for concurrent system calls.

## Event Handlers

> Is this up to date? No.

Events are exceptions or software interrupt events defined by the IDT vector referenced, are caused by tasks, and are dispatched after system entry. Each process has a pointer to a system entry routine.

Events are caused by the user only. Exceptions in the kernel will instead go to a hookable list of handlers, which does not include INT calls because kernel mode cannot use INT.

## Scheduler Internals

### Standard Register Dump

```c
typedef struct {
        _DWB_R(A);
        _DWB_R(B);
        _DWB_R(C);
        _DWB_R(D);

        _DW_R(SI);
        _DW_R(DI);
        _DW_R(BP);

        LONG   pm_ES;
        LONG   pm_DS;
        LONG   pm_FS;
        LONG   pm_GS;
        _DW_R(IP);

        LONG   CS;
        _DW_R(FLAGS);
        _DW_R(SP);
        LONG   SS;

        LONG   v86_ES;
        LONG   v86_DS;
        LONG   v86_FS;
        LONG   v86_GS;
}STDREGS, *PSTDREGS;
// This structure will never change.
```

The reason why there are two segment register sets is to avoid complex branching in the system entry code. If exiting from V86, all data segments except for SS are set to zero. The system entry code pushes the segment registers, whether they are zero or perfectly valid protected mode ring-3 selectors (the kernel always uses the flat model) does not matter. They are restored later and the correct values will end up in the registers after IRET.

> In certain contexts, the STDREGS structure is actually not so standard and the last four V86-related fields can actually be omitted entirely depending on the context. The v86_xxx fields are only accessible in trap frames if the context was V86.

## Interrupt Reflection

OS/90 does not use SV86 to handle IRQs. To avoid the pointless latency of scheduling a process to handle SV86 IRQs or modifications to SV86 to suit interruptibility from itself and other issues, OS/90 switches to physical real mode to handle a reflected interrupt.

Switching to real mode directly allows SV86 to be interrupted by a real mode reflected interrupt with no additional complexity.

This is VERY slow, but it was done in Windows standard mode, so it can't be THAT bad, right? And switching out of real mode is much faster on the i386 than the 80286, so it is actually better than that. Furthermore, switching to V86 is not without its costs either.

According to the 80386 manual, IN is faster in protected mode but uses 26 clocks rather than 12 if running in V86 and not a privileged ring. INS uses 29 instead of 15. This means that V86 costs us 14 clocks per iteration for input, and 13 for output since its a bit faster.

This means that our approach is actually good for IO speed, though protected mode emulation of string ops would be faster.

On the i486, the same exact thing happens. I have no idea why PM would have faster IO. Maybe intel was trying to enourage us to use it more.

The OS/90 kernel API provides no API for accessing physical real mode for such purposes because it would be wrong anywhere outside the main IRQ dispatcher.

## Context Types in Detail

> Change to ABC?

T0: Interrupts disabled, preempt count irrelevant
T1: Interrupts enabled, preemption disabled
T2: Interrupts enabled, preemption enabled

Interrupts are fully non-interruptible and non-preemptible atomic contexts that totally finish upon executing. There are ways to handle IRQs with interrupts enabled, but it uses a thread.

T2 can be ring-0 or ring-3, while T0 and T1 are always the kernel, because processes can never change the preemption count. (imagine if they could!).

It could be possible to change the preemption count on behalf of a process and go back to the process.

## Thread Safety of API

The entire kernel API is thread safe unless otherwise specified. Some functions are reentrant and have 'r' appended to the end. Some functions can only run in certain context types.

# Supervisory Virtual 8086 Mode (SV86)

SV86 is a mechanism by which the kernel or any driver obtains a service provided by real mode software like the BIOS or DOS. OS/90 does not implement any of those features or interfaces natively and depends on the DOS interface.

SV86 is a special context under which no preempting can occur. It is entered through a separate RD structure.

These functions are related to SV86:
```
VOID OS_HookINTxH(BYTE vector, V86HND hnew, V86HND *out_prev);
VOID OS_INTxH_t012(BYTE vector, PSTDREGS regparm);
```

# Drivers (VERY WIP)

Drivers are essentially DLL's for the kernel. They can be used to implement the OS layer of a subsystem or can provide an interface to a device. They share a global symbol table and can use each other interfaces.

Drivers can have the extensions:
- BIO: Basic I/O
- SVC: Subsystem virtualization core
- SLI: Subsystem link interface

> There is one driver for every device of a certain class and there cannot be duplicate drivers.

> Drivers use a custom executable format. See EXEC.MD for more information on building procedures.
## Environment

The entry point procedure is called by the kernel. It will be using a very limited stack space, perhaps no larger than 512 bytes.

## Detection of Features

Driver interfaces implement something called a FEATURE. This is essentially a list of strings related to what features the system "supports." Each driver will specify the features it depends on and those that it implements.

The feature is bound to the driver that implements it, but a driver cannot be queried for its own features through this interface. A separate interface must be established for this.

```
VOID DriverEntry(VOID)
{}
```

## Drivers and Subsystems

### Device Driver and Link Driver

Drivers are meant to provide function calls to the global kernel namespace that allow for a device to be used in an abstract manner.

Devices drivers support various features, except for those related to device operation:
- Saving or restoring state
- Reseting a device
- Disabling device

The functions implemented by the device driver must be used by another type of driver called a subsystem link interface, or SLI. This does not technically have to be a separate file, but generally it should be.

The subsystem decides how it wants to handle any kind of IO request. Anything that goes to IO ports needs to be decoded using the appropriate kernel functions and sent to a SLI. The SLI can create a new device context for emulation, queue requests, etc. however it wants.

The interface is not formally defined,  but it is recommended that devices are handled one-by-one.

> How does a subsystem know what device to talk to?

> The naming protocal for SLI's is to add "BIO" to the name or insert into the last three characters to fit in 8 characters. Drivers must have 8.3 names. For example: `VGABIO.SLI` or 8042BIO.

> Can there be multiple links?

### Device Discovery

OS/90 keeps track of every link driver and the device driver it is bound to.

### Code Examples

> How do I deal with the concept of a bus?
```

static LINK ln;
static HDEV hdev;

VOID DriverEntry(VOID)
{
        if ((ln=OS_DeclareSubsystemLink(DRV_NAME_ASCIIZ, "8042", "DOS")) == -1) {
                OS_KernelLog("Error linking to subsystem");
        }

        hdev = OS_DeclareDevice(&dev_opcodes);
}
```

Devices can then be taken over by the susbsytem using this interface by asking the kernel to grant resources.

```
// SUBSYSTEM DRIVER

VOID DriverEntry(VOID)
{
        OS_RequestLink("8042");
}

BYTE first_time = 1;

VOID HandleIOPortAccess(IO_PORT_PACKET *p)
{
        ...
        LINK link = OS_GetLinkFromResource(RES_IOPORT, 0x60, 1, 0x64, 1);

        OS_RequestLink(link);


        OS_Droplink(link);
}
```

> Link is a const char * internally, but should never be changed so it is opaque.

It is up to the subsystem to create links and close them. Subsystems can share devices through this method.

### Realistic Examples

Some drivers implement a V86 interface and have no need to deal with subsystem-device interaction. For example, a disk caching or 32-bit disk/FS access driver; it does not need to support direct INT 13H access from DOS programs. This is not impossible, however.

Most devices cannot reasonably support context switching. To handle this, a device driver should either have a request queueing system or more often have a function that locks it.

The link between the subsystem and the device driver is exclusive as long as the subsystem keeps the connection. This does not mean that the device can safely multitask at the driver level because multiple tasks can run under a subsystem.

# Subsystem Drivers

Subsystems are drivers which implement a trap-and-emulate runtime for an operating system, and handle the execution and termination of tasks subordinate to it. Tasks are created by subsystems with limited kernel involvement.

The subsystem is defined by a subsystem descriptior block, or SDB.

```
typedef struct {
        SS_CMD  commands[16];
        SS_XH   exceptions[32];
}SDB,*P_SDB;
```

The command list:
```
enum {
        SSC_
};
```

- xh is the exception handler, but there is a catch. The procedures are not standard PSTDREGS consumers and take a pointer to the task that caused the issue and a code indicating the cause.

## Exceptions in Detail

The exception handlers are called by the real handlers in the kernel for the process, but not all the exceptions have subsystem-controlled handlers and some are abstracted.

These exceptions are supported:
```
VE_DE   Divide by zero
VE_GP   General protection fault
VE_AC   Align check
VE_UD   Illegal instruction
VE_FE   Floating point error (abstract)
```

Coprocessor segment overrun is passed to VE_GP.

## Example Subsystem

Building a subsystem requires a lot of code that can be quite

# API Ideas (OLD)

```
GS = Get or set based on input CF=0/1, by convention, ALWAYS set the carry flag before calling.
     Do not rely on its current value.

Get = Only gets
Set = Only sets

Reserve_IOPort_Region
Release_IOPort_Region

Get_Base_IRQ_Vector
GS_Lvl2_ISR
GS_IRQ_Mask


GS_Trap_Vector

Get_PID

> Process functions can work for current process or any process using special TID parameter.

Get_RD_Of_Proc

GS_IOPL_Of_Proc

Global_Copy_IOPB

Reserve_LDT_Descriptor
Free_LDT_Descriptor
LDT_Segutil

GS_Process_Priority
Terminate_Process
Block_Process

Do_Int86
Exec_Int86

GS_Poweroff_Proc

; Subsystem specification allows OS/90 to emulate many operating systems.
; Definitions currently:
; 0 = Custom/embedded
; 1 = DOS (DPMI/RM)
; 2 = Win16
; 3 = GNU/Linux 32-bit
; Means very little currently
GS_SSDB_Address

Get_Num_Running_Tasks

; The only way to ensure kernel is safe to enter within IRQ context.
Async_Try_Kernel_Global_Lock
Mutex_Lock
Mutex_Unlock

Mem_Info
Alloc_Chain
Free_Chain
Extend_Chain
Get_Chain_Size
Set_Swap_File
Get_Swap_File_Name
Swap_Out_Pages
Swap_In_Pages

List64_New
List64_Del
List64_Add    (CF=1 uses position, works like the cursor of a text editor)
List64_Swap   (CF=1 and it swaps any element with the top of the stack)
List64_Get    (CF=1 uses position, otherwise gets the top)
List64_Set    (CF=1 uses position, otherwise sets the top)
List64_Remove (CF=1 uses position)

Fixup_Driver_Library
Load_Driver
Unload_Driver

Get_Boot_Disk_BIOS_ID

```
