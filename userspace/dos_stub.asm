;
; The DOS stub for OS/90 programs. It is an MZ file, so FASM assembler
; must be used instead of nasm

    FORMAT      MZ
    STACK       1024
    ENTRY       Main

    SEGMENT     CSEG
Main:
    ; Obtain DPMI entry point
    mov     ax,1687h
    int     2Fh
    mov     [fpEntryPoint],di
    mov     [fpEntryPoint+2],es

    ; Enter protected mode
    call    far [fpEntryPoint]

    CODE32
In32BitMode:
    ; The program is in 32-bit mode
    ; Now we must call the kernel. It will generate
    ; the segments and allocate memory

    SEGMENT     DSEG

fpEntryPoint    DD 0
