;-------------------------------------------------------------------------------
; Description
;
; This module contains cdecl functions for string operations, written in
; assembly for maximum speed
;
;
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;                               E x p o r t s


global StrCpy, StrLen, StrLower, StrUpper

;                        E n d   o f   E x p o r t s
;-------------------------------------------------------------------------------


        section .text

;-------------------------------------------------------------------------------
; PARAM:        CDECL(str) -> Length of string excluding NUL terminator
; WARNINGS:     Will not handle strings longer than 65535 including NUL.
;
StrLen:
        cld
        xor     eax,eax
        mov     ecx,65535
        mov     esi,[esp+4]
        repnz    scasb

        not     ecx
        lea     eax,[ecx-1]
        ret

; C ARGUMENTS: dest, src
;
StrCpy:
        mov     edi,[esp+4]
        mov     esi,[esp+8]
        cld
.l:
        lodsb
        stosb
        cmp     al,0
        jnz     .l
.end    ret

;-------------------------------------------------------------------------------
; PARAM: CDECL(str) -> VOID
;
; These work as expected.
;
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
