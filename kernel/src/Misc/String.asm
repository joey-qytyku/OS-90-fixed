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


global StrCpy, StrLen, StrLower, StrUpper, Uint32ToString

;                        E n d   o f   E x p o r t s
;-------------------------------------------------------------------------------


        section .text

MAX_STR_LENGTH_OF_UINT32 equ 10

;-------------------------------------------------------------------------------
; PARAM: CDECL (valueToConvert, buffer) -> Start of characters to print pointer
;
; Recommend having a null terminator for the buffer so that the resulting
; pointer can be used to print the string iteratively.
;
; CLOBBERS: All
;
Uint32ToString:
        push    ebp
        mov     ebp,esp
        cld
        ; Its 8 and 12
        ; not 4 and 8

        ; Clear the buffer and fill with NUL character
        xor     eax,eax
        mov     edi,[ebp+12]                      ; obuffer
        mov     ecx,MAX_STR_LENGTH_OF_UINT32
        rep     stosb

        mov     edi,[ebp+12]                      ; obuffer
        mov     esi,MAX_STR_LENGTH_OF_UINT32 - 2 ; buff_off
        mov     ebx,1                            ; digit_divisor
        xor     ecx,ecx                          ; i
        mov     ebp,[ebp+8]                      ; value
.L:
        cmp     ecx,MAX_STR_LENGTH_OF_UINT32-1
        jz      .break

        xor     edx,edx

        mov     eax,ebp
        div     ebx

        xor     edx,edx

        push    ebx

        mov     ebx,10
        div     ebx

        pop     ebx

        ; write to obuffer[buff_off]
        add     edx,'0'
        mov     byte [edi+esi],dl

        inc     ecx
        dec     esi
        imul    ebx,10

        jmp     .L

.break:
        ; Count leading zeroes. Simple string operation.
        xor     eax,eax
        mov     al,'0'
        mov     ecx,10

        ;scasb uses EDI apparently. It is already set.

        repe   scasb           ; Scan until != 0

        ; EDI is now outside the buffer or pointing +1 to where it should be
        lea     eax,[edi-1]

        pop     ebp
        ret

;-------------------------------------------------------------------------------
; PARAM: CDECL(value, obuffer) -> VOID
; TESTED WORKING
; TODO: Count leading zeroes and return address to skip them.
;
; The buffer must at least 8 bytes long and can include a null terminator.
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
        mov     edi,[esp+4]
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
