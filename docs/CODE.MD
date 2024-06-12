# Source Code

## Structures and Types

### Integral Types

```
LONG    32
SHORT   16
BYTE    8
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

- `SIGINT` is to be used for return types that convey an OS/90 error code.
- Unsigned types should be used in every place where a signed type is not clearly needed.

### Strings

Strings should be defined as arrays of characters for any named string constants to avoid allocating a needless address field. Character pointers are only for parameters.

Strings are pointers to or arrays of unsigned bytes. `const char *` is not allowed. Use `PBYTE const`. The latter type is compatible witht he former.

## Procedure Naming

Pascal case is to be used.

It does not look nice, but is actually a great idea. It forces the programmer to think about important attributes of functions and makes wrong code look extremely wrong.

Callbacks also use this system.

Prefixes:
- i386: Low-level functions for CPU (Kernel only)
- TI: (comes before numbers) Safe inside ISR or IRQ off section. This may be because it disables INTs. Just because a function is reentrant, does not mean an ISR should use it!
- T0: Safe for interrupts off context caller
- T1: Safe for non preemptible context caller with interrupts on
- T2: Safe for preemptible context caller (may still not be thread safe)
- T{0,1,2}: Safe for any combination. For example, T01 or T12
- Tx: Safe for 0,1,2
- Ts: Thread safe (does not imply reentrant nor T2).
- Re: Reentrant (implies safe in all cases, including As)
- Um: Uses mutex lock
- Xm: Expects mutex is held
- Xpd: Expects preemption disabled (may remove)
- Xid: Expects interrupts disabled (may remove)
- D{p,i} Disables preemption or interrupts

This naming convention is stringly recomended and should be used even for non-API code.

For example, the ISR function can have _TI appended to it.

Functions that convert a data item from one formt o another should have the output type explicitly in the name and indicate a FROM operation so that the type returned is on the left side.

For example:
```
char charFromLONG(LONG l);
```

## Variable Naming

Snake case is to be used for all variables. Global variables must be prefixed with `g_`.
