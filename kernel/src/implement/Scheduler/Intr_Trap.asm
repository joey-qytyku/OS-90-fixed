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

;-------------------------------------------------------------------------------
;                       M a c r o   D e f i n i t i o n s

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

;                  E n d   M a c r o   D e f i n i t i o n s
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;                               I m p o r t s

EXTERN  System_Entry_Point

;                           E n d   I m p o r t s
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;                               E x p o r t s

GLOBAL  Low15, Low7, LowRest
GLOBAL  _dwErrorCode
GLOBAL  LowSwi

; Unwrap
%assign i 0
%rep 17
        GLOBAL  LowE %+ i
        %assign i i+1
%endrep

;                          E n d   o f   E x p o r t s
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
                                 section .text
;-------------------------------------------------------------------------------

        align   32
LowRest:
Low15:
Low7:

;-------------------------------------------------------------------------------
                                 section .bss
;-------------------------------------------------------------------------------

        global _ErrorCode, _ExceptIndex
dwEIP           RESD    1

_dwErrorCode    RESD    1

; DO NOT MAKE THIS GLOBAL
dwActualIRQ     RESD    1

;-------------------------------------------------------------------------------

;********************************\Fun Fact/********************************
; Did you know that the TEST instruction sets the parity flag to 0 if the
; number of one bits in the number is even and 1 if odd?
