# Joey Qytyku's Code Conventions

## Project Rules

OS/90 is intended to be the work of I, Joey Qytyku, and all code introduced to it that is not mine is credited appropriately, but OS/90 is NOT a team effort. If anybody wants to see their code added, please fork instead..

Almost all code contained here is mine, and I intend it to be that way, but if anything does not belong to me and proper credit is not given, please reach out.

OS/90 is licensed under GPLv2 and any compatible licenses and no proprietary software is required to run it either.

## C Style Rules

### Tabs or Spaces

Both. Tabs are used to indent and spaces are used to refine at the space level.

A special VSCode extension exists for this. If the editor does not have such a feature, spaces should be prefered. I don't make a big deal about this, just eb consistent.

### Naming

```
FunctionCase
MACRO_DEFINE_CASE
TYPE_CASE
variable_case
```

All structures must be typedefed to have a pointer type and a regular type:
- THETYPE
- PTHETYPE

If it starts with P, add an underscore:
- PTYPE
- P_PTYPE

### Bracket Style

```
typedef struct {
        ...
};

if (...) {
        ...
}

for (...) {
        ...
}

while (...) {
        ...
}
// For a while, brackets on the same line is optional
```

```
function declaration
{
        ...
}
```

### Function Style

If the function takes two parameters, the parameter list should look be formatted like this:

```
BOOL Vm_PageOp( PVOID base,
                LONG page_count,
                LONG opmask)
{
        ...
}

```
> The old style will be phased out.

Restrict pointer arguments should always use the RSTC macro over the type because not all compilers handle it the same way.

#### Argument Positions and Naming

Functions that take similar inputs should take them in roughly the same order.

```
VOID AddByte(BYTE *ch, LONG num) { (*ch)++; }
VOID SubByte(BYTE *ch, LONG num) { (*ch)++; }
```

## Design Principles

### Static Limits

Static limits should always be prefered over dynamic memory allocation because the latter is always slow on OS/90 and any other operating system running on an old computer.

For kernel code, do not allocate whole pages if the data is too small. The allocation functionality is deliberately barebones.

Static limits should still be robust. If they are exceeded, consider allocating memory in that rare case. Do not do the original Unix thing of silently truncating lines or something like that.

Dynamic structures should have this sort of hierarchy. Ask if one is sufficient before moving on down:
1. Static limit
2. Static limit and reallocate if exceeded
3. Resizable linear memory block that acts as an array
4. Linked list inside a linear pool
5. Sparse linked list with discontoguous blocks

### Goto

`goto` is good as long as it is not jumping backward. In general, C is meant to be more logical than assembly, but if the opposite is true, by all means use `goto`.

### Robustness

- Develop "library" style functions in another operating system.
- Use sentinel bytes to check for buffer overflows even in userspace
- Test edge cases
- Always check error codes
- Never run CPU instructions that are not detected first
- Always expect memory allocations to potentially fail
- Run tests with inline asserts and testing functions.
- Try to check pointers before dereferencing
- If unsure about the behavior of various hardware configurations or confused by datasheets, do tests and use asserts in debug code.
- Avoid assembly language unless it is clearly better.
- Check generated code to see if a rewrite in assembly is needed

