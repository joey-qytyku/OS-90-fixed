> Redirecting IRQs to userspace?

> Can we store address mapping in an array for each process? This way, we dont need to allocate page tables? Probably not.

> Should I rename drivers to something else? Maybe system service (SSV or SVC)

# Introduction

OS/90 features a preemptible, reentrant, multitasking, based on an exokernel-inspired design. The kernel is capable of emulating any 32-bit operating system.

# Memory Manager

OS/90 supports (or will):
- Virtual memory and swapping
- Uncommitted memory allocations
- Direct access to physical memory from kernel space
- Dynamic arrays, mappings, and other features
- Bit arrays

Features related to virtual memory are fully in the control of the subsystem driver or a subsystem-asscociated task using whatever inteface is provided by the former.

## Virtual Memory

### Page Fault Handling

# Scheduler

OS/90 uses cooperative multitasking with advanced priority capabilities.

## Cooperative Multitasking

OS/90 uses cooperating multitasking. This seems impossible for an OS designed around emulation, but it does work. All processes have to perform IO or somehow invoke the OS in order to do any useful work. Each of such events represent a potential implicit yield to the next process.

Whether or not a yield occurs is under the control of a driver. This allows fine-grained control which preemptive multitasking does not, but there is reduced consistency.

## System Entry

The kernel does not differentiate between exception and INT calls. Both using ring-0 IDT entries that cannot be called from user mode directly. A system entry is the invokation of any IDT vector by userspace. It will always generate an exception.

Exception contexts are task-local and capable of yielding. Handlers must be thread-safe.

If an IDT entry of ring-0 is called by userspace, a #GP is caused. The GP handler chain is invoked until the exception is swallowed. One of the handlers should check if there is an INT or similar instruction at the stored CS:EIP and proceed to dispatch it to the correct handler.

> Use a far call with offset as the functiton ID?

## System Exit

Invoking the system exit is done by finding the exit location and simply jumping to it. This will cause the system to restore the userspace (always) register dump state, returning to the process that entered the kernel. The CR3 value stays the same because the process has not changed.

The address of system exit is right after the registers on the stack. A simple indirect jump is all that is needed using the offset `RD.exit`.

## Yield

Yield cannot be a jump location because it may not actually occur and the process should be returned to in such a case. The location instead is a procedure call with no arguments.

## Tasks Control Block

The task control block is a standardized structure that is 4096 bytes long and naturally aligned. The begining is the 32-bit register dump. This RD includes segment registers for real mode and protected mode. The real mode sregs are not valid if the task was in protected mode and vice versa.

> What about hooking SV86? I think we need a capture chain for it.

The reason why there must be PM and RM registers is because the system entry procedure avoids conditional processing and unconditionally pushes the segment registers. Exiting V86 zeroes all DS,ES,FS, and GS. By the time we push them back, only the stack segment will be used in order to pop the rest of the INT/IRET stack frame.



The rest of the TCB is defined as
```
typedef struct {
    RD  rd;
    PVOID   _yield;   // ?
    PVOID   _sysexit;

    _TCB*   next;
    _TCB*   prev;
    U32     id;
    U16     priority;
    U16     counter;

    U32     general_flags;
    U16     subsystem;
    U16     architecture;

    SCHED_PROC  post_proc;
    SCHED_PROC  pre_proc;
    SCHED_PROC  term_proc;


}TCB, *P_TCB;
```

## Kernel Stacks

Kernel stacks are at least 3K in size and are part of the TCB.


## Reentrancy of the Kernel

Mutex locks work as expected, but have implicit yield loop semantics to prevent deadlocks. The priority is also dropped significantly for the task spinning on the lock. It will raise the spin flag in this special case.

The TCB is always safe to access since each system entry is tied to one instance of the process.

> The lock function tries the lock first and if it is held, spin is raised

> The entire kernel is a sort of class with methods and tasks are instances of the class.

## Priority Levels

Each chance to run yield as a no-op is called a "void yield." Each void yield will decrement the counter and if it goes to zero, the number is reset to the priority number.

When the scheduler encounters the zero counter, it will reset it to the original value and repeat it.

A negative priority level causes each counter decrement to mean one time that the process is ignored by the scheduler before it runs normally.

## Exception Handling

OS/90 supports reentrant exceptions.

## Event Handlers

Events are exceptions or software interrupt events defined by the IDT vector referenced, are caused by tasks, and are dispatched after system entry. Each process has a pointer to a system entry routine.

Events are caused by the user only. Exceptions in the kernel will instead go to a hookable list of handlers, which does not include INT calls because kernel mode cannot use INT.

# Supervisory Virtual 8086 Mode (SV86)

SV86 is a mechanism by which the kernel or any driver obtains a service provided by real mode software like the BIOS or DOS. OS/90 does not implement any of those features or interfaces natively and depends on the DOS interface.

SV86 is a special context under which no yielding can occur. It is entered through a separate RD structure. (Technically, we can make it multitasking as long as no more than one process enters.)

SV86 is emulated by default in the kernel.

# Emulation Support

Since there is no default behavior for INTs, exceptions, virtual IRQs, etc, it is necessary to provide subsystem drivers the necessary utilities to implement the operations.

This includes:
- Simulate far call/far return
- Simulate INT/IRET in 16-bit or 32-bit mode

# Drivers

## Executable Format

Drivers are very simple pieces of software. They are flat binaries with a list of relocations appended to the end.

The way they are generated is by merging any object files into one, generating a list of the relocations in binary, converting the object file to flat binary, and appending the relocations.

i386 ELF has two types of relocations that matter: R_386_32 which is a simple addition by the addend. R_386_PC32 is relative to the instruction pointer (S+A-P).

The linker will eliminate PC32 relocations. They are only needed when a procedure needs to be near-called with a relative 32-bit address and the only option is to use an absolute address in the binary and relocate it later. For position-dependent code like OS/90 drivers, only R_386_32 relocations matter. These will simply add to the addresses.

### Recap

The header:
```c
struct DRV_EXEC_HDR {
    U32     magic; // "JQJQ"
    U32     rel_off;
    U32     rel_num;
};
```
Relocations are code-local offsets, or RVA's.

## Environment

The entry point is the load point. For initialization, the kernel initialization stack is being used, which is under a kilobyte in size (how big?). Avoid large stack allocations and recursion.

# API

## ABI

The stack is used for all parameters. It is automatically cleaned. Arguments are to be pushed in reverse order as with cdecl.

> The ABI is not cdecl compatible.

Calling the API is done using a far call to LDT entry 16. The function code is the offset, which normally would be discarded.

## Library Drivers

Any driver can act as a library and expose an API to other drivers.

Libraries may be called with whatever calling mechanism is necessary. OS/90 has a protocol for obtaining a library entry point LDT entry using the name of the driver, with automatic loading as well.



# API Ideas
```
TODO:
    Process becomes task. "proc" means procedure.

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
> Add hooking IDT entries? Maybe not? V86 and PM require different behavior. Each process needs the event handler.

# C and C++ Support

The ABI is compatible with newer versions of GCC with advanced inline assembly features. OSCall is an inline function.
```
OSCall(List64_New, &handle);
OSCall(List64_Add, 5, 0);
OSCall(List64_Delete, &handle);
```

Better support, including for C++ could be implemented later.

```
List64 list = List64();

void Driver_Main()
{
    list64.add(10L);
    list64.add(15L);
    list64.add(1, 20L);
    // 10, 20, 15
}
```
Wrappers like this would have zero abstraction cost and should be almost completely inlined.

Process register dumps can easily be accessed using a constant pointer with the __fs attribute.
```
P_RD const _rd = (P_RD)0;

```

GCC allows for this:
```
goto *_rd.YIELD;
```
