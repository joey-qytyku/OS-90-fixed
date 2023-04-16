%if 0
     This file is part of OS/90.

    OS/90 is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 2 of the License, or (at your option) any later
    version.

    OS/90 is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with OS/90. If not, see <https://www.gnu.org/licenses/>.
%endif


;===============================================================================
; MACRO DEFS

%define NOCODE  0
%define ERRCODE 1

;Requires the error code to be
;pushed off stack for exceptions that use it
%macro SaveTrapRegs 0
        push    ebp
        push    esp ; Meaningless
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
        add     esp,4
        pop     ebp
%endm

; END MACRO DEFS
;===============================================================================

;===============================================================================
; IMPORTS
        EXTERN  last_mode
        EXTERN  InMasterDispatch
        EXTERN  ScExceptionDispatch

; END IMPORTS
;===============================================================================

;===============================================================================
; EXPORTS
        GLOBAL  Low15, Low7, LowRest
        GLOBAL  _dwErrorCode
; END EXPORTS
;===============================================================================

[section .text]

;------------------------------------------------------------------
;In case of a spurious interrupt, the ISR will be zero in the PIC
;chip that caused it, so the computer cannot differentiate between
;interrupts unless different ISRs are used
;
;We only need to know the actual IRQ of #15 and #7 because those vectors
;are the only ones capable of spurious interrupts. The second parameter
;passed to the master dispatcher will be zero if any other IRQ or 15/7.
;Otherwise, it is not important, we get it from the PICs ISR.
;------------------------------------------------------------------
;Each handler is aligned to four bytes.

        align   4
Low7    push byte 7
        jmp short ContinueIRQ
Low15   push byte 15
        jmp short ContinueIRQ
LowRest:
        push byte 0

ContinueIRQ:
    pop     dword [dwActualIRQ]
    SaveTrapRegs
    lea     eax,[esp+48]
    mov     edx,[dwActualIRQ]
    call    InMasterDispatch

    RestTrapRegs
    iret

global LowDivide0
LowDivide0:
    push    byte 0
    jmp     short ContinueExcept

global LowDebug
LowDebug:
    push    byte 1
    jmp     short ContinueExcept

global LowNMI
LowNMI:
    push    byte 2
    jmp     short ContinueExcept

global LowBreakpoint
LowBreakpoint:
    push    byte 3
    jmp     short ContinueExcept

global LowOverflow
LowOverflow:
    push    byte 4
    jmp     short ContinueExcept

global LowBoundRangeExceeded
LowBoundRangeExceeded:
    pop     dword [ss:_dwErrorCode]
    push    byte 5
    jmp     short ContinueExcept

global LowInvalidOp
LowInvalidOp:
    push    byte 6
    jmp     short ContinueExcept

global LowDevNotAvail
LowDevNotAvail:
    push    byte 7
    jmp     short ContinueExcept

global LowDoubleFault
LowDoubleFault:
    pop     dword [ss:_dwErrorCode]
    push    byte 8
    jmp     short ContinueExcept

global LowSegOverrun
LowSegOverrun:
    push    byte 9
    jmp     short ContinueExcept

global LowInvalidTSS
LowInvalidTSS:
    pop     dword [ss:_dwErrorCode]
    push    byte 10
    jmp     short ContinueExcept

global LowSegNotPresent
LowSegNotPresent:
    pop     dword [ss:_dwErrorCode]
    push    byte 11
    jmp     short ContinueExcept

global LowStackSegFault
LowStackSegFault:
    pop     dword [ss:_dwErrorCode]
    push    byte 12
    jmp     short ContinueExcept

global LowGeneralProtect
LowGeneralProtect:
    pop     dword [ss:_dwErrorCode]
    push    byte 13
    jmp     short ContinueExcept

global LowPageFault
LowPageFault:
    pop     dword [ss:_dwErrorCode]
    push    byte 14
    jmp     short ContinueExcept

global LowFloatError
LowFloatError:
    push    byte 16
    jmp     short ContinueExcept
global LowAlignCheck
LowAlignCheck:

;-------------------------------------------------------------------------------
; Segment registers are destroyed after exiting virtual 8086 mode. It is the
; responsibility of the OS to restore them. This is done simply by copy SS
; into the data segment registers.
;
; When a 32-bit context is interrupted, the stack will contain the value of
; the stack segment. The rest of the segment registers are still in context
; and must be saved. They will simply be saved to the stack by pushing.
; The RD defines have macros with negative offsets to refer to this, but the
; only useful purpose for this would be the scheduler interrupt.
;
; At the end of the ISR, the segment registers of the ring-3 and 32-bit
; context will be restored along with all the other registers. This only
; happens if the context was 32-bit ring-3.
;
; If the kernel was interrupted, there is no need to modify the segment
; registers at all. We can just save and restore the registers, and IRET.
; The last mode variable tells us the ring of the previous context.
;
; Now we put this all together. The following is the procedure.
; Note that the conditions can be pre-computed into variables.
;-------------------------------------------------------------------------------
; Save general purpose registers on the stack
;
; If previous context V86 Then:
;   WasV86 <- 1
;
; If !WasV86:
;   If last mode switched is KERNEL:
;       NOTHING
;
;   Else if last mode switched is USER and VM = 0:
;       SAVE SREGS
;
; Write register parameters
; Call high level C handler
;
; If last mode switched is KERNEL:
;   RESTORE REGS
;   IRET
; If WasV86:
;   RESTORE REGS
;   IRET
;
; If previous context was VM=0 and USER:
;   RESTORE SREGS
;   RESTORE REGS
;   IRET
;-------------------------------------------------------------------------------
; New idea for exceptions. Just index a table of handlers in C like with IRQ.
; Much cleaner that way and it saves some code space.
; According to my calculations, defining each individually would take up
; about 510 bytes of code while the other way takes only 136 bytes.
;-------------------------------------------------------------------------------
ContinueExcept:
        ;Serialization is needed for accessing the last mode
        cli

        ;Only the stack segment is garaunteed to be valid right now
        ;The selector is the same as the data segment, so use an override
        pop     dword[ss:_dwExceptIndex]
        test    dword[ss:esp+8],1<<17
        setnz   [ss:_bWasV86]
        SaveTrapRegs
        jz      .NotV86

        ;If it was V86, the sregs are already saved on the stack now
        jmp     .SetSregs

.NotV86:
        ;If the interrupt happened while kernel was running, the segments
        ;are exactly what I want them to be
        ;If it happened during userspace, they could be anything
        ;and I cannot clobber them

        ;Was the context kernel?
        cmp     byte[ss:last_mode],0
        setne   [ss:_bSregs32]
        ;If not, we have to save the segment registers and set selectors
        je      .SaveAndSetSregs

        ;If so, the SREGS are what they should be
        ;We go straight to the handler
        mov     byte[ss:_bSregs32],0
        jmp     .HandleException

        ;We can either save the sregs and set the kernel selectors
        ;set them without saving, or just handle directly
.SaveAndSetSregs:
        mov     [old_es],es
        mov     [old_ds],ds
        mov     [old_fs],fs
        mov     [old_gs],gs

.SetSregs:
        mov     ax,ss
        mov     ds,ax
        mov     es,ax
        mov     fs,ax
        mov     gs,ax

.HandleException:
        mov     eax,esp
        call    ScExceptionDispatch
.End:
        ;Did we save the segment registers?
        ;If so, restore them now
        cmp     byte[_bSregs32],1
        jne     .NotSaved
        mov     es,[old_es]
        mov     ds,[old_ds]
        mov     fs,[old_fs]
        mov     gs,[old_gs]

.NotSaved:
        ;Interrupts will go back to their previous state
        iret

;------------------------------------;
; Other IA32 exceptions are not used ;
;------------------------------------;

[section .data]

[section .bss]
global _ErrorCode, _ExceptIndex, _bWasV86

_dwErrorCode    RESD    1
_dwExceptIndex  RESD    1
_bWasV86        RESB    1
_bSregs32       RESB    1   ; LOCAL

old_es:         RESW    1
old_ds:         RESW    1
old_fs:         RESW    1
old_gs:         RESW    1

; DO NOT MAKE THIS GLOBAL
dwActualIRQ:
    RESD    1
