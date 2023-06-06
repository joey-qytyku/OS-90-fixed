;===============================================================================
;                         This file is part of OS/90.
;
;    OS/90 is free software: you can redistribute it and/or modify it under the
;    terms of the GNU General Public License as published by the Free Software
;    Foundation, either version 2 of the License, or (at your option) any later
;    version.
;
;    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY
;    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
;    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
;    details.
;
;    You should have received a copy of the GNU General Public License along
;    with OS/90. If not, see <https://www.gnu.org/licenses/>.
;===============================================================================

%include "Asm/Kernel.inc"

;===============================================================================
; MACRO DEFS

%define NOCODE  0
%define ERRCODE 1

;Requires the error code to be
;pushed off stack for exceptions that use it

%macro SaveTrapRegs 0
        push    ebp
        push    edi
        push    esi
        push    edx
        push    ecx
        push    ebx
        push    eax
%endm

%macro RestTrapRegs 0
        pop     eax
        pop     ebx
        pop     ecx
        pop     edx
        pop     esi
        pop     edi
        pop     ebp
%endm

; END MACRO DEFS
;===============================================================================

;===============================================================================
; IMPORTS
        EXTERN  ExceptionDispatch
        EXTERN  InterruptDispatch
        EXTERN  SystemEntryPoint
; END IMPORTS
;===============================================================================

;===============================================================================
; EXPORTS
        GLOBAL  Low15, Low7, LowRest
        GLOBAL  _dwErrorCode
        GLOBAL  LowSystemEntryPoint

%assign i 0
%rep 17
        GLOBAL  LowE %+ i
        %assign i i+1
%endrep


; END EXPORTS
;===============================================================================

[section .text]

    align   16

LowRest:
Low15:
Low7:

LowE0:  DB      66h,0E8h, 72, 0
LowE1:  DB      66h,0E8h, 68, 0
LowE2:  DB      66h,0E8h, 64, 0
LowE3:  DB      66h,0E8h, 60, 0
LowE4:  DB      66h,0E8h, 56, 0
LowE5:  DB      66h,0E8h, 52, 0
LowE6:  DB      66h,0E8h, 48, 0
LowE7:  DB      66h,0E8h, 44, 0
LowE8:  DB      66h,0E8h, 40, 0
LowE9:  DB      66h,0E8h, 36, 0
LowE10: DB      66h,0E8h, 32, 0
LowE11: DB      66h,0E8h, 28, 0
LowE12: DB      66h,0E8h, 24, 0
LowE13: DB      66h,0E8h, 20, 0
LowE14: DB      66h,0E8h, 16, 0
LowE15: DB      66h,0E8h, 12, 0
LowE16: DB      66h,0E8h, 8, 0
LowE17: DB      66h,0E8h, 4, 0
LowSwi: DB      66h,0E8h, 0, 0

        pop     dword [ss: .smc]

        push    gs
        push    fs
        push    es
        push    ds
        SaveTrapRegs

        DB      0B8h    ; MOV EAX,
.smc:   DD      0       ;         Imm32

        mov     ebx,LowE0
        sub     eax,ebx
        shr     eax,2
        mov     ebx,esp
        call    SystemEntryPoint

        ; To IRET or not to IRET?
        RestTrapRegs

        section .bss

        global _ErrorCode, _ExceptIndex

_dwErrorCode    RESD    1
_dwExceptIndex  RESD    1

; DO NOT MAKE THIS GLOBAL
dwActualIRQ     RESD    1

;********************************\Fun Fact/********************************
; Did you know that the TEST instruction sets the parity flag to 0 if the
; number of one bits in the number is even and 1 if odd?
