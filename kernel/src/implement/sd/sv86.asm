; 浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様融
; �                   Copyright (C) 2023-2024, Joey Qytyku                     �
; 麺様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様郵
; �  This file is part of OS/90 and is published under the GNU General Public  �
; �    License version 2. A copy of this license should be included with the   �
; �      source code and can be found at <https://www.gnu.org/licenses/>.      �
; 藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕

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

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
;                                I m p o r t s
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�

        extern g_preempt_count
        extern TSS

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
;                                E x p o r t s
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�

        global EIP_CONTINUE
        global OS_HookINTxH, OS_INTxH_t12

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
                                section .text
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�

;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
; EAX: stdregs ptr, EDX: error code
;
; SV86 does not need the error code. (NOTE SAVE EBX)
;
; What context is this?
;
SV86_HandleGP:
        push    ebx

        mov     esi,[eax+STDREGS._CS]
        shl     esi,4
        add     esi,[eax+STDREGS._EIP]

        pop     ebx
        ret

;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
; In C: BYTE vector, ISR* out_isr_old, ISR isr_new => VOID
;
; The new ISR must call the previous. It does not need to call it directly.
; Compilers notice identical prototypes and convert it to a jump.
;
; I think it should return a thunk though.
;
OS_HookINTxH:
.tryagain:
        bts     dword [sv86_lock],0
        jc      .tryagain

        mov     esi,[eax*4+v86_table]   ; ESI = The current entry
        mov     [edx],esi               ; *out_isr_old = current entry (ESI)
        mov     [eax*4+v86_table],ecx   ; current entry = isr_new

        mov     byte [sv86_lock],0
        ret
;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
; BYTE vector, PSTDREGS regparm => VOID
;
;
OS_INTxH_t12:
.tryagain:
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Save the base pointer.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        push    ebp

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Lock SV86
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        bts     dword [sv86_lock],0
        jc      .tryagain

        xchg bx,bx
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Copy the current SS0:ESP0 values in the TSS. They belong to the
        ; active process and reflect the stack location on system entry.
        ; This means it CANNOT be clobbered after leaving this function.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        push    dword [TSS+4]
        push    dword [TSS+8]

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; We will call the first handler chain entry.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�

        ; Save arguments, will clean later (takes only two)
        push    eax
        push    edx

        mov     ebx,[eax*4+v86_table]
.not_consumed:
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Return value is the next handler to call, if there is one.
        ; EBX is callee saved, so we can use it here.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        call    ebx
        mov     ebx,eax

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Did it specify itself to be the last handler by returning 1 instead?
        ; This is done by the default handler. If so, reflect to DOS/BIOS.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        cmp     al,1            ; If this is the reflection signal handler
        jz      .must_reflect   ;       Go to reflection procedure.

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Did it consume (Returned !NULL)?
        ; If so, we are done. Otherwise, we keep looking.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        test    eax,eax         ; If not consumed (EAX == NULL if so)
        jnz     .not_consumed   ;       Repeat the loop

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Otherwise, the INT was consumed. We are done.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�

        add     esp,16
        pop     ebp
        ret

.must_reflect:

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Restore arguments
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        pop     edx
        pop     eax

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Set the CS and EIP in STDREGS to the interrupt vector
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        movzx   esi, word[eax*4]
        movzx   edi, word[eax*4+2]
        mov     [edx+STDREGS._CS],edi
        mov     [edx+STDREGS._EIP],esi

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; OR in VM=1 into eflags and mask out IOPL so it is always zero.
        ; EFLAGS is otherwise copied from regparm.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        or      dword [edx+STDREGS._EFLAGS],1<<17
        and     dword [edx+STDREGS._EFLAGS],~(0x3000)

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Create the IRET stack frame for SV86.
        ; Order of push: GS, DS, DS, ES, SS, ESP, EFLAGS, CS, EIP
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�

        ; We can memcpy the IRET frame. Registers will be read with MOV.

        mov     esi,edx
        lea     esp,[esp-(9*4)]
        mov     edi,esp
        mov     ecx,9
        cld
        rep     movsd

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; The stack has now advanced forward, but the other registers are not
        ; loaded yet. EDX still points to them.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Load all other registers.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        mov     eax,[edx]
        mov     ebx,[edx+4]
        mov     ecx,[edx+8]
        mov     edx,[edx+12]

        mov     esi,[edx+16]
        mov     edi,[edx+20]
        mov     ebp,[edx+24]

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Configure the TSS to restore the stack to what it was exactly right
        ; here.
        ;
        ; The handler for SV86 general protection faults knows that preemption
        ; is off, which is unusual for exception handlers. The handler will
        ; surely be using the same stack as the thread of the caller.
        ;
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        mov     [TSS+4],esp
        mov     [TSS+8],ss


        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Bravo six, going dark.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        iret
.end:
EIP_CONTINUE:
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Clear the stack of the IRET frame that was pushed.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        add     esp,9*4

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Remove the callee saved registers from the stack
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        pop     edx
        pop     eax

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Restore base pointer and original TSS values.
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        pop     dword [TSS+8]
        pop     dword [TSS+4]

        pop     ebp

        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        ; Unlock SV86
        ;陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�
        mov     byte [v86_table],0

        ret

V86StubHandler:
        mov     eax,1
        ret

InitV86:
        mov     eax,V86StubHandler
        mov     edi,v86_table
        rep     stosd
        ret

;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
                                section .data
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�

        align   64
sv86_lock DD 0

v86_table:
        times 256 DD V86StubHandler
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
                                section .bss
;様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�

