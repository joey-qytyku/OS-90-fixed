# Executable Format

OS/90 has a special executable format for drivers and anything else.

It support relocations, shared libraries, and other features, but is simpler than other executable formats. Executables are contructed from objects and executables of the ELF format using readelf and a Python script parses the contents to generate the OS/90 executable.

## File Structure

```
MZ Header
OS/90 Header
Code and data *
Relocation table *
Import library table *
Import symbol table *
Export symbol table *
String table *
```

Anything with * is aligned in the file at a 512-byte boundary.

## Header

The OS/90 executable starts with an MZ header. e_lfnew points to the extended header. The magic number is 0x6A716A71 in little endian hexadecimal.

```
#define EXEC_FLAGS_LIB 1

typedef struct {
        LONG    magic;
        LONG    flags;
        LONG    foff_reltab;
        LONG    init;
        LONG    fini;
        LONG    code_bytes;
        LONG    bss_bytes;
        LONG
        LONG    num_syms;
        LONG    strtab_bytes;
}EXEC_HEADER;
```

## ELF Relocations

First, we will discuss how ELF does relocations.

Straight from the SysV ABI spec:
```
A This means the addend used to compute the value of the relocatable field.
B This means the base address at which a shared object has been loaded into
memory during execution. Generally, a shared object file is built with a 0
base virtual address, but the execution address will be different. See ??Pro-
gram Header?? in the System V ABI for more information about the base
address.
P This means the place (section offset or address) of the storage unit being
relocated (computed using r_offset).
S This means the value of the symbol whose index resides in the relocation
entry.
```

A is the addend field of the relocation entry.

S is the Sym.Value printed by readelf. If it is part of the calculation, it must be obtained from the symbol table to get the value.

P is the address of the thing being relocated. It is the offset field of the relocation entry and is section-relative.

R_386_32: S + A calculation. This means the address of the symbol is added with an addend. The actual value referenced by instructions in .text is always zero. The addend is zero and S is the real address.

R_386_PC32: The instruction operand is generally negative the size of the instruction. For example, call would have -5 as an operand. The operation is S + A - P.

Any other relocations do not matter.

## OS/90 Relocations

OS/90 uses the exact same relocation scheme as ELF. The relocation tables are added into the final binary with a linker script. There is a rela table and rel table, both are separated.

```
typedef struct {
        SHORT   flags;
        SHORT   symbol;
        LONG    offset;
        LONG    addend;
}OS90_RELA;
```

flags can be R_32/R_PC32, R_16/R_PC16, R_8/R_PC8. The values correspond directly with the values that ELF uses. The first eight bits are reserved for this purpose and all others must be zero.

OS90_REL is the same but without the addend.

## Symbol Tables

Symbol tables do not contain strings, but string table-relative offsets. The index to each entry corresponds with a string in the strtab.

## String Table

The string table stores each string in pascal format for fast traversal. Any time a string must be referenced, this table is looked-up.

Strings can be no longer than 255 bytes because of the 8-bit size specifier. They must also be aligned in the file by a 4-byte boundary.

## Execution and Memory Model

### Entry

A function with any name may be used as an entry point for drivers or programs. The entry point is an RVA subject to offsetting by the load base determined suitable by the loader.

The entry point is called with the standard OS/90 calling conventions. A stack is provided, but can be switched in userspace but never for drivers..

### Global Symbol Namespace

The string table and symbol table are both loaded into memory but are not directly accessible. The relocations are not because they are applied at load time.

In the case of drivers, all drivers share the same symbol space. Global symbols are exported to one giant global state. This means that naively compiling the driver could introduce symbols that could collide with other drivers or generally pollute the global namespace with unnecessary symbols. This is important when using functions that apply accross translation units of a driver but are not public to other drivers.

In driver code, the compile command should include `-fvisiblity=hidden`. This means that anything that is not explicitly marked with the macro OS_PUBLIC or the visible attribute is automatically not visible. This will not apply the correct calling conventions, so make sure that is done too if the compiler is not compatible.

When the linker is passed `--strip-all`, it will delete symbols which are not visible.

```
OS_PUBLIC VOID PublicFunction();
```
