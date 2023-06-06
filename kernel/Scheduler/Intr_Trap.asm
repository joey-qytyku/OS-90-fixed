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
;
;-------------------------------------------------------------------------------
; Another algorithm that does not avoid segment register loads on ring switches
; Pseudocode
; {
; SAVE REGS
; SAVE SREGS            -- Even if they are zero, not including stack of course
; SET KERNEL DSEGS
;
; EAX <- ESP + 16
; CALL HANDLER
;
; RESTORE SREGS FROM STACK
; IRET
; -- This does actually work, even if they are zeroed
; -- If we are returning to V86, the correct values
; -- are automatically loaded upon IRET
; -- If we are returning to protected mode, the sregs
; -- are popped off the stack.
; }
;
; The standard register dump structure will have protected mode and real mode
; segment registers. (DO not use the negative offset?)
; The PM sregs are pushed to the stack even if they are invalid.
;
;IA32 will always flush segment caches upon loads, which is slow, but this
;algorithm is much simpler and has less branching. In fact, memory is accessed to
;execute the complex avoidance code, so it may be a lesser of two evils.
;-------------------------------------------------------------------------------

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

;-------------------------------------------------------------------------------
; This code section handles exceptions and IRQs. It then passes control to a high-level
; dispatcher. The ISRs themselves are different in order to tell them apart.
; Each entry point must be registered with the IDT as IRQ so that IF=0 on entry.
; We will enable interrupts later on for exceptions.

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

        RestTrapRegs
        iret

;===============================================================================
; Entered with interrupts disabled by IDT. This will only be caused if the INT
; instruction was called. This is the only way for a process to enter kernel
; mode with preemption enabled. This will enter the kernel mode thread using
; the process-local stack which contains the register save structure.
; These vectors should never be called by ring-0 software or a fatal error
; will occur.
;

        align   16
LowSystemEntryPoint:
        ; Save user registers to the register dump
        push    ebx

        mov     ebx,esp
        and     ebx,~1FFFh

        section .bss

        global _ErrorCode, _ExceptIndex

_dwErrorCode    RESD    1
_dwExceptIndex  RESD    1

; DO NOT MAKE THIS GLOBAL
dwActualIRQ     RESD    1

;********************************\Fun Fact/********************************
; Did you know that the TEST instruction sets the parity flag to 0 if the
; number of one bits in the number is even and 1 if odd?
