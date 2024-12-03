
;
; Directly referenced by the IDT.
;

	align   64
IRQ0:
        ; Currently testing V86, this will do nothing ATM.
        nop
        push    eax
        mov     al,20h
        out     20h,al
        pop     eax
        iret

        ; Fixed, used to be 7
        test    dword [ss:esp+4],3
        jz      SWITCH_FROM_R0

SWITCH_FROM_R3:
        ; Why is there a B character in the CS? TF?
        ; Must have something to do with how I manage the stack.
        ; Other than that, I am not so sure.

        ; 8086AA42?

        ; Okay, partially fixed now.
        ; THIS IS STILL HAPPENING
        mov ebx,[ss:esp+4]
        mov eax,0xCAFEBABE
        jmp $
SWITCH_FROM_R0:
        ; Save all registers to the current task block
        push    ebx
        mov     ebx,10h
        mov     ds,ebx
        mov     es,ebx

        ; This is where I would do the scheduling stuff?

        mov     ebx,esp
        and     ebx,~4095

        mov     [ebx+_EAX],eax
        mov     [ebx+_ECX],ecx
        mov     [ebx+_EDX],edx

        mov     [ebx+_ESI],esi
        mov     [ebx+_EDI],edi

        mov     [ebx+_EBP],ebp

        mov     [ebx+_ESP],esp
        add     dword [ebx+_ESP],12

        pop     dword [ebx+_EBX]

        ; Everything is saved now. I can destroy the registers.

        ; Are we switching to ring 3? If so, we will use a stack-based entry.
        test    dword [ebx+_CS],3
        jnz     PREP_SWITCH_FROM_R0_TO_R3

        ; Copy the current IRET frame into the current task structure.
        cld
        mov     esi,esp
        lea     edi,[ebx+_ESP]
        mov     ecx,4
        rep     movsd

        mov     ebx,[ebx+80]    ; Will need later

        ; Switch the stack. No worries, the current one is saved and set
        ; to clear upon reentry of current thread.

        mov     esp,[ebx+_ESP]

        ; Allocate on the new stack to copy the IRET frame
        sub     esp,12

        ; Copy the new IRET frame from the next task
        ; Wait. Before re-entering, the stack is cleared in the switch
        ; process. We are essentially pushing values here.

        ; Yes, with the intent of popping them.
        ; Nothing wrong with that. The stack is switched for obvious reasons.
        ; What could possibly be wrong here?

        push    dword [ebx+_EFLAGS]
        push    dword [ebx+_CS]
        push    dword [ebx+_EIP]

        ; Send EOI
        mov eax,20h
        out 20h,al

        ; Load the context
        mov     eax,[ebx+_EAX]
        mov     ecx,[ebx+_ECX]
        mov     edx,[ebx+_EDX]
        mov     esi,[ebx+_ESI]
        mov     edi,[ebx+_EDI]
        mov     ebp,[ebx+_EBP]
        mov     ebx,[ebx+_EBX]

        iret                    ; Bravo six, going dark

PREP_SWITCH_FROM_R0_TO_R3:
        ; Push the registers to the stack
        ;<------- TODO
        mov eax,0xDEADBEEF
        jmp $
        Save
        ; If we jump here, it will just fall through to the rest of the
        ; switch procedure.


        ; The standard stack-based entry procedure.
        ; The IRET frame will have SS:ESP included.
SWITCH_TO_R3:
        Restore
        iret

current_task:   DD 0
system_uptime:  DQ 0
preempt_count:  DD 0

