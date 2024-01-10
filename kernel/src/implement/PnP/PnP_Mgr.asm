;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                           ;;
;;                     Copyright (C) 2023, Joey Qytyku                       ;;
;;                                                                           ;;
;; This file is part of OS/90 and is published under the GNU General Public  ;;
;; License version 2. A copy of this license should be included with the     ;;
;; source code and can be found at <https://www.gnu.org/licenses/>.          ;;
;;                                                                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;===============================================================================
; The following code allows for calling the protected mode Plug-and-Play BIOS
; This is because the real mode interface may not be complete. QEMU for example
; does not support any functions in real mode but seems to work fine in PM.
;===============================================================================

global PnInsertEntryPoint
global PnCallBiosInternal

;PnP does not require the DS register to be modified
;Most functions, however, will require the BIOS selector
;to be pushed for every call, which is more convenient

[section .bss]

;===============================================================================
; The location at which the PnP kernel calls the BIOS
; 16-bit far pointers are used with the 66h size override prefix so that the
; jump instruction uses a 32-bit seg:off FP rather than a 48-bit FP.
;
wfpEntryPoint:
    RESD    1

[section .text]

;The selector being predefined in IA32.h but cannot be included here
;The offset is TBD, but it is 16-bit

PnInsertEntryPoint:
    mov     eax,[esp+4]
    mov     [wfpEntryPoint],eax
    ret
;===============================================================================
; Variadic functions are called by simply pushing
; the desired number of arguments on the (always) stack
;
; BIOS plug-and-play BIOS calls use 16-bit far calls
; and are required to handle both 32-bit and 16-bit
; stack segments.
;-------------------------------------------------------------------------------
; PnCallBiosInternal  |  Uses regparam(1)
; args: dword argc [EAX], dword func, ...
;
; Requires that the ROM space (shadow and actual) are identity mapped
PnCallBiosInternal:
    push    esi
    push    eax

    ;In memory, arguments are in sequential order
    ;The following will copy the low word of each
    ;32-bit argument AFTER the first one back by one word
    ;to narrow them to 16-bit words for PnP 16-bit PM interface
    ; |A32|A32|A32|EIP|
    ; |---|-|A|A|A|EIP|

    ;Because stack addressing is relative, extra
    ;bytes will not cause any problems

    mov     ecx,eax
    lea     esi,[esp+4] ;First argument address
    std
.L:
    jecxz   .Done
    add     esi,4       ;Goes to second arg on 1st iteration
    mov     eax,[esi]
    mov     [esi+2],ax
    jmp     .L
.Done:
    call    far word [wfpEntryPoint]

    ;PnP returns exit status in AX
    ;cdecl requires narrow returns to be sign/zero extended
    movzx   eax,ax

    cld
    pop     eax
    pop     esi
    ret
