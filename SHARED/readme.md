> Needs updating

# Shared Modules

This folder contains implementation files for C-library functions that can run in kernel mode and user mode using certain defines.

## printf

> This will link as a library.

A standard printf is provided.

This module has been changed to not be portable. It can only work within the i386 ABI.

### DISABLE_FLOAT

If defined, floating point is turned off and float conversions are UNDEFINED.

### Interface

_printf_core is used to implement the entire family of formatting calls. It is like vsnprintf but with a buffer commit callback.

There are two callback types:

commit_buffer_f: Write a character array into the buffer
dupch_f:	 Duplicate a number of characters into the output destination

### Required Declarations

- strlen
- memcpy
- memset

These should be declared with the C standard signatures or anything functionally equivalent in the target architecture.

## String

This module has implementations of string instructions in assembly and C.

"string" is a library that must be linked. Several of the procedures cannot be inlined and must be called.

## chclass

Character classification functions are not included in the kernel for performance reasons and because there is little use for them.

This has data overhead of about 512 bytes. Functions are also not all required and use separate translation units, plus are always inlined unless the header file is not included.

The inlining should amount to nothing more than a simple test instruction.


