# Deprecation Note

I have decided that the dynamic recompiler is not worth the effort. It was a nice idea, but far more complicated than what I was trying to avoid. This document and some source code still remain for historical reasons.

# ModRM Observations

ModRM-using opcodes seem to appear in groups of four on the opcode list every other four opcodes starting with 00, whose range is ModRM.

This ends at 3B

Don't think this is workable.

# Real Mode Dynamic Recompilation

OS/90 never uses virtual 8086 mode. It runs all 16-bit software in a highly optimized dynamically recompiling interpreter within ring-0.

The advantage over regular emulation is that register transfers, IO, and memory access are not emulated in software, which means even a poorly implemented dynamic recompiler would be much faster than an interpreter, but this will not be poorly implemented.

# TODO

Add 32-bit prefix support. This is the only way programs can access 32-bit DPMI.

# Hypothesis

If my recompiler is well-designed then it should be able to emulate some programs with LESS overhead that virtual 8086 mode due to a lack of mode switches. On the i386, a mode switch is about 99 clocks, and the full mode switch process is even slower.

String IO instructions are already confirmed to be faster in ring-0 protected mode due to IOPB bypass, and are actually even faster than in real mode (according to i386 manual).

TODO: I will try the Dhrystone benchmark on: Windows 3.11 DOS prompt in exclusive mode, OS/90, and real mode DOS.

# Terminology

Trace buffer: A 128-byte location in memory where translated instructions are placed
Retrace: Forcing the trace buffer to be executed immediately
Scheduling: Placing an instruction in the trace buffer

# Recompiler Behavior

## Context Switching and RECL_16 (TODO)

The dynamic recompiler can execute 16-bit interrupt service routines. In this situation, it is not running in a threaded context.

## Size of Instruction

Getting the size of an instruction is not that simple because the 8086 has ModRM which can change the size of the instruction even with the same opcode.

Only the MOD bit is necessary. We do not need the R/M parameter because that describes the specific addressing mode.

|MOD|Added Bytes|Meaning
|-|-|-|
|00| 2 | Memory no disp
|01| 1 | Memory 8-bit disp
|10| 2 | Memory 16-bit disp
|11| 0 | Register only

First, we need to know if the opcode uses a ModRM byte. Some instructions use immediate values or the ISA has shortcut codes, so assuming they are all one byte is not correct.

## Trace Buffer and Machine States

The trace buffer is always 128 bytes. It will go in the PCB of all processes for real mode support. It is executed by a CALL and exited by a RET.

## GS Segment (TODO)

The GS segment is saved for all kernel thread contexts. It allows the generated recompiler code to access the kernel's memory. GS cannot be used by any real mode software and doing so is an error.\

## Segmentation

Real mode code is the exact same as 16-bit protected mode code except for the handling of segment registers.

The following instructions read or write registers:
* mov sreg,rm16
* mov r16,sreg
* push sreg
* pop sreg

The recompiler uses its own dedicated DS, ES, SS, and CS. These are stored in the GDT.

### Data/Extra Segment

These segments will behave as expected. They will have 64K limits. The GDT entry for them will be modified whenever a segment register load takes place.

### Code Segment

The code segment has a base and limit within only the execution window.

### Segment Override Prefix

If a segment override is found, a flag is set. On the next iteration, the segment override flag is reset to have a "no override" meaning.

## Prefixes

Instruction prefixes are part of the opcode. They cannot be emitted on their own.

### Emulating Segment Instructions

Real mode software must be able to perform segment arithmetic. This is done by outputting real mode segments for sreg loads and writing the base addresses into GDT descriptors for sreg writes.

__Move to Segment Register__

mov sreg,rm16 has two forms. One has a register operand and the other has an RM operand. We will expect the register form but the other must be supported.

The other form can be optimized. The following sequence is generated
```
push ax
mov ax,sreg
jmp 8h:RetraceMovSreg
```

This will retrace.

__Push/Pop__

This requires access to SS:SP, which means that a retrace is required.

## Privileged and IOPL-Sensitive Instructions

The recompiler runs in ring 0. This means that mode switches as a result of executing the code, which improves performance. This does make the execution of privileged instructions more difficult.

### IO Operations

IO cannot be directly performed. If the recompiler is in supervisor mode

### IRET and INT

The INT instruction is essentially just a special CALL instruction in ring-0, in which case it does not save SS:ESP on the stack. It is more memory dense for the trace buffer to hold an INT instruction for running INT, but a near CALL is faster because it does not need to load from the IDT or save EFLAGS.

IRET is not emitted the same because it is not a safe instruction and will actually IRET if it runs.

### Branches

Conditional branches force the entire trace buffer to be executed and its results written back.

Unconditional branches do not suffer from the same problem. The recompiler can simply keep scheduling instructions in the trace buffer. Branches not taken also do not incur this penalty.

Intersegment branches can use an immediate CS:IP value or a ModRM. These will change the code segment register.

### Call, Return, and Protected Mode Entry

Call and return use a virtual call stack. A call pushes an old ISP value and a return pops it to the current ISP. Procedures that modify the IP on the stack will not work. An error occurs if the call stack is exhausted, but it is quite large.

Besides the stack behavior, the near CALL instruction is essentially an unconditional branch, so it does not retrace. RET does not either.

The RETFD opcode will always initiate a switch to protected mode using the stack to get the CS:EIP values.

## Self-Modifying Code

The OS/90 dynamic recompiler has to support self-modifying code in order for overlays to work. It will check for writes to code using these two conditions:
* CS=DS=ES=SS (tiny model). We check by ANDing them together and checking if the result is the same
* CS override is being used.

If the CS override is being used, the operation is assumed to be self-modifying code and a retrace will happen.

Self-modifying code, at least according to the Intel manuals, does not work consistently on an i486 and can sometimes fail due to cache coherency between the prefetch cache and L1. OS/90 will make it always work.

## Main Loop

We get the number of bytes left in the buffer. Next, we fetch a byte from the instruction stream and index the opcode table with it. If it is not a branch instruction, it will be copied directly if space allows. At least one byte must remain for a RET opcode.

Instructions that use segments are fixed up.

IO opcodes are copied directly as well. The kernel will deal with emulation if necessary.

# Notes

The code inserted into to the buffer can be larger than what exists in the original code. I can emulate complex instructions with this sort of method.

# Idea

I can add an intermediate phase where I place pseudo-branch codes that are later checked for possible optimizations.

# Optimizations

> Manual update to the ISP?

Some of these optimizations are geared toward the i486 and its handling of cache.

## Speeding up Branches

Let us suppose the architecture of the recompiler requires execution and register writeback for all conditional branches to access the flags. This would make most programs much slower. Conditional branches are very common and used in tight loops.

### Solution #1: Break up the Trace Buffer

It could be possible to insert "retrace points" inside the same trace buffer to execute part of it and write back

### Encode it Directly if Possible

## Register Saving

Partial register access is slow on pipelined processors like the i486, but real mode code will use them anyway, so why even bother? When the registers are saved, they will go in 16-bit locations.

Any 16-bit code that uses 32-bit overrides will not function.

## Lookup Tables

The following lookup tables are used:
* Use ModRM
* Default operation size (including ModRM)
* ModRM addend table

The Use ModRM table is a bit list that is 32 bytes long for each 8086 opcode.

The default operation size uses one byte per entry. The addend table does too.

## Code Optimizations

The entire recompiler is written in assembly and disregards calling conventions except when the stack is needed.

All branch targets and procedures are aligned at 4-byte and 16-byte boundaries respectively. Branches are generally avoided unless necessary.

All variables are assigned to naturally aligned boundaries or at 16-byte boundaries if they fit together.

## Agressive Loop Optimization

(THIS MAY NOT BE CORRECT)

If the LOOP instruction is detected, the recompiler applies an optimization where it retraces previous code and copies the looped region into the trace buffer for execution. This only applies to the generic LOOP operation and not the others.

This could be problematic if there are nested loops with the LOOP instruction and PUSH CX.

A static overhead of the optimization is that a retrace is required. The benefit is that the loop itself will run at native performance.

### Candidacy

There are two factors that determine the effectiveness of the optimization:
* Loop iterations
* Size of the looped region

Implicitly unrolling loops is not a terrible idea. The trace buffer is already cleared.

The rule with the OS/90 dynamic recompiler is the following:
* Unroll if these are all true:
  * Looped region <=16 bytes
  * All iterations fit inside the loop buffer
* Regular optimization if:
  * Anything not unrolled

Unrolling avoids the overhead of the LOOP instruction.

The LOOP instruction is slow. Unfortunatly, we cannot generate superior code because LOOP does not change flags and DEC does.

### Usefulness

The LOOP instruction was commonly used for hand-written assembly programs due to its code desnity.

# Implementation of Dynamic Recompiler

There is a main loop that fetches instructions, checks for special opcodes, finds their size, and copies them to the trace buffer. Copying to the trace buffer is an independent operation that can be done at any time. If there is no more space, a retrace does not need to be caused and the requesting code will know that there is no more space.

The buffer has a location counter which is reset when the buffer is "cleared"

The following operations are supported and can be invoked at any point in time, but ordering can influence the results of the translated code:

__Fetch Instruction (FETCH):__

FETCH reads a byte from the input stream, finds the size of the opcode, and copies the instruction into the trace buffer. The speed of fetching the data from memory will vary between processors in speed because REP MOVSB is used. Pentium has very fast string operations.

FETCH will take into account prefixes and will not fetch prefixes if the rest of the opcode does not fit.

FETCH can have three possible outcomes:
* Copied the data successfully
* Ran out of space
* Special instruction detected

```
IN:     None

OUT: (If copied successfully)
    EAX := Undefined
    ECX := 0
OUT: (If ran out of space)
    EAX := Undefined
    ECX := 1
OUT: (If special instruction found)
    EAX := Opcode of special instruction (zero extended)
    ECX := 2
```

__Retrace (RETR):__
This will execute the trace buffer, save the results, and the buffer will be ready for new instrcutions.

__Execute without Retrace (XWRT)__

This will execute the trace buffer without clearing it and write results. This is used for repeating code blocks without having to fetch more instructions.

__Discard and restore last trace (DSCR):__

This will discard all data in the trace buffer and command the recompiler to clear the trace buffer and reload the original context. This is useful for optimizations or any other special handling of instructions where the alternative code cannot fit in the buffer.

__Emit__

Special instructions need to be emitted differently to emulate their behavior. If they do not fit, a retrace is required.

## Format of Code

The recompiler has a main loop that calls subroutines for the commands specified above.

We keep an input stream pointer and a trace buffer pointer. These values remain in registers while translating and are saved to memory when executed.

The ISP occupies ESI. EDI is the TBP. The direction flag is always cleared by the main loop.
