# Executable Format

OS/90 uses a special executable that is meant to be simple like the Windows PE/COFF while removing the Microsoft-specific things. It supports dynamic load libraries. Programs that have special requirements that do not fit this format can use DPMI to setup their environment differently.

The file naming conventions are NXF for executables, LIB for libraries, and DRV for drivers. The format is called OS90_NXF (native executable format). NXF files can be executed from the DOS prompt because the loader for all executable types is 32-bit and notices the NXF extention.

Structure:
|NXF Format|
-|
Header
Relocation Table
Symbol table
Real mode stub
Program Data

## DOS Stub

Each file has a DOS stub which is always executed upon program entry. The stub is right after the header. It is executed first and the rest of the executable is not in memory until the loading process is initiated.

NXF uses a non-standard file extension, which means the DOS stub does not need to be the begining.

### Procedures for the Stub

This section describes what all executables must do in the stub. The stub must enter protected mode eventually using the INT 0FFh API. It does NOT need to print a message for when it is run in DOS mode because NXF cannot execute on actual DOS. The loading process is already completed when an NXF is executed, and the only thing needed is to enter the environment.

OS/90 drivers enter protected mode immediately based on the driver header. They do not use the stub at all. LIBs do not use the real mode stub either because they are not executable.

## Header

```c
typedef struct {
    DWORD       magic; /* 'JQJQ' */

    // Where to load the program, prefered base address
    DWORD       load_address;
    DWORD       prog_pages;

    // Number of pages to allocate for uninitialized memory
    // Garaunteed to be zeroed on entry
    DWORD       bss_pages;

    // Number of pages to allocate to the default heap.
    // The heap is automatically set to use the virtual addresses
    // after BSS.
    WORD        heap_size_init_pages;

    // The symbol table is part of the program data.
    //
    WORD        symtab_rva;
    WORD        symtab_size_bytes;

    DWORD       reloc_rva;

    // RVA of stack
    DWORD       stack_init_rva;

    // RVA of entry point
    DWORD       entry_point_rva;
}OS_EXE_HEADER;
```

The NF format uses relative virtual addresses, refered to as RVA in the docs. Originally, I planned to make all addresses file-relative, but this proved to be unnecessary and made conversion from PE more complicated.

The stack is a location in the BSS section. The size is defined in the header. The entry point of the driver simply points to the driver header, it is not the real entry point. The RVA is relative to the program data section, or in other words, relative to were the file is loaded in memory.

If a file were to load at 0x110000, and a symbol were defined at 0x110010, the RVA of the symbol would be 0x10. File offsets can be converted to RVAs by subtracting the size of the DOS stub.

## Sections

There are three "sections":
* PROG
* BSS
* HEAP

Heap and BSS do not exist in the executable, but their boundaries are defined in the header. The stack can be located anywhere. The size does not need to be specified, as the BSS section should be large enough to handle it.

## Imports and Exports

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

LIB functions are accessible using a single symbol which acts as a struct pointer with function pointer entries.

```c
#include <Core.h>

DWORD Main(DWORD argc, PIMUSTR argv[])
{
    $CORE.Logf("Hello, world!\n\r");
}
```

This solution has performance problems like the ELF global offset table. When loading the library, the addresses still have to be rebased, but there is no need to resolve symbols in the requesting executable, as they can be referenced through the structure. Global variables can be exported using libraries, but this obviously should be avoided.

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

PE and NF both use RVAs. PE allows sections to be loaded at any address as long as there is no overlap. NF does not. EXETOOL will simply require that .code and .data sections are merged into one and aligned at a 4K boundary.

## The Execution Control Specification

INT 0FFh is used by OS/90 to access certain features. It is used to automate program loading and dynamic linking. It can be called from real mode or the procedure can be called using KeExeCtlProc() with a register dump pointer argument, as this function is automatically linked to the executable.

All registers not specified as clobbered for outputs are preserved unless stated otherwise.

### Execute Native Format in Protected Mode

```
Inputs:
    AX    = 8010h
    ES:BX = Program segment prefix
Outputs:
    All registers destroyed. Segment registers are loaded with flat model selectors (including ES,FS,GS). Program will run the entry point.
```

Normally, DPMI programs allocate memory blocks and set segment descriptors up to point to them. Internally, OS/90 will simply use page frame allocation to map the data to the load location.

The segment selectors will be set to the GDT entries for userspace programs. The LDT is only used if the program explicitly request LDT entries.

### Driver: Insert Kernel Symbols

Drivers do not use DLL files and that sort of dynamic linking. They use a fixed kernel symbol table with names of functions and their addresses.

Make sure to give exported functions original names or errors may happen.

```
Inputs:
    EAX = 8011h
    ECX = Driver header
    EBX = Pointer to symbol list
    EDX = Number of symbols to add to table

Outputs:
    None

Must be called in protected mode by a driver.
```

This function can be safely called before initializing dynamic linking. Symbol entries have the following format:

```c
typedef struct
{
    DWORD   address;
    DWORD   address_of_name;
};
```
The name refers to the symbol table in memory.

### Driver: Initialize Dynamic Link

This will paste the symbols for each null symbol table entry into the code. Call this before using any kernel API features (besides KeExeCtlProc). By default, missing symbols are set to point to a procedure that should print an error message.

```
Inputs:
    AX      = 8012h
Outputs:
    None

Must be called in protected mode by a driver.
```

### User: Load Library

```
Inputs:
    EAX = 8013h
    EBX = Name of library control symbol (null terminated, no longer than 24)
    ECX = Filespec (null terminated)

Outputs:
    EAX = FFFFFFFF if failed
          0 if successful
    EBX = Actual load address


The program must be in protected mode.
```

### User: Detach Library

# Rationale for NF Executable Format

PE could have worked fine, but I found it to be far too bloated. It contains a lot of data that is irrelevant for OS/90. I also did not see the point in having named sections with independent virtual addresses. I created NF as a sort of 32-bit extention of the MZ format with dynamic library capability.

NXF files cannot be executed by DOS, so I was able to put the header at the start of the file.
