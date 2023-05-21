;===============================================================================
;    This file is part of OS/90.
;
;   OS/90 is free software: you can redistribute it and/or modify it under the
;   terms of the GNU General Public License as published by the Free Software
;   Foundation, either version 2 of the License, or (at your option) any later
;   version.
;
;   OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY
;   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
;   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
;   details.
;
;   You should have received a copy of the GNU General Public License along
;   with OS/90. If not, see <https://www.gnu.org/licenses/>.
;===============================================================================

        %include "include/Kernel.inc"

        global  EnterV86_16
        global  ContinueCallerV86
        section .text

;--------------------------------------------------------------
; Brief: Enter V86
;
; TODO: Serious refactoring underway
;
; This is a very complicated function (along with ShootdownV86). It is callable
; from C and enters V86. It is not called by the scheduler to run regular
; software and is only for calling DOS software. Because of this, any call to
; this procedure HAS to be coupled with a continue call in an exception.
;
; The output registers are copied by the GP handler from the trap frame to the
; context provided to EnterV86.
;
; ESP points to the last pushed element. Pushing first decrements by 4
; before writing the value.

        extern  SetESP0, GetESP0    ; Argument goes in EAX

EnterV86_16:
        ; We cannot use a critical section here.

        lea     ebx,[esp+4]

        ;Save cdecl callee saved registers to stack

        iret

;-------------------------------------------------------------------------------
;     Continue caller of ScEnterV86
;
; The only sane place to use OnExceptRetReenterCallerV86
; is an exception handler, like #GP. It does not have to be at level one.
; It could be at at any call level within an exception handler.
;
; This will not immediately switch to the context. The exception must exit
; normally. This will only change the context to get back to the caller
;
global OnExceptRetReenterCallerV86
OnExceptRetReenterCallerV86:
        ;I do not know where the stack is right now, but I will get the address
        ;saved to the TSS. That will allow me to pull the necessary values
        call    GetESP0
        ;This address will allow us to copy the necessary data
        mov     esi,[eax-4]
        ;Flags?
        ;Copy EIP from the caller to interrupt frame
        ;We will assume the frame is valid for V86 (i.e has segment registers)

        ;We can use the trap frame struct by subtracting the size with
        ;the offset, The resulting number is the negative offset
        ;Example, EAX at offset zero, so the relative address is -68

        mov     ecx,[eax+0]                       ;Get EBP value
        mov     [eax-4+(TF._ebp-TF._size)],ecx    ;Write it to the trap frame

        mov     ecx,[eax+4]                       ;ESI value
        mov     [eax-4+(TF._esi-TF._size)],ecx

        mov     ecx,[eax+8]                       ;EDI value
        mov     [eax-4+(TF._esi-TF._size)],ecx

        mov     ecx,[eax+12]                      ;EBX value
        mov     [eax-4+(TF._esi-TF._size)],ecx

        ;FLAGS? The compiler may be using the flags. Also, the interrupt
        ;have to be restored to their previous state.

        ret

        section .data

        section .bss
