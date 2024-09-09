# EF90 (2024)

> This is NOT currently being used. A simpler format is in use for kernel drivers.

EF90 is an executable format for drivers and userspace programs using the 32-bit flat model. It support relocations, shared libraries, and other features, but is simpler than other executable formats.

The most innovative part of the format is alphabetically sorted symbol table entires.

EF90 is bloated for very small programs and requires a memory-intensive linking method, but has extra information that greatly accelerates the loading process.

## Limitations

- No named segments
- Symbol names can accumulate to about 256M in size. This is practically a non-limit.
- 4G of code and 4G of data is possible, though it would never happen because of 32-bit addressing.
- 256K is the maximum size of a symbol length group. Practically a non-limit.
- The whole format is very bloated, though perhaps not as bad as Win32 PE.
- The format is 32-bit. It could of course be adapted to 64-bit.
- A total of 65,536 kilolibraries can be imported. Practically a non-limit.

## File Structure

There is no specific file extension required.

```
MZ Header
EF90 Header
Code and data *
Relocation table *
Import symbol table *
Export symbol table *
String table *
Import library table
```

Anything with * is 512-byte aligned and granular. This is 100% totally not related in any way to the size of a sector `:)`.

This makes the minimum size of an EF90 executable over 2.5K. This is not bad because the average cluster size in FAT is 8K-16K.

## Header

The executable starts with an MZ header. After the MZ stub, there must be an EXEC_HEADER. A loader has the option to not require the MZ stub, but it must be supported if it exists.

```
#define EXEC_FLAGS_LIB 1
#define EXEC_FLAGS_CMP 2

typedef struct { // COMPLETELY OUTDATED
        LONG    magic;
        LONG    flags;
        LONG    foff_reltab;
        LONG    init;
        LONG    fini;
        LONG    code_bytes;
        LONG    bss_bytes;
        LONG    num_syms;
        LONG    strtab_bytes;
}EXEC_HEADER;
```

EXEC_FLAGS_LIB indicates that the file is a library. This means that relocations can be purged from memory but the symbol table must be retained. Only exported symbols should be exposed.

EXEC_FLAGS_CMP means that the executable is compressed.

All other flags must be zero.

init and fini are both procedures. The exact ABI is not specified. Every OS should specify its own since executables cannot run accross operating systems anyway.

BSS must be zeroed by the loader.

> The format does not support arbitrary sections and hence has only one relocation table.

## ELF Relocations

First, we will discuss how ELF does relocations.

Straight from the SysV ABI spec:
```
A This means the addend used to compute the value of the relocatable field.
B This means the base address at which a shared object has been loaded into
memory during execution. Generally, a shared object file is built with a 0
base virtual address, but the execution address will be different. See "Pro-
gram Header" in the System V ABI for more information about the base
address.
P This means the place (section offset or address) of the storage unit being
relocated (computed using r_offset).
S This means the value of the symbol whose index resides in the relocation
entry.
```

A is the addend field of the relocation entry.

S is the Sym.Value printed by readelf. If it is part of the calculation, it must be obtained from the symbol table to get the value.

P is the address of the thing being relocated. It is the offset field of the relocation entry and is section-relative.

R_386_32: S + A calculation. This means the address of the symbol is added with an addend. The actual value referenced by instructions in .text or .data is always zero. The addend is zero and S is the real address. The purpose is to insert symbols that usually refer to variables or a function by absolute address (e.g. function pointer).

R_386_PC32: The instruction operand is generally negative the size of the instruction. For example, `call` would have -5 as an operand. The operation is S + A - P. This is used to insert near call addresses.

Any other relocations do not matter much unless the compiler tries to insert 16-bit call instructions for intramodular calls. Some compilers may do this, but GCC has not been observed doing this. Even multipass assemblers like FASM do not perform such optimizations (personal experience).

## OS/90 Relocations

> I should switch to a "structure of arrays" so that I can compress the relocations. 16 bytes is too much.

OS/90 uses the exact same relocation scheme as ELF and uses similar definitions. Relocations are the only part of the format that have subpar loading performance due to poor alignment. This is to reduce file size.

```
enum {
        R_32
        R_PC32

        R_16
        R_PC16

        R_8
        R_PC8
};
```

Relocations have a confusing format, so pay attention.

The top 4 bits of the first 32-bit value fetched are the opcode. If the relocation type does not need an offset, 28 bits can be read to
????

> Why not batch up relocations? If the same symbol is used for something, there is little reason not to.

All relocations are in the same array. The addent is only present if the flags indicate that it is needed. The full 32-bit value is used.

## Symbol Reference Table (SRT)

Symbol tables do not contain strings, but string table-relative offsets. The index to each entry corresponds with a string in the strtab.

> Symbols are ALWAYS imported or exported. The format is not designed as an intermediary object format.

```c
typedef _Packed struct {
        SHORT bytes;
        SHORT sdp_index;// DWORD array index to SDP entry.
}EXEC_SRT_ENTRY;
```

The number of bytes is rounded to DWORD values in most operations, but the actual size is still important (e.g. printing the name of a symbol).

### First Two Entries

The first two SRT entries are special. The SRT is partitioned into a section for symbols with an even number of bytes in the symbol names and other for odd, in that order.

The format is instead:
```
BYTE  0
SHORT <offset>
```
It represents the number of even-sized symbols.

## String Data Pool (SDP)

The string data pool starts with 26 32-bit file offsets totalling 104 bytes that represent subtables for each letter of the alphabet regardless of case. All of these offset entries are required and assumed to be there by the loader. A zero offset indicates that there are no symbols to search with that starting character.

> The linker must interpret the first character as some sort of case-insensitive letter and place it in one of the subtables. Z is recommended for anything that does not fit since it is rarely used for naming.

Each table begins at a 4-byte boundary starting location and has all entries space padded. The strings are in blocks of 4.

> There is no specific restriction on the contents of the strings. There is also no specific requirement for how the linker decides to alphabetically sort the strings, so name decorations like `_` or anything C++ does can be ignored. File size does not benefit by not sorting symbols.

# Import Library Table

The ILT is an array of libraries which must be loaded. Symbols are required to specify which library to import from. There is no alphabetical craziness here. Because each symbol specifies a library, there simply is no need.

The kernel automatically imports all necessary symbols for drivers. Importing symbols from another driver can be done statically if there is a hard dependency.

# Load and Dynamic Link Procedure

It is easy enough to simply read the code section right to memory. The boundary that is (totally not) the standard sector size improves the load time. Relocations are applied in essentially the same way as in ELF on i386.

Symbol pasting is a bit more complicated and requires multiple lookups, but it ultimately much faster due to the alphabetical sorting. The process for inserting a DLL into another module is to lookup the symbol on the exporter and find out of the importer has that same symbol. Symbols are looked up by encountering a relocation that references it and finding the base address of the string using the SRT to get the string. Because of space padding, the strings can be compared with a simply `REP CMPSD`.

# Toolchain Guidelines

A second stage linker exists. The program must be compiled to a 32-bit DLL. Symbols should be stripped as part of this process.


