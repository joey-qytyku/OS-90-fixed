# Source Code

OS/90 drivers and the kernel are written in an official format. Community software is recommended to adhere to these guidelines.

## Compiler Compatibility

The official compiler of OS/90 is no longer GCC, but the Digital Mars C/C++ compiler. As long as Type.h and a few other things are ported, other compilers can be used for non-kernel components.

## Data Types

### Integral Types

```
LONG,  SIGLONG          32
SHORT, SIGSHORT         16
BYTE,  SIGBYTE          8
```

Each has a pointer variant.

### Naming

All types are capitalized. The basic integer types have pointer forms with a simple 'P' added to the start. Any type that starts with P in its basic form should have an underscore after the P to indicate pointer. For example, `P_PNP_INSTCHECK`.

Astericks when not used to dereference or multiply should be avoided.

Pointer work a little different with typedefs.

- PBYTE: Pointer to byte
- const PBYTE: Constant pointer to a mutable byte
- PBYTE const: Mutable pointer to a constant byte
- const BYTE const: Const ptr to const byte

> Because PBYTE is one single unit, the const qualifier will apply to all of it, not just the base type.

### Signed Integers

Unsigned types should be used in every place where a signed type is not clearly needed.

### Strings

OS/90 primarily uses C strings. Arguments and pointers should use the type PBSTR. Actual strings should be defined using a BYTE array. `const char *` is prohibited, and so is any attempt to initialize a pointer to a `char`oid type to a string constant except for structure assignments.

## Procedure Naming

Pascal case is to be used.

Kernel code uses code section tags, which are captial letters with an underscore. These can be found in chapter 1, but are not supposed to be used outside of the kernel API.

Functions that convert a data item from one formt o another should have the output type explicitly in the name and indicate a FROM operation so that the type returned is on the left side.

For example:
```
BYTE BYTEFromLONG(LONG l);
```

## Variable Naming

Snake case is to be used for all variables. Global variables must be prefixed with `g_`.
