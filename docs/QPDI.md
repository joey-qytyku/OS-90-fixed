# DEPRECATION NOTE

I have decided that allowing a program to access the standard syscall interface along with the DOS is a bad design choice. The goal of native OS/90 applications is to abstract the DOS interface and avoid such redundancies. For that reason, I will maintain support for DPMI, but will also allow programs to be native and not use DPMI at all.

I did some investigation with WfW 3.11. I called INT 10H AX=3 to change to text mode, and it switched to text mode. I was even able to print a hello world string.

# QPDI

I have canceled DPMI support and instead concentrated efforts QPDI, which is incompatible and supports the internal structure of OS/90 better.

General Concepts:
* Supervisor instructions will always cause errors
* All programs must run in ring-3
* Built-in support for the flat model
* 32-bit code

There is no 16-bit code support, so there is practically no reason to support segmented memory models. The goal is to limit interractions between protected and real mode as much as possible and avoid redundancy.

All function calls will save registers unless specified as outputs or clobbers.

## Extended INT 21H

Many DOS extenders and the true DPMI implementation of Windows support a translated INT 21H API. QPDI also does. It uses interrupt calls internally and allocates bounce buffers for calls that require it. Function calls that read or write data are limited to 64K. Near pointers used by regular DOS are converted to 32-bit pointers, with segments being disregarded. 16-bit registers are converted to 32-bit and return values are zero extended if necessary. 8-bit returns leave the rest of the register unmodified.

Any functions outside of INT 21H must be called using INT 41H.

### Restrictions

The DOS API available to QPDI programs is strictly controlled.

These actions must be forbidden:
* Functions 4B, 4C, 4D, 4E
* AH=55h, create PSP
* INT 20h
* Calling any BIOS function or real mode ISR

Vectors 22h-28h:
* INT 25h/26h absolute disk R/W
* INT 27H terminate and stay resident
* Set or get interrupt vector through DOS
* Calling INT 22-24H

Other functions are subject to virtualization by the supervisor. Some must be virtualized such as:
* Extended error information
* XMS and EMS

Far calling procedures in real mode is supported.

Example:
```
        mov     ah,9
        mov     edx,hello
        int     21h

hello:
        DB      "Hello, world!",10,13,36
```

## Critical error, Control-break, idle loop

* INT 22H: Exit address
* INT 23H: ^C
* INT 24H: Critical error

These interrupts are never called by the software, but by the system. It is desireable to allow protected mode software to set its own handlers. If it does not, the program must be terminated by the supervisor with great prejudice, as calling other software is impossible.

COMMAND.COM implements a critical error handler for most programs.

## Error Codes

Nearly all function calls can fail.
```
Q_OK
Q_ERROR
Q_SYSTEM_INTEGRITY
Q_OUT_OF_MEMORY
```

## DPMI Incompatibility

The QPDI supervisor must report that DPMI is not present thorugh INT 2Fh, as DPMI is incompatible with QPDI and cannot exist on the same system.

## Initiation and Exit Section

When in real mode, the program has to use INT 255 to prepare the environment. When it is inside the environment, INT 41H is used. Exiting protected mode is not allowed. Subprocesses of the program will be immediately terminated if possible.

### Version

```
INPUT:
    EAX     = 0x0000

OUTPUT:
    EAX     = Q_ERROR if already in protected mode
    EBX     =
```

### Enter Protected Mode from Real Mode

This runs only in real mode. Upon entry, The segment selectors will point to descriptors that match the input segment state, but in protected mode. Once the code being loaded is fixed up and ready to execute, the program can then switch to the segments returned by the version function.
```
INPUT:

OUTPUTS

TO CALL:
    INT     0FFh
```

### Terminate Protected Mode Program

```
INPUT:
    AL      = Return code
```

## Input and Output Section

IO instructions are emulated using interrupt calls to avoid the decode overhead. This means that a failed IO attempt will not halt a program.

String operations may ultimately fail and complete partially.

### Simulate INS

```
INPUTS:
    EAX     = 0xA000
    EBX     = Operand size (0=8, 1=16, 2=32)

    EDX     = Port
    ESI     = Data pointer
OUTPUTS:
    EAX     = Q_OK or Q_ERROR
```

### Simulate OUTS

```
INPUTS:
    EAX     = 0xA001
    EBX     = Operand size (0=8, 1=16, 2=32)
    ECX     = Count

    EDX     = Port
    ESI     = Data pointer

OUTPUTS:
    EAX     = Q_OK or Q_ERROR
```

### Simulate IN
```
INPUTS:
    EAX         = 0xA002
    EBX         = Operand size (0=8, 1=16, 2=32)

    EDX         = Input port

OUTPUTS:
    EAX         = Q_OK or Q_ERROR
    ECX/CX/CL   = Data
```

### Simulate OUT

```
INPUTS:
    EAX         = 0xA003
    EBX         = Operand size (0=8, 1=16, 2=32)

    ECX/CX/CL   = Data
    EDX         = Port

OUTPUTS:
    EAX     = Q_OK or Q_ERROR
```

## Memory Manager Section

Managing memory in conventional memory is done by calling the DOS API. Extended memory can be allocated with QPDI. Virtual memory is none of the business of the virtual machine.

### Allocate Linear Region

```
INPUTS:
    EAX     =
    EBX     = Number of pages
    ECX     = Flags
            [0] = 1 if read only memory

OUTPUTS:
    EBX     = Base address
    ECX     = Handle
```

### Delete Linear Region
```
INPUTS:
    EAX     =
    EBX     = Handle to free
```

### Acquire Virtual MMIO Range

```
INPUTS:
    EAX     = 0xB000
    EBX     = Base address
    ECX     = Size of range in pages
OUTPUTS:
    EAX     = Q_SUCCESS, Q_ERROR, or Q_OUT_OF_MEMORY
    EBX     = Virtual address to access
```

This function is typically used to access framebuffers. In virtualized implementations, the MMIO range should be carefully checked.

## Video Control Section

Programs may want to set their own video modes. Doing this through the BIOS could be possible, but BIOS calls are prohibited in protected mode. The video mode values correspond with the VGA BIOS mode codes.

In a multitasking system, it is necessary to arbitrate access to the video modes so that programs can run in fullscreen until termination, where the default mode can be used.

### Set Video Mode

```
```

### Get Video Mode

## Interrupt Section

The interrupt flag does not reflect the status of interrupts for the virtual machine.

DPMI allows programs to hook exceptions. QPDI does not because it violates isolation. Instead, special error signals are sent by the supervisor. If a signal handler is not set, the virtual machine is terminated. Signal handlers are part of the virtual ISR class. Control C and critical error, and idle loop are implemented using signal handler.

Interrupt service routines
* The state of virtual IF is 0 if an IRQ and indeterminate in all other cases
* Near return and use a near call stack frame
* Use the ring-3 program stack

An ISR should only call INT 41H functions that are approved in such contexts. If a function is not supported in an ISR, an error will occur.

### Set Virtual Interrupt Mask

Programs may want to disable a virtual interrupt. This is done with the following function.
```
INPUT:
    EAX     = Get/Set
    BX      = Mask value
OUTPUT:
    BX      = Output mask if get
```

### Get/Set Virtual Interrupt State

```
INPUT:
    EAX     = x if Disable, y if Enable
    AL      = State to set (0 or 1)
OUTPUT:
    AL      = State returned
```

This function cannot fail.

### Set Signal Handler

### Set Virtual IRQ Handler

```
INPUTS:
    EAX     =
    EBX     = Address
    ECX     = IRQ (0-15)
```

Setting IRQ#2 is invalid on the PC/AT architecture and will cause an error.
