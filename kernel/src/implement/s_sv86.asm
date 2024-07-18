;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                     Copyright (C) 2022-2024, Joey Qytyku                ;;
;;                                                                         ;;
;; This file is part of OS/90.                                             ;;
;;                                                                         ;;
;; OS/90 is free software. You may distribute and/or modify it under       ;;
;; the terms of the GNU General Public License as published by the         ;;
;; Free Software Foundation, either version two of the license or a later  ;;
;; version if you chose.                                                   ;;
;;                                                                         ;;
;; A copy of this license should be included with OS/90.                   ;;
;; If not, it can be found at <https:;;www.gnu.org/licenses/>              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	; TODO: UPDATE FOR NEW CALLING CONVENTIONS!!!!

	.386p
	.model  flat, stdcall
	include OSK/ASM/structs.inc

	extern g_preempt_count:DWORD
	EXTERNDEF TSS:DWORD

	PUBLIC EIP_CONTINUE
	PUBLIC V_HookINTxH, V_INTxH
	PUBLIC g_sv86

	.CODE

;===============================================================================
; In C: BYTE vector, ISR* out_isr_old, ISR isr_new => VOID
;
; The new ISR must call the previous. It does not need to call it directly.
; Compilers notice identical prototypes and convert it to a jump.
;
;
; TODO: SAVE EBX!!!!!!!!!!!!!!!!!
;
V_HookINTxH:
	inc     g_preempt_count


	dec     g_preempt_count
	ret

; PROC is not used here because it does a bunch of hidden things to the
; generated code that are not desireable.
;
; In C, this takes (BYTE vector, PSTDREGS)
;
	align   64
V_INTxH:
	push    ebp
	mov     ebp,esp

	;-----------------------------------------------------------------------
	; ABI requires EBX, ESI, EDI, and EBP to be saved. The last already is.
	;
	push    ebx
	push    esi
	push    edi

	;-----------------------------------------------------------------------
	; Preemption will be off, but occasionally, it will be turned back on.
	;
	inc     g_preempt_count

	xchg bx,bx
	;
	; The task state segment contains the SS:ESP to switch to when ring-3
	; tries to access ring-0. This is set by the scheduler before entering
	; a context that is currently in user mode. This value is reset to the
	; start of the local kernel mode stack upon system exit to ring-3.
	;
	; Simply reading this value is safe since the scheduler ensures that
	; the values are synchronized with the current task.
	;
	push    DWORD ptr [TSS+4]
	push    DWORD ptr [TSS+8]

	;-----------------------------------------------------------------------
	; We will call the first handler chain entry.
	;-----------------------------------------------------------------------

	movzx   eax,[ebp+8]             ; Get vector argument
	mov     ebx,[eax*4+v86_table]   ; Get first handler in the table

	align   16
not_consumed:
	;-----------------------------------------------------------------------
	; Return value is the next handler to call, if there is one.
	; EBX is callee saved, so we can use it here.
	;-----------------------------------------------------------------------
	; Preemption is potentially enabled so that the caller is preemptible
	; while the SV86 data remains protected. Only reflection needs to run
	; in T1.
	;-----------------------------------------------------------------------
	dec     g_preempt_count
	call    ebx
	mov     ebx,eax
	inc     g_preempt_count

	;-----------------------------------------------------------------------
	; Did it specify itself to be the last handler by returning 1 instead?
	; This is done by the default handler. If so, reflect to DOS/BIOS.
	;-----------------------------------------------------------------------
	cmp     al,1            ; If this is the reflection signal handler
	jz      must_reflect    ;       Go to reflection procedure.

	;-----------------------------------------------------------------------
	; Did it consume (Returned !NULL)?
	; If so, we are done. Otherwise, we keep looking.
	;-----------------------------------------------------------------------
	test    eax,eax         ; If not consumed (EAX == NULL if so)
	jnz     not_consumed    ;       Repeat the loop

	;-----------------------------------------------------------------------
	; Otherwise, the INT was consumed. We are done.
	; Delete the saved TSS values (not needed) and restore the registers.
	;-----------------------------------------------------------------------
	jmp     exit

	align   16
must_reflect:

	;-----------------------------------------------------------------------
	; Restore arguments
	;-----------------------------------------------------------------------
	pop     edx
	pop     eax

	;-----------------------------------------------------------------------
	; Set the CS and EIP in STDREGS to the interrupt vector
	;-----------------------------------------------------------------------
	movzx   esi, WORD PTR [eax*4]
	movzx   edi, WORD PTR [eax*4+2]
	mov     DWORD PTR [edx+STDREGS._CS],edi
	mov     DWORD PTR [edx+STDREGS._EIP],esi

	;-----------------------------------------------------------------------
	; OR in VM=1 into eflags and mask out IOPL so it is always zero.
	; EFLAGS is otherwise copied from regparm.
	;-----------------------------------------------------------------------
	or      DWORD PTR [edx]._EFLAGS, 1 shl 17
	and     DWORD PTR [edx]._EFLAGS, not 0x3000

	;-----------------------------------------------------------------------
	; Create the IRET stack frame for SV86.
	; Order of push: GS, DS, DS, ES, SS, ESP, EFLAGS, CS, EIP
	;-----------------------------------------------------------------------

	; We can memcpy the IRET frame. Registers will be read with MOV.

	mov     esi,edx
	lea     esp,[esp-(9*4)]
	mov     edi,esp
	mov     ecx,9
	cld
	rep     movsd

	;-----------------------------------------------------------------------
	; The stack has now advanced forward, but the other registers are not
	; loaded yet. EDX still points to them.
	;-----------------------------------------------------------------------
	; Load all other registers.
	;-----------------------------------------------------------------------
	mov     eax,[edx]
	mov     ebx,[edx+4]
	mov     ecx,[edx+8]
	mov     edx,[edx+12]

	mov     esi,[edx+16]
	mov     edi,[edx+20]
	mov     ebp,[edx+24]

	;-----------------------------------------------------------------------
	; Configure the TSS to restore the stack to what it was exactly right
	; here.
	;
	; The handler for SV86 general protection faults knows that preemption
	; is off, which is unusual for exception handlers. The handler will
	; surely be using the same stack as the thread of the caller.
	;
	;-----------------------------------------------------------------------
	mov     DWORD PTR [TSS+4],esp
	mov     DWORD PTR [TSS+8],ss

	;-----------------------------------------------------------------------
	; Bravo six, going dark.
	;-----------------------------------------------------------------------
	iret
_END:

EIP_CONTINUE:
	;-----------------------------------------------------------------------
	; Clear the stack of the IRET frame that was pushed.
	;-----------------------------------------------------------------------
	add     esp,9*4

	;-----------------------------------------------------------------------
	; Remove the callee saved registers from the stack
	;-----------------------------------------------------------------------
	pop     edx
	pop     eax

	;-----------------------------------------------------------------------
	; Restore base pointer and original TSS values.
	;-----------------------------------------------------------------------

exit:
	pop     DWORD PTR [TSS+8]
	pop     DWORD PTR [TSS+4]

	pop     edi
	pop     esi
	pop     ebx
	pop     ebp
	cld
	ret

V86StubHandler:
	mov     eax,1
	ret     4

InitV86:
	mov     eax,OFFSET V86StubHandler
	mov     edi,OFFSET v86_table
	rep     stosd
	ret

	.DATA

	align 4

	.DATA?
		align   64
g_sv86    DD 0
v86_table DD 256 DUP(?)

END
