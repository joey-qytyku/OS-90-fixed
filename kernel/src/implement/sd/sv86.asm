; ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
; บ                   Copyright (C) 2023-2024, Joey Qytyku                     บ
; ฬออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออน
; บ  This file is part of OS/90 and is published under the GNU General Public  บ
; บ    License version 2. A copy of this license should be included with the   บ
; บ      source code and can be found at <https://www.gnu.org/licenses/>.      บ
; ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ

struc STDREGS

._EAX    RESD 1
._EBX    RESD 1
._ECX    RESD 1
._EDX:   RESD 1

._ESI    RESD 1
._EDI    RESD 1
._EBP    RESD 1

._pm_ES  RESD 1
._pm_DS  RESD 1
._pm_FS  RESD 1
._pm_GS: RESD 1

._EIP:   RESD 1
._CS     RESD 1
._EFLAGS RESD 1
._ESP    RESD 1
._SS     RESD 1

._v86_ES RESD 1
._v86_DS RESD 1
._v86_FS RESD 1
._v86_GS RESD 1
.size:

endstruc

;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
;                                I m p o r t s
;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

        extern g_preempt_count
        extern TSS

;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
;                                E x p o r t s
;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

        global EIP_CONTINUE
        global V_HookINTxH, V_INTxH
        global g_sv86

;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
                                section .text
;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ


;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
; In C: BYTE vector, ISR* out_isr_old, ISR isr_new => VOID
;
; The new ISR must call the previous. It does not need to call it directly.
; Compilers notice identical prototypes and convert it to a jump.
;
;
; TODO: SAVE EBX!!!!!!!!!!!!!!!!!
;
V_HookINTxH:
        inc     dword [g_preempt_count]

        mov     esi,[eax*4+v86_table]   ; ESI = The current entry
        mov     [edx],esi               ; *out_isr_old = current entry (ESI)
        mov     [eax*4+v86_table],ecx   ; current entry = isr_new

        dec     dword [g_preempt_count]
        ret

V_INTxH:
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Save the base pointer.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        push    ebp

        xchg bx,bx
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Copy the current SS0:ESP0 values in the TSS. They belong to the
        ; active process and reflect the stack location on system entry.
        ; This means it CANNOT be clobbered after leaving this function.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        push    dword [TSS+4]
        push    dword [TSS+8]

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; We will call the first handler chain entry.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ

        ; Save arguments and ECX, will pop later
        push    ebx
        push    eax
        push    edx

        mov     ebx,[eax*4+v86_table]
.not_consumed:
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Return value is the next handler to call, if there is one.
        ; EBX is callee saved, so we can use it here.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Preemption is potentially enabled so that the caller is preemptible
        ; while the SV86 data remains protected. Only reflection needs to run
        ; in T1.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        dec     dword [g_preempt_count]
        call    ebx
        mov     ebx,eax
        inc     dword [g_preempt_count]

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Did it specify itself to be the last handler by returning 1 instead?
        ; This is done by the default handler. If so, reflect to DOS/BIOS.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        cmp     al,1            ; If this is the reflection signal handler
        jz      .must_reflect   ;       Go to reflection procedure.

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Did it consume (Returned !NULL)?
        ; If so, we are done. Otherwise, we keep looking.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        test    eax,eax         ; If not consumed (EAX == NULL if so)
        jnz     .not_consumed   ;       Repeat the loop

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Otherwise, the INT was consumed. We are done.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ

        add     esp,16
        pop     ebx
        pop     ebp
        ret

.must_reflect:

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Restore arguments
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        pop     edx
        pop     eax

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Set the CS and EIP in STDREGS to the interrupt vector
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        movzx   esi, word[eax*4]
        movzx   edi, word[eax*4+2]
        mov     [edx+STDREGS._CS],edi
        mov     [edx+STDREGS._EIP],esi

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; OR in VM=1 into eflags and mask out IOPL so it is always zero.
        ; EFLAGS is otherwise copied from regparm.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        or      dword [edx+STDREGS._EFLAGS],1<<17
        and     dword [edx+STDREGS._EFLAGS],~(0x3000)

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Create the IRET stack frame for SV86.
        ; Order of push: GS, DS, DS, ES, SS, ESP, EFLAGS, CS, EIP
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ

        ; We can memcpy the IRET frame. Registers will be read with MOV.

        mov     esi,edx
        lea     esp,[esp-(9*4)]
        mov     edi,esp
        mov     ecx,9
        cld
        rep     movsd

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; The stack has now advanced forward, but the other registers are not
        ; loaded yet. EDX still points to them.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Load all other registers.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        mov     eax,[edx]
        mov     ebx,[edx+4]
        mov     ecx,[edx+8]
        mov     edx,[edx+12]

        mov     esi,[edx+16]
        mov     edi,[edx+20]
        mov     ebp,[edx+24]

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Configure the TSS to restore the stack to what it was exactly right
        ; here.
        ;
        ; The handler for SV86 general protection faults knows that preemption
        ; is off, which is unusual for exception handlers. The handler will
        ; surely be using the same stack as the thread of the caller.
        ;
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        mov     [TSS+4],esp
        mov     [TSS+8],ss


        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Bravo six, going dark.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        iret
.end:
EIP_CONTINUE:
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Clear the stack of the IRET frame that was pushed.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        add     esp,9*4

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Remove the callee saved registers from the stack
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        pop     edx
        pop     eax

        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        ; Restore base pointer and original TSS values.
        ;ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
        pop     dword [TSS+8]
        pop     dword [TSS+4]

        pop     ecx
        pop     ebp

        ret

V86StubHandler:
        mov     eax,1
        ret

InitV86:
        mov     eax,V86StubHandler
        mov     edi,v86_table
        rep     stosd
        ret

;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
                                section .data
;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

        align   64
g_sv86 DD 0

v86_table:
        times 256 DD V86StubHandler
;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
                                section .bss
;อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

