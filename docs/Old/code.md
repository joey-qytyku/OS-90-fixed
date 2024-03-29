# About This Document

The goal is to establish how the source code is written. Some of these are just good practice while other rules are style and personal prefference.

It is recommended that programs written for OS/90 are written in accordance with at least the style guidelines.

# Code Rules

The entire source code of the OS/90 kernel and associated drivers follow these guidelines.

# C Guidelines

## Style Rules

These rules decide how the code should be styled and have no affect of functionality.

### Brackets

Always use wide open curly brackets for function scope. It is easier to read.

```
int main()
{
    // This is how
}
```

For structures, its okay since they don't have much nesting.

### Switches

Code between `case` labels should be tabbed.

### Capitalization


Name     |Case|
-|-
Function | Hybrid_Case
Variable | snake_case
Type     | MACRO_CASE,P_MACRO_CASE
Macro    | MACRO_CASE

### Spaces and Tabs

* Indents are four spaces
* Line endings are always CRLF for DOS/Windows compatibility
* Struct members and long parameter lists should be aligned with spaces
* Lines should be no longer than 80 characters for viewing in DOS

### Code Conventions

* Extern globals in header files should be prefixed with underscore.

I have determined that functions with long parameter lists are most readable when formatted as:
```
return-type FunctionName(
    type    param,
    type    param
){
        // Code
}
```

Structure types that may need to be used as pointers should be defined with a pointer variant. They should also use `tstruct` or `tpkstruct` (byte packed) instead of the full `typedef struct`. This is because I always misspell `typedef`.
```
tstruct {}NAME, *P_NAME;
```

If a structure should never be used as either a pointer or an object, one of the definitions can be omitted. `P_` is the recommended prefix for the pointer type.

Integer types do not have the underscore.

## Programming Rules

The following are rules for how OS/90 software written in C should operate and may affect how code is generated by the compiler.

The target platform demands high code density and minimum memory usage.

Never:
* Use pre-increment or decrement, this is confusing to read

Never pass and return structs (It's bad for performance). User pointers instead. 80x86 is a very register-starved architecture and cannot transfer non-trivial structures easily.

Structures should be as self contained as possible and should contain everything needed to work with a set of data.

```c++
typedef struct { int i; ... }EXAMPLE,*P_EXAMPLE;

void Good(PEXAMPLE j);
void Bad(int j, PEXAMPLE k);
```

## Comments

Single line comments only except if they do not work (macro comments)

## Global Variables

Global variables are acceptable as long as they are accessed using static inline functions or macros in the same header file that externs them. An underscore prefix should be used to indicate that they are abstracted by other functions in the header. Otherwise, globals should be avoided.

## Structures

Structures of arrays should be prefered in most cases unless the reverse is more acceptable. This is to improve cache locality and reduce memory usage.

Structures should be used to encapsulate complex data.

## Parameters

No three star programming is allowed. The only time pointer parameters need to be used is if the function needs to return it to the caller along with another value through the return type. In other situations, double and triple pointers are not a good idea and there is very little need for that sort of indirection.

Array parameters should never be used (eg. param[x]) because it does nothing useful. Arrays, however, can be passed to functions by value in place of a pointer.

## Types

### Strings

OS/90 has strict rules in regards to strings.

What type to use:
* `const char *` or `char *` for a __function parameter__ or pointer to a string in any scope.
* `const char[]` or `char[]` for a string that behaves as array of bytes __in file scope__.
* Apply `const` if the string/character does not __change location or values__.

Rules for using strings:
* __Never__ define a string in local scope, even with static.
* __Never__ define strings with global visibility.
* __Never__ initialize `char *` or `const char *` with a string constant. Use `const char[]` in file scope.
* Use `[const] char *` for __parameters__ to function `ONLY` with no exception.

A `const char*` parameter does not need to be passed with the exact type. It is okay to pass a pointer to a non-constant pointer paramter since it only ensures the function does not change it.

## Mutual Exclusion

If two procedures operate on identical input and are both called by the same caller, mutual exclusion of these procedures must be done by the caller. Telling a function to simply "take care of it" should be avoided, as the mutual exclusion conditions are separated despite being related.

Prefered:
```c
if (cond1)
    Proc1();
if (cond2)
    Proc2();
```

Not Prefered:
```c
Proc1();
Proc2();
```
Proc1 and 2 have the conditions in their function implementation. This makes

# Optimization Guidelines

Some are general concepts, while other tips here are specific to OS/90.

* Optimize the code that runs repeadedly and is the slowest, aka speed critical
* Avoid optimizing code that runs only once or takes the least execution time
* Use a structure of arrays when possible or when benefitial
* Pack structures and use bit fields to save memory
* Align certain structures for performance

## Kernel Code Size and Memory Usage

Currently, the bid for maximum kernel size is 48K. The compiler uses size optimization by default.

The kernel must avoid memory allocation like the plague. The only things that should be dynamically allocated are process control blocks and other large data structures that are page-multiple sized.

# Assembly Guidelines

x86 assembly code must be compatible with the Netwide Assembler.
* Tabs should be used between instructions and operands.
* Tab size is 8 spaces
* No spaces after the comma
* Cases same as C for procedures
* No instructions after a label on same line
* Separate code with comments, maximize readability.

Local variables are not really a concern here.

Register parameters can be passed in any way that is convenient, as long as it is described in comments.

Variables should use Hungarian case because assembly is typeless, but this is not a huge deal because assembler code is rare in OS/90 and is for things that are hard to do in C.

```
i{any type}Name - Signed
wName - Word
bName - Byte
lName - Long, aka dword
qName - Quad word
sName - String of bytes
wfpName 16-bit far pointer (SEG:OFF16)
lfpName 32-bit far pointer (SEG:OFF32)
{j,f}Name   Jump/call target or function pointer
p{w,b,l,q,s}Name pointer to {x}, eg. pwName
a{w,b,l,q,s}Name array of {x}
```
Examples:
```
pawVgaTextMem = 0B8000h
asArgv
ilArgc
```

As seen in some of the examples, the type info may be too verbose, so comments should be prefered if it's too long.
