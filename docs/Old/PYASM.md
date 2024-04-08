> WIP PROTOTYPE

> How do we deal with nested macro expansion?
> Why not just use NASMX?
> I can run multiple passes.

# PyAsm

PyASM is a Python library that acts as a pre-processor for assembly code. It works with NASM only at the moment.

The idea is that you insert definition sections into the top of source files (must be at the top) where python code can define macros. The rest of the file goes through a simple parsing process.

The default is at least three equal signs to start and three to terminate.

# Examples
```python
======================================================

def invoke(args_array):
    args_array = args_array[::-1]

    for x in args_array:
        if x.is_number():
            emit(f"push {str(x)}\n")
        elif x.is_string:
            emit("section .data\n")
            emit(f"DB {x}\n")
            emit("section .text\n")

add_macro("invoke", invoke)

======================================================
```

Externs must be specified using PyASM so that it can keep track of the symbols.

Comment characters can be reconfigured and automatically converted to semicolons.

# Implementation Details

Lines are tokenized individually in a similar way to how nasm would do it. Token types include symbols, numbers, and strings.

A symbol can also be an instruction. A list of instructions is already built in.

# The Library

PyASM provides a class called NasmValue. This represents a number, string, or symbol name. Using str() on it will convert the NASM object to a string.

Symbols can include or start with dots.

## emit(string)

Emit a string to the final output. New lines are not automatically outputted.


# PyAsmLib

PyAsmLib is a library that adds support for:
- Register parameter declarations

The MOVE macro does not emit a move instruction if the move is redundant.

ALLOCA allocates space on the stack and stores a pointer in the specified register. This requires the use of a base pointer.

## Example

Commas are used as a delimiter to merge tokens before passing them to the macro implementation.
```
        SECT<.data>

tosquare SET<U32, 10>

Change to Square CPROC<U32 x>?

    SECT<.text>

Square CPROC<U32 x>
        mov     eax,[local.x]
        imul    eax,eax
        ret
ENDP<Square>

main CPROC<I32 argc, PTR argv>
    mov U32 []

```

> There is no need for type checking, except for variables maybe...
> Type checking can be done by checking the tokens. Except in cases of movzx/movsx, we can figure out the type of the whole operation by checking for any size specifiers or registers.

> Symbols use @ sign? Separate macro syms from actual syms?

```
unit = Unit("main")
unit.sections = {
    '.text':unit.TEXT
    '.data':unit.DATA
}

align(16)
with PROC('Example', {'argc':U32, 'argv':PTR}).args() as argv:
    mov(eax, argv['argc'])

    with LB('.label'):
        jmp('.label')

    ret()

```