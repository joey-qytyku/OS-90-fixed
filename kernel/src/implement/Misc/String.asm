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


global StrCpy, StrLen, StrLower, StrUpper, Hex32ToString

;                        E n d   o f   E x p o r t s
;-------------------------------------------------------------------------------


        section .text

MAX_STR_LENGTH_OF_UINT32 equ 10

;-------------------------------------------------------------------------------
; PARAM: CDECL(value, obuffer) -> VOID
; TESTED WORKING
;
; I will not be counting leading zeroes so that hex output is aligned.
;
; The buffer must at least 9 bytes long and will auto null terminate.
; Takes a number in AL and converts it to a hex nibble character.
; BCD instructions may not be fast but there are no memory access or branch
; penalties.
;
; I found the trick here:
; quora.com/What-are-the-most-obscure-useless-x86-assembly-instructions?
;
Hex32ToString:
        mov     edi,[esp+8]
        mov     ebx,[esp+4]
        add     edi,7
        mov     ecx,8
        std
.L:
        mov     eax,ebx
        and     eax,0Fh

        cmp     al,10
        sbb     al,69h
        das
        stosb

        shr     ebx,4

        dec     ecx
        jnz     .L

        mov     al,0
        stosb

        cld
        ret

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
