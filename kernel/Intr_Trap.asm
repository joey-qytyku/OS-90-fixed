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

; remove bogus esp, will this break something
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
        EXTERN  last_mode
        EXTERN  ExceptionDispatch
        EXTERN  InterruptDispatch
; END IMPORTS
;===============================================================================

;===============================================================================
; EXPORTS
        GLOBAL  Low15, Low7, LowRest
        GLOBAL  _dwErrorCode
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

    align   4
LowE0:
    push    byte 0C0h
    jmp     ContAsyncISR
LowE1:
    push    byte 0C1h
    jmp     ContAsyncISR
LowE2:
    push    byte 0C2h
    jmp     ContAsyncISR
LowE3:
    push    byte 0C3h
    jmp     ContAsyncISR
LowE4:
    push    byte 0C4h
    jmp     ContAsyncISR
LowE5:
    push    byte 0C5h
    jmp     ContAsyncISR
LowE6:
    push    byte 0C6h
    jmp     ContAsyncISR
LowE7:
    push    byte 0C7h
    jmp     ContAsyncISR
LowE8:
    pop     dword [ss:_dwErrorCode]
    push    byte 0C8h
    jmp     ContAsyncISR
LowE9:
    push    byte 0C9h
    jmp     ContAsyncISR
LowE10:
    pop     dword [ss:_dwErrorCode]
    push    byte 0CAh
    jmp     ContAsyncISR
LowE11:
    pop     dword [ss:_dwErrorCode]
    push    byte 0CBh
    jmp     ContAsyncISR
LowE12:
    pop     dword [ss:_dwErrorCode]
    push    byte 0CCh
    jmp     ContAsyncISR
LowE13:
    pop     dword [ss:_dwErrorCode]
    push    byte 0CDh
    jmp     ContAsyncISR
LowE14:
    pop     dword [ss:_dwErrorCode]
    push    byte 0CEh
    jmp     ContAsyncISR
LowE15:
    push    byte 0CFh
    jmp     ContAsyncISR
LowE16:
    push    byte 0D0h
    jmp     ContAsyncISR
LowE17:
    pop     dword [ss:_dwErrorCode]
    push    byte 0F1h
    jmp     ContAsyncISR
Low15:
    push    byte    8Fh
    jmp     ContAsyncISR
Low7:
    push    byte    87h
    jmp     ContAsyncISR

ContAsyncISR:
        pop     byte [ss: .smc+1]
        SaveTrapRegs

        push    gs
        push    fs
        push    ds
        push    es

        ; Set kernel segment registers to what they should be
        mov     eax,ss
        mov     es,eax
        mov     ds,eax
        mov     fs,eax
        mov     gs,eax

.smc:
        mov     al,0    ; Self-modifying code for the lulz
        movzx   eax,al

        ; Test if we exited from virtual 8086
        mov     edx,[esp + REG_DUMP._eflags]
        bt      edx,17
        setc    [_bWasV86]

        mov     [_global_trap_frame],esp

        ; We have two procedures that we may need to call:
        ; - ExceptionDispatch
        ; - InterruptDispatch
        ; They use regparm(1), so EAX is the first argument

        bt      eax,6
        jc      .Exception
        jmp     .IRQ

.Except:
        and     al,11000000b
        call    ExceptionDispatch
.IRQ:
        and     al,11000000b
        call    InterruptDispatch

        pop     es
        pop     ds
        pop     fs
        pop     gs

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
; This assembly stub is meant to be reentrant. It only uses thread local data.
; VERY performance sensitive!
;
;
        align   64
LowHandleIntV86Reflection:
        ;The stack is assumed to be what it would look like if a switch to
        ;ring 0 from 3 happened. In the case of virtual 8086 mode, segment
        ;registers were pushed on the stack first. If it was protected mode
        ;we still have to save the segment registers.

        SaveTrapRegs

        mov     ebx,esp
        and     ebx,~1FFFh

        test    dword [ss:esp+IFRAME._eflags],(1<<17)
        setnz   [ss:ebx+RSAVE.u_was_v86]
        jnz     .WasV86

        ; Not V86

        ; We need to save the segment registers
        mov     [ss:ebx+RSAVE.u_gs],gs
        mov     [ss:ebx+RSAVE.u_es],es
        mov     [ss:ebx+RSAVE.u_fs],fs
        mov     [ss:ebx+RSAVE.u_ds],ds

        ; Go back to the user mode thread

.WasV86:
        ; Here, we have to copy the data segment registers from the stack
        ; and place them in RSAVE


.RestOfProcedure:

        ; Regardless of V86 or not, we need to set our data segment registers
        ; to the proper values.
        mov     eax,ss
        mov     ds,eax
        mov     es,eax
        mov     gs,eax
        mov     fs,eax

        ; Move EFLAGS, CS, SS, and EIP to the register save area
        ; The order in rsave is the same as on the stack rn.
        lea     esi,[esp+IFRAME._eip]
        lea     edi,[ebx+RSAVE.u_eip]
        mov     ecx,4
        cld
        rep     movsd

        push    ebx     ; Save EBX, we will need it later
        sti
        call    HandleIntV86Reflection
        cli
        pop     ebx

        ; Likely branch assumed to be V86. Will this matter though?
        cmp     dword [ebx+RSAVE.u_was_v86],0
        jnz     .ReturnToPM

.ReturnToV86:

        ; If we were in V86 mode, we need to place the current segment registers
        ; into the V86 IRET stack frame. Sometimes, segment registers
        ; are modified by V86 hooks. We can just string copy them
        mov     ecx,4                           ; 5
        lea     esi,[ebx+RSAVE.u_dsegs]         ; 4
        lea     edi,[esp+IFRAME._dsegs]         ; 4
        cld                                     ; 1
        rep     movsd                           ; 2

        ; We cannot do this for protected mode, so skip to the common procedure
        jmp     .ReturnCommon
.ReturnToPM:
        ; We must manually write the data segment registers
        ; Moving them directly works as long as we do DS last
        mov     es,word [ebx+RSAVE.u_es]
        mov     fs,word [ebx+RSAVE.u_fs]
        mov     gs,word [ebx+RSAVE.u_gs]
        mov     ds,word [ebx+RSAVE.u_ds]

.ReturnCommon:
        ;Write EFLAGS, CS, SS, EIP onto the stack
        ;The data segment registers may be destroyed here, so we must
        ;use a segment override.

        mov     ecx,4
        lea     esi,[ss: ebx+RSAVE.u_eip]
        lea     edi,[ss: esp+IFRAME._eip]
        rep     movsd

        RestTrapRegs
        iret

        section .bss

        global _ErrorCode, _ExceptIndex

_dwErrorCode    RESD    1
_dwExceptIndex  RESD    1

; DO NOT MAKE THIS GLOBAL
dwActualIRQ     RESD    1
