;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                           ;;
;;                     Copyright (C) 2023, Joey Qytyku                       ;;
;;                                                                           ;;
;; This file is part of OS/90 and is published under the GNU General Public  ;;
;; License version 2. A copy of this license should be included with the     ;;
;; source code and can be found at <https://www.gnu.org/licenses/>.          ;;
;;                                                                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "Asm/Kernel.inc"

;===============================================================================
; MACRO DEFS

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
[section .text]

        EXTERN  ExceptionDispatch
        EXTERN  InterruptDispatch
        EXTERN  SystemEntryPoint
; END IMPORTS
;===============================================================================

;===============================================================================
; EXPORTS
        GLOBAL  Low15, Low7, LowRest
        GLOBAL  _dwErrorCode
        GLOBAL  LowSwi

%assign i 0
%rep 17
        GLOBAL  LowE %+ i
        %assign i i+1
%endrep


; END EXPORTS
;===============================================================================

    align   16

LowRest:
Low15:
Low7:

;
; This is a series of call instuctions with 16-bit operands.
; The EIP saved onto the stack is copied.
;

; Note there will be an empty space in the stack upon entry
        align   16
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
LowSwi: DB      66h,0E8h, 0, 0  ; Switch to NOP?

        pop     dword [ss: dwEIP]

        ; Did we come from virtual 8086 mode? If so, there is no need to save
        ; the sregs

        ; Contradiction here?
        ; We should ALWAYS save the segment registers
        ; Also, the error code needs to be pushed off

        test    dword [ss:esp+8],1<<17
        jz      .WasV86

        ; push the segment registers because they could be anything right now.
        push    gs
        push    fs
        push    es
        push    ds

.WasV86:
        SaveTrapRegs

        ; (Call table saved EIP - Base of call Table) / sizeof(CtabEntry)
        mov     eax,[dwEIP]
        mov     ebx,LowE0
        sub     eax,ebx
        shr     eax,2

        mov     ebx,esp         ; Trap frame pointer
        call    SystemEntryPoint

        ; The kernel may have switched the VM flag in EFLAGS. We have to
        ; honor the VM flag as it currently is and check it AGAIN.

        RestTrapRegs
        ; If we were in protected mode, the sregs are restored normally.
        ; If it was V86, only IRET should pop the segment registers because
        ; they are real mode segments.

        test    dword [esp+8],1<<17   ; No need for override

        push    ds
        push    es
        push    fs
        push    gs

        iret

        ; Note that the IRET may not actually be reached. If this is an
        ; entry to the kernel from a process, the kernel thread will simply
        ; hang until completion.

        section .bss

        global _ErrorCode, _ExceptIndex
dwEIP           RESD    1

_dwErrorCode    RESD    1

; DO NOT MAKE THIS GLOBAL
dwActualIRQ     RESD    1

;********************************\Fun Fact/********************************
; Did you know that the TEST instruction sets the parity flag to 0 if the
; number of one bits in the number is even and 1 if odd?
