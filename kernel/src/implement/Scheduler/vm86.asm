;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                           ;;
;;                     Copyright (C) 2023, Joey Qytyku                       ;;
;;                                                                           ;;
;; This file is part of OS/90 and is published under the GNU General Public  ;;
;; License version 2. A copy of this license should be included with the     ;;
;; source code and can be found at <https://www.gnu.org/licenses/>.          ;;
;;                                                                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        %include "Macro.inc"

        global Enter_Real_Mode

;===============================================================================
;                               I m p o r t s
;===============================================================================

        extern  SetESP0, GetESP0    ; Argument goes in EAX
        extern  preempt_count

;===============================================================================
;                               E x p o r t s
;===============================================================================

        global  _RealModeRegs, _RealModeTrapFrame, EnterRealMode

;===============================================================================
                                section .text
;===============================================================================

;-------------------------------------------------------------------------------
; Procedure:
;       - Set the stack pointer to what it would be after calling this function
;         if it was a normal one with a RET code.
;       - Load parameters into register file
;       - Generate IRET V86 stack frame and IRET
;       - Only Exit_Real_Mode can leave
;
; Precoditions:
;       - Preemption MUST be off
;       - Trap frame parameters should be valid (EFLAGS.VM must ==1)
;
; Save EBP?
Enter_Real_Mode:
        lea     eax,[esp+4]     ; ?
        call    SetESP0

        mov     [RealModeCalleeSaved.lEbp],ebp
        mov     ebx,[esp]
        mov     [RealModeCalleeSaved.lEip],ebx

        mov     ebx,_RealModeRegs
        mov     eax,[ebx]
        mov     ecx,[ebx+8]
        mov     edx,[ebx+12]
        mov     esi,[ebx+16]
        mov     edi,[ebx+20]
        mov     ebp,[ebx+24]
        mov     ebx,[ebx+28]

        ; May not be the best instructions
        ; Use EBX

        push    U32 [_RealModeTrapFrame]        ; GS
        push    U32 [_RealModeTrapFrame+4]      ; FS
        push    U32 [_RealModeTrapFrame+8]      ; DS
        push    U32 [_RealModeTrapFrame+12]     ; ES
        push    U32 [_RealModeTrapFrame+16]     ; SS
        push    U32 [_RealModeTrapFrame+20]     ; ESP
        push    U32 [_RealModeTrapFrame+24]     ; EFLAGS
        push    U32 [_RealModeTrapFrame+28]     ; CS
        push    U32 [_RealModeTrapFrame+32]     ; EIP

        iret

; This procedure should never be called anywhere outside an exception handler
; which is non-preemptible context. Because it is non-preemptible, it is safe.

;-------------------------------------------------------------------------------
; This is not a procedure, but a jump location that leads to system exit.
; Before that, it will set ESP, EBP, and EIP on the SEP RD to go back to the
; caller.
;
Exit_Real_Mode:
        ; Make it a jump location?
        ; TODO

;===============================================================================
                                section .data
;===============================================================================

;===============================================================================
                                section .bss
;===============================================================================


; TODO: Make into IRET frame. We do not need to use all the fields.

; The order here is expedient to EnterRealMode and does not have anything
; to do with the trap frame
_RealModeRegs:
        RESD    7
_RealModeTrapFrame:
        RESD    9

; When we return to the called of EnterRealMode, this buffer is loaded.
RealModeCalleeSaved:
.lEsp:  RESD    1
.lEbp:  RESD    1
.lEip:  RESD    1
.lEflags:  RESD    1
