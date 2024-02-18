# Virtual Memory Managment Unit

VMMU is a specification for a software interface that allows for somewhat efficient MMU on computers that do not support it. This document is written from an x86-16 perspective, but can be adapted to other architectures.

VMMU requires internal compiler support or manual use of macros or inline assembly, though this has limited use.

# Basic Concept

VMMU uses live patching to make it faster to access memory that is present. In the case of IA-16, segment registers and an optional offset register must be loaded.

VMMU implements paging internally and exposes a 32-bit addressing space (though 24-bit should be possible too and variants with reserved address bits are permitted). If the memory is present, the section of code that calls the translation function is live patched.

Paging is prefered since internal fragmentation is less problematic than external fragmentation.

For IA-16, the following interrupt vectors (currently undefined) are used:
- VMMU_XLAT_IMM
- VMMU_XLAT_FROM_FPTR
- VMMU_FAR_CALL
- VMMU_FAR_RET
- VMMU_MMAP

Example:
```
int VMMU_XLAT
DD  0xDEADBEEF
DB  VMMU_SEG_ES | VMMU_OFF_DI
```
This a total of 7 bytes long. To put that into context, loading to AX, writing a segment register, and loading an offset is...

```
B8 xx xx  ; mov ax,imm16
8E C0     ; mov es,ax
BB xx xx  ; mov bx,imm16
```
Eight bytes.

# Compiler Support

There a different levels of support for VMMU that compilers can have. 32-bit code segment addressing is possible but requires advanced linking and compiler support.

An alternative is to provide VMMU pointer types that generate the appropriate VMMU calls.

```
int __vmmu* pointer = 0x400000;
```

The pointer is a 32-bit little-endian value.
