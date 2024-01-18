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

        global EnterRealMode
        section .text

        extern  SetESP0, GetESP0    ; Argument goes in EAX
        extern  preempt_count

        global  _RealModeRegs, _RealModeTrapFrame, EnterRealMode

;
; This function takes no arguments. It will increment the preempt count
; to prevent preemption to a kernel thread while it reads from the
; register buffer.
;

EnterRealMode:
        lea     eax,[esp+4]
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

        ; Something wrong with IDT?

        ; May not be the best instructions
        push    dword [_RealModeTrapFrame]        ; GS
        push    dword [_RealModeTrapFrame+4]      ; FS
        push    dword [_RealModeTrapFrame+8]      ; DS
        push    dword [_RealModeTrapFrame+12]     ; ES
        push    dword [_RealModeTrapFrame+16]     ; SS
        push    dword [_RealModeTrapFrame+20]     ; ESP
        push    dword [_RealModeTrapFrame+24]     ; EFLAGS
        push    dword [_RealModeTrapFrame+28]     ; CS
        push    dword [_RealModeTrapFrame+32]     ; EIP

        iret

        section .data
        section .bss

; This procedure should never be called anywhere outside an exception handler
; which is non-preemptible context. Because it is non-preemptible, it is safe
ExitRealMode:

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
