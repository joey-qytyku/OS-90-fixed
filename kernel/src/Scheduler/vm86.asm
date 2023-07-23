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
        inc     dword [preempt_count]

        mov     [RealModeCalleeSaved.lEsi],esi
        mov     [RealModeCalleeSaved.lEdi],edi
        mov     [RealModeCalleeSaved.lEbp],ebp

        mov     ebx,_RealModeRegs
        mov     eax,[ebx]
        mov     ecx,[ebx+8]
        mov     edx,[ebx+12]
        mov     esi,[ebx+16]
        mov     edi,[ebx+20]
        mov     ebp,[ebx+24]
        mov     ebx,[ebx+28]

        push    dword [_RealModeTrapFrame]        ; GS
        push    dword [_RealModeTrapFrame+4]      ; FS
        push    dword [_RealModeTrapFrame+8]      ; DS
        push    dword [_RealModeTrapFrame+12]     ; ES
        push    dword [_RealModeTrapFrame+16]     ; SS
        push    dword [_RealModeTrapFrame+20]     ; ESP
        push    dword [_RealModeTrapFrame+24]     ; EFLAGS
        push    dword [_RealModeTrapFrame+28]     ; CS
        push    dword [_RealModeTrapFrame+32]     ; EIP

        xchg    bx,bx
        iret

        section .data
        section .bss

_RealModeRegs:
        RESD    7
_RealModeTrapFrame:
        RESD    9

RealModeCalleeSaved:
.lEsp:  RESD    1
.lEbp:  RESD    1
.lEsi:  RESD    1
.lEdi:  RESD    1
.lEip:  RESD    1
.lEflags:  RESD    1
