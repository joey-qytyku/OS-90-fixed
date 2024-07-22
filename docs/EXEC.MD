# Executable Format (2024)

OS/90 has a special executable format for drivers and anything else.

It support relocations, shared libraries, and other features, but is simpler than other executable formats. Executables are contructed from objects and executables of the ELF format using readelf and a Python script parses the contents to generate the OS/90 executable.

The most innovative part of the format is alphabetically sorted symbol table entires. It is no coincidence that the kernel API uses Quake prefix naming.

The OS/90 executable format is bloated for very small programs and requires a memory-intensive linking method, but has extra information that greatly accelerates the loading process. The goal is to make OS/90 load drivers fast.

## Limitations

- Symbol names can accumulate to about 256M in size. This is practically a non-limit.
- 4G of code and 4G of data is possible, though it would never happen because of FAT limits.
- 256K is the maximum size of a symbol length group. Practically a non-limit.
- The whole format is very bloated, though perhaps not as bad as Win32 PE.
- The format is 32-bit. It could of course be adapted to 64-bit.

> Compression?

## File Structure

- OS/90 drivers end with the extension `.90`. There are no libraries since drivers are libraries.
- The extensions for userspace executables are `.CMD` and `.LIB`.

```
MZ Header
OS/90 Header
Import library table
Code and data *
Relocation table *
Import symbol table *
Export symbol table *
String table *
```

> Anything with * is 512-byte aligned and granular. This is 100% totally not related in any way to the size of a sector `:)`.

## Header

The OS/90 executable starts with an MZ header. e_lfnew points to the extended header. The magic number is 0x6A716A71 in little endian hexadecimal. Drivers can totally ignore the MZ header as the kernel mode loader checks for the extended header first.

```
#define EXEC_FLAGS_LIB 1
#define EXEC_DUPABLE_DSEG 2

typedef struct {
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

init and fini are both procedures and use the standard calling conventions. Libraries can use both to initialize and clean up. Drivers use the entry point for a different purpose as described in the drivers chapter of the manual.

> The format does not support arbitrary sections and hence has only one relocation table.

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

R_386_32: S + A calculation. This means the address of the symbol is added with an addend. The actual value referenced by instructions in .text or .data is always zero. The addend is zero and S is the real address. The purpose is to insert symbols that usually refer to variables or a function by absolute address (e.g. function pointer).

R_386_PC32: The instruction operand is generally negative the size of the instruction. For example, `call` would have -5 as an operand. The operation is S + A - P. This is used to insert near call addresses.

Any other relocations do not matter much unless the compiler tries to insert 16-bit call instructions for intramodular calls. Some compilers may do this.

## OS/90 Relocations

> I should switch to a "structure of arrays" so that I can compress the relocations. 16 bytes is too much.

OS/90 uses the exact same relocation scheme as ELF. The relocation tables are added into the final binary with a linker script. There is a rela table and rel table, both are separated.

```
typedef struct {
        SHORT   flags;
        SHORT   symbol;
        LONG    offset;
        LONG    addend;
}EXEC_RELA;
```

flags can be R_32/R_PC32, R_16/R_PC16, R_8/R_PC8. The values correspond directly with the values that ELF uses. The first eight bits are reserved for this purpose and all others must be zero.

OS90_REL is the same but without the addend.

## Symbol Reference Table (SRT)

Symbol tables do not contain strings, but string table-relative offsets. The index to each entry corresponds with a string in the strtab.

The SRT consists of a 24-bit value (SRT itself is file page aligned) and a length byte. These values are indices to the 4-byte array of the String Data Pool.

```
typedef struct {
        SHORT longs;
        SHORT srt_index;
}EXEC_SRT_ENTRY;
```

The first 4 entries are pointer/counts of the SYMENT groups. These are based on how large the symbol is in DWORDS, with the associated size being the maximum, not the exact. The order goes {8,16,32,other}. `SRT_ENTRY.index` is instead an index into the SRT and the `SRT_ENTRY.bytes` represents the size of the subtable but

> The rationale for this is that pasting symbols from a DLL can be accelerated. If the loader finds the length of a symbol referenced by the main module it can reduce the number of strings it needs to search.

## String Data Pool (SDP)

The string data pool starts with 26 32-bit file offsets totalling 104 bytes that represent subtables for each letter of the alphabet regardless of case. All of these offset entries are required and assumed to be there by the loader. A zero offset indicates that there are no symbols to search with that starting character.

> The linker must interpret the first character as some sort of case-insensitive letter and place it in one of the subtables. Z is recommended for anything that does not fit since it is rarely used for naming.

Each table begins at a 4-byte boundary starting location and has all entries space padded. The strings are in blocks of 4.

> There is no specific restriction on the contents of the strings. There is also no specific requirement for how the linker decides to alphabetically sort the strings, so name decorations like `_` or anything C++ does can be ignored. File size does not benefit by not sorting symbols.

# Load Procedure

It is easy enough to simply read the code section right to memory. The boundary that is (totally not) the standard sector size improves the load time. Relocations are applied in essentially the same way as in ELF on i386.

Symbol pasting is a bit more complicated and requires multiple lookups, but it ultimately much faster due to the alphabetical sorting. The process for inserting a DLL into another module is to lookup the symbol on the exporter and find out of the importer has that same symbol. Symbols are looked up by encountering a relocation that references it and finding the base address of the string using the SRT to get the string. Because of space padding, the strings can be compared with a simply `REP CMPSD`.
