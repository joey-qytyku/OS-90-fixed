# Call Gate

System calls are implemented in an unusual way using a call gate where the offset field is interpreted as a function code.

If we used a software interrupt:
```
mov     eax,FunctionCode
int     WhateverVector
```

The first operation would take up five bytes. The second will take two. That is seven bytes to set the function code.

Now with a call gate:
```
call    word 83h:FunctionCode
```

A 16-bit far call is possible using the 66 prefix, and the syntax varies between assemblers. FASM for some reason uses DWORD as the override while NASM uses WORD. The 16-bit call is _required_ because a 16-bit function code is assumed and the opcode is checked for correctness. If the wrong opcode is used, the program will be terminated. In total, six bytes are used with this encoding.

Call gate invocations will terminate with a far return. It is essentially the same as a regular far call but it will change the CPL. GCC does not support far procedures and associated stack frames, so a simple assembly thunk will translate the call. Registers are used for parameters. The segment selector is garaunteed to be 83h, or 131 in decimal.

Only 32-bit DPMI or native programs can access the system call interface. 16-bit protected mode programs are simply ignored by the kernel. Addresses always assume the flat model.

# List of System Calls

DPMI and native programs can both access the system call interface. It is essentially just an extention on top of the existing DOS/DPMI interface.

The function code is 16-bit, where the high byte is the functional group and the low byte is the function.

Group | Name
-|-
AA | Program Control
B0 | Memory Manager
E0 | Events and Signals
|

## Installation check: INSTCK

```
Code = 0AA55h
OUT:
    EAX = Underlying DOS version code
    EBX = OS/90 version code
    CL  = Processor version
```

The function code is a reference to the PC boot sector. Versions are in BCD (e.g AH=6, AL=22). This is necessary in programs that could execute in a DOS-based DPMI server.

CL will equal 3 for 386, 4 for 486, 5 for Pentium, or 6 for Pentium pro.

## Load Dynamic Library

## Unload Dynamic Library

## Memory Block Size MEMBLKSIZE

Future versions of OS/90 may not use pool allocation, in which case, a 4096 byte block size is reported. This value will always be a power of two.
