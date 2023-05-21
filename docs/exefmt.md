# DEPRECATION NOTE

OS/90 is not using its own executable format any more. We are going to use the Windows Portable Executable instead.

# Executable Format

OS/90 uses a special executable that is meant to be simple like the Windows PE/COFF while removing the Microsoft-specific things. It supports dynamic load libraries. NXF is only meant to be used within OS/90 for userspace software or drivers.

Structure:
|NXF Format|
-|
Header
Relocation Table
Symbol table
Program Data

## Header

```c
typedef struct {
    DWORD       magic; /* 'JQJQ' */
    DWORD       prog_pages;

    // Number of pages to allocate for uninitialized memory
    // Garaunteed to be zeroed on entry
    DWORD       bss_pages;

    // Number of pages to allocate to the default heap.
    // The heap is automatically set to use the virtual addresses
    // after BSS.
    WORD        heap_size_init_pages;

    // The symbol table is part of the program data.
    WORD        symtab_rva;
    WORD        symtab_size_bytes;

    DWORD       reloc_rva;

    // RVA of stack
    DWORD       stack_init_rva;

    // RVA of entry point
    DWORD       entry_point_rva;
}OS_EXE_HEADER;
```

The NXF format uses relative virtual addresses, refered to as RVA in the docs. Originally, I planned to make all addresses file-relative, but this proved to be unnecessary and made conversion from PE more complicated.

The stack is a location in the BSS section. The size is defined in the header. The entry point of the driver simply points to the driver header, it is not the real entry point. The RVA is relative to the program data section, or in other words, relative to were the file is loaded in memory.

If a file were to load at 0x110000, and a symbol were defined at 0x110010, the RVA of the symbol would be 0x10. File offsets can be converted to RVAs by subtracting the size of the DOS stub.

## Sections

There are three "sections":
* PROG
* BSS
* HEAP

Heap and BSS do not exist in the executable, but their boundaries are defined in the header. The stack can be located anywhere. The size does not need to be specified, as the BSS section should be large enough to handle it. BSS is always zeroed.

## Imports and Exports

Static libraries require the use of the import

## Symbols

The symbol table is a list of strings matched with addresses. It is used to export functions and global variables to other translation units. The symbols are stored in the program data section and referenced by the header using an RVA. The symbol table is relocated by the relocation table since it contains RVAs that need to be translated into absolute addresses.

The unresolved address is an index to the symbol table. The symbol table entry that is to be imported has an address of 0xFFFFFFFF.

```
; Code
mov eax, dword [Unresolved]

;Symtab

DB "Unresolved", 0
DD 0FFFFFFFFh
```

Resolving symbols is a very slow process. Currently, the loader will simply scan one symbol and compare every single entry in the library, making the overhead scale exponentially.

## Relocation

In order for addresses to be relocated, a table of relocations is used. The relocation table is simply a list of 32-bit pointers. It works just like the MZ format, but without segmentation.

The address actually in the program before loading are relative to the load address. Because of this, relocations can be completely removed for EXE files in favor of absolute addresses. The toolchain offers this option for reducing file size.

Drivers make use of relocations as they may be loaded anywhere in the kernel address space.

## Dynamic Libraries

## EXETOOL

EXETOOL is a linux program that loads a PE object file as a memory mapped file and converts it to the OS/90 native format executable. Sections must be aligned to 4096-byte boundaries or an error will occur.

### Usage

Command line parameters:
```
path_to_stub_binary
entry_point_symbol_name
load_address
stack_pointer_symbol
output_file_name
input_file_name
Y/N
```
Examples
```
exetool stub.exe MyEntryPoint 0x110000 StackInit output.exe input.obj Y
```
The file name must not have spaces.

### Executable Generation Process

* Compile program to ELF
* Convert it to a single PE .EXE using the linker script
* Use EXETOOL

### EXETOOL Implementation

PE and NXF both use RVAs. PE allows sections to be loaded at any address as long as there is no overlap. NF does not. EXETOOL will simply require that .code and .data sections are merged into one and aligned at a 4K boundary.

## The Execution Control Specification

INT 0FFh is used by OS/90 to access certain features. It is used to automate program loading and dynamic linking. It can be called from real mode or the procedure can be called using KeExeCtlProc() with a register dump pointer argument, as this function is automatically linked to the executable.

All registers not specified as clobbered for outputs are preserved unless stated otherwise.

# Rationale for NXF

PE could have worked fine, but I found it to be far too bloated. It contains a lot of data that is irrelevant for OS/90. I also did not see the point in having named sections with independent virtual addresses. I created NF as a sort of 32-bit extention of the MZ format with dynamic library capability.

NXF files cannot be executed by DOS, so I was able to put the header at the start of the file.

## Ideas

Dynamic loading is like many other things in computer science. There are pros and cons to every solution. In this section, I list solutions that I considered for implementing dynamic load libraries and their possible pros and cons.

### Interrupt

One idea for implementing libraries is to use an interrupt vector to initate dynamic linking and generate a fixup. A near call with an immediate operand is created. Note that x86 uses relative near jumps.

This would be difficult to implement in C. Calling a procedure naturally would be basically impossible and some kind of wrapper would be required. This is no greater in overhead to dlsym on Unix, however. Once the symbol has been resolved, the program will call it normally.

The implication of this method is that the executable does not need to have unresolved symbols in the file. They can be part of the program data.

```
    INT     40h
    DD      LookupName

LookupName:
    DB      "HelloWorldProc",0
```

Example in C:
```

const char *YieldCPU = "YieldCPU";

USTAT MainProc(DWORD argc, PBYTE argv)
{
    LCALL(0, YieldCPU);
}
```

The macro for calling the procedure is complicated. It needs to support C calling conventions. Is it even possible?

The better way to do this is to include a header with wrapper definitions into the source code. The header would have something like this:

### The Old-fasioned way (What we are doing)

The normal way is to implement getting addresses dynamically and to have static library linking at run time. Static library imports must be known at load time. The advantage of static linking is that procedures can be called without any wrappers and there is no runtime performance loss.
