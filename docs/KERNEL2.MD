# Introduction

OS/90 features a preemptible, reentrant, multitasking kernel capable of emulating any 32-bit operating system. It is designed to force few abstractions.

# Memory Manager

OS/90 supports (or will):
- Virtual memory and swapping
- Uncommitted memory allocations
- Direct access to physical memory from kernel space
- Dynamic arrays, mappings, and other features
- Bit arrays

Features related to virtual memory are fully in the control of the subsystem driver or a subsystem-asscociated task using whatever inteface is provided by the former.

## Virtual Memory

# Scheduler

OS/90 is preemptively multitasking and allows the kernel to be preempted by tasks.

## Exception Handling

OS/90 supports reentrant exceptions. This means exception handlers can be called from multiple processes. This is necessary in order to allow for concurrent system calls.

## Event Handlers

Events are exceptions or software interrupt events defined by the IDT vector referenced, are caused by tasks, and are dispatched after system entry. Each process has a pointer to a system entry routine.

Events are caused by the user only. Exceptions in the kernel will instead go to a hookable list of handlers, which does not include INT calls because kernel mode cannot use INT.

## Scheduler Implementation

OS/90 uses an unconventional scheduler design, using a form of system-level ucontext/longjmp style approach to implement multitasking.

The basic concept is that of the __context__. Contexts are a standard register dump structure containing the set of registers that is created when entering the system from an interrupt gate, entering SV86 or exiting, or entering a process context or saving one.

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

There are procedures for entering different

## Context Switching

Contexts can be entered by calling an assortment of different routines which generate a stack frame. Each context switching function is non-reentrant and can


The context switching is mandated by a "shadow program" that consists of a single function with no return and an endless loop that schedules processes and runs the next process.

> Consider single linked list? Also do we need to have a "current process"?

## Interrupt Reflection

OS/90 does not use SV86 to handle IRQs. To avoid the pointless latency of scheduling a process to handle SV86 IRQs or modifications to SV86 to suit interruptibility from itself and other issues, OS/90 switches to physical real mode to handle a reflected interrupt.

Switching to real mode directly allows SV86 to be interrupted by a real mode reflected interrupt with no additional complexity.

This is VERY slow, but it was done in Windows standard mode, so it can't be THAT bad, right? And switching out of real mode is much faster on the i386 than the 80286, so it is actually better than that. Furthermore, switching to V86 is not without its costs either.

According to the 80386 manual, IN is faster in protected mode but uses 26 clocks rather than 12 if running in V86 and not a privileged ring. INS uses 29 instead of 15. This means that V86 costs us 14 clocks per iteration for input, and 13 for output since its a bit faster.

This means that our approach is actually good for IO speed, though protected mode emulation of string ops would be faster.

On the i486, the same exact thing happens. I have no idea why PM would have faster IO. Maybe intel was trying to enourage us to use it more.

The OS/90 kernel API provides no API for accessing physical real mode for such purposes because it would be wrong anywhere outside the main IRQ dispatcher.

# Supervisory Virtual 8086 Mode (SV86)

SV86 is a mechanism by which the kernel or any driver obtains a service provided by real mode software like the BIOS or DOS. OS/90 does not implement any of those features or interfaces natively and depends on the DOS interface.

SV86 is a special context under which no preempting can occur. It is entered through a separate RD structure. (Technically, we can make it multitasking as long as no more than one process enters.)

SV86 is emulated by default in the kernel.

# Emulation Support

Since there is no default behavior for INTs, exceptions, virtual IRQs, etc, it is necessary to provide subsystem drivers the necessary utilities to implement the operations.

This includes:
- Simulate far call/far return
- Simulate INT/IRET in 16-bit or 32-bit mode

# Drivers

## Executable Format

Drivers are very simple files. They are flat binaries with a list of relocations appended to the end.

### Design Considerations

The way they are generated is by merging any object files into one, generating a list of the relocations in binary, converting the object file to flat binary, and appending the relocations.

i386 ELF has two types of relocations that matter: R_386_32 which is a simple addition by the addend. R_386_PC32 is relative to the instruction pointer (S+A-P).

The linker will eliminate PC32 relocations. They are only needed when a procedure needs to be near-called with a relative 32-bit address and the only option is to use an absolute address in the binary and relocate it later. For position-dependent code like OS/90 drivers, only R_386_32 relocations matter. These will simply add to the addresses.

### Recap

The header:
```c
struct drv_exec_header {
    uint    magic; // "JQJQ"
    uint    bss_rva;    // Relative address of uninitialized memory.
    uint    bss_pages;  // Pages of uninitialized memory
    uint    code_bytes; // Number of bytes of actual code bytes
    uint    rel_off;    // FILE offset to relocations
    uint    rel_num;    // Number of relocations
};
```

BSS uses pages because that is the simplest way to allocate it. It is not in the actual file so there is no need for byte granularity for space-saving reasons. Code using a byte count so that padding bytes are avoided. In the end, both "sections" are allocated using page granularity.

Relocations are code-local offsets, or RVA's (relative virtual address). They are nothing more than 32-bit pointers.

### Executable Image

The executable in memory behaves like a flat binary. It will be loaded at a page aligned boundary. Alignment of progbits sections is the responsibility of the linker.

The BSS section is not section in the truest sense. It is simply a region of memory garaunteed to be exactly above the executable data. BSS is always zeroed.

### gendrv.py

This python script takes the path of an elf executable (not object) composed of every translation unit of a driver. It then generates the OS/90 executable.

This process is quite simple. Getting the contents of the merged .text section is easily done by

## Environment

The entry point is the load point. For initialization, the kernel initialization stack is being used, which is under a kilobyte in size (how big?). Avoid large stack allocations and recursion.

> BSS is always zeroed, as with the kernel.


# API Ideas
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
