;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                           ;;
;;                     Copyright (C) 2023, Joey Qytyku                       ;;
;;                                                                           ;;
;; This file is part of OS/90 and is published under the GNU General Public  ;;
;; License version 2. A copy of this license should be included with the     ;;
;; source code and can be found at <https://www.gnu.org/licenses/>.          ;;
;;                                                                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "Scheduler/SchedDefs.inc"

;===============================================================================
;                       M a c r o   D e f i n i t i o n s
;===============================================================================

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
%endmacro

%macro RestTrapRegs 0
        pop     eax
        pop     ebx
        pop     ecx
        pop     edx
        pop     esi
        pop     edi
        pop     ebp
%endmacro

;===============================================================================
;                  E n d   M a c r o   D e f i n i t i o n s
;===============================================================================

;===============================================================================
;                               I m p o r t s
;===============================================================================

extern System_Entry_Point

;===============================================================================
;                           E n d   I m p o r t s
;===============================================================================

;===============================================================================
;                               E x p o r t s
;===============================================================================

; Remove the repeated globals

global  Low15, Low7, LowRest
global  _dwErrorCode
global  LowSwi
global _ErrorCode, _ExceptIndex

global Interrupt_Descriptor_Table,System_Exit_Point

;===============================================================================
;                          E n d   o f   E x p o r t s
;===============================================================================

;===============================================================================
                                 section .text
;===============================================================================

        align   64
LowRest:
Low15:
        pop     dword [ss:lActualIRQ]
Low7:

        ; IF=0 on entry
        align   64
Low_System_Entry:
        SaveTrapRegs
        push    gs
        push    fs
        push    ds
        push    es

        mov     eax,ss
        mov     ds,ax

        jmp     System_Entry_Point

        align   16
System_Exit_Point:
        cli

        pop     es
        pop     ds
        pop     fs
        pop     gs
        RestTrapRegs
        iret

;===============================================================================
                                 section .data
;===============================================================================

; The IDT by default is in a form in which the linker can handle where addresses
; are 32-bit and not scrambled. The format is like this in order of low to high
; memory.
;
; Note, you can use stack segment for data access.
;
; Instead of wasting memory to copy the table, we simply rearrange the existing
; one. Every IDT entry goes to the low system entry handler, including
; exceptions. IA32.asm does all this.
;
; +0 [OFF 15..0] [OFF 31..16]
; +4 [Whatever ] [ SELECTOR ]
;
; Operation: xhcg +0 with +6
;
        align   64
Interrupt_Descriptor_Table:

%assign i 0
%rep 256

Low_Handler_ %+ i :
        DD      Low_System_Entry
        DB      0, 0EFh
        DW      8
%assign i i+1

%endrep

;-------------------------------------------------------------------------------
; The low handlers for exceptions.
; These will pop the error codes off.
;-------------------------------------------------------------------------------

%macro MK_Error_Code_Except 0
%endmacro

%macro MK_Except 0
%endmacro

%assign i 0
;        %if i == 0 || i ==

        ; %else
        ; %endif
%rep 256


;%if i ==

%assign i i+1
%endrep

;===============================================================================
                                 section .bss
;===============================================================================

_dwErrorCode    RESD    1

; DO NOT MAKE THIS GLOBAL
lActualIRQ     RESD    1
