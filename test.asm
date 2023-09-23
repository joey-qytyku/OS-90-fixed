section .text

global _start

StrUpper:
        mov     ebx,0E0h        ; -32 as signed byte
        jmp     Skip
StrLower:
        mov     ebx,32
Skip:

        mov     esi,[esp+4]
.l:
        lodsb
        test    al,al
        jz      .end

        sub     al,'a'
        cmp     al,'z'-'a'
        ja      .l

        add     byte [esi-1],bl
        jmp     .l
.end    ret

_start:
        push    msg
        call    StrUpper

        mov     eax,4
        mov     ebx,1
        mov     ecx,msg
        mov     edx,len
        int     80h

        mov     ebx,eax
        mov     eax,1
        int     0x80

section .data
    msg db  "rip bozo",0
    len equ $ - msg - 1

segment .bss
