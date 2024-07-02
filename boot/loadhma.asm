; Note that this should be assembled with FASM, not NASM, since the latter
; does not support DOS EXE files.
;
; Loads the kernel binary into the high memory area after allocating all of it.
; Then the loader allocates whatever is left of the extended memory with XMS
; and passes the base address and size to OS/90.

        format  mz
        entry   CSEG:start

        segment CSEG
start:
        mov     ax,CSEG
        mov     ds,CSEG
        mov     es,CSEG
        ; TODO, save the PSP

        lmsw    ax
        test    al,1
        jnz     CONT

        mov     ah,9
        mov     dx,messages.protected_mode_software
        int     21h

        mov     ax,4C00h
        mov     bx,logo_exec
        mov     dx,logo_com
        int     21h


cont:
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        ; Execute LOGO.COM. If failed, nothing will be outputted.
        ;컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
        mov     ax,4B00h
        mov     dx,logo_com
        int     21h

logo_com: DB "OS90\LOGO.COM",0

logo_exec:
        DW      0
        DD      0       ; There is no command line
        DD      0,0,0,0
messages:
.no_386:                  DB "OS/90 requires an i386 or better.",10,13,36
.protected_mode_software: DB "EMM386 or protected mode OS in use.",10,13,36
.xms_not_present:         DB "XMS driver not present.",10,13,36
.hma_not_available:       DB "HMA not available", 10,13,36

        segment KERNEL
        FILE KERNEL_EXEC

