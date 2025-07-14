	section .text

EnterV86:
        inc     dword [preempt_count]
        mov     [saved_ebx],ebx
        mov     [saved_esi],esi
        mov     [saved_edi],edi
        mov     [saved_ebp],ebp

        ; xchg bx,bx

        mov     ebx,[esp+4]     ; stdregs ptr
        ; add     ebx,4           ; Why needed?

        mov     [saved_regbuffptr],ebx

        ;
        ; The SV86 stack provided in the context block now contains
        ; an IRET frame pointing to the provided context CS:IP.
        ; It does not need to be valid all the time, but in case we want
        ; to resume the context to run a further interrupt, this is
        ; necessary.
        ;

        ; Save the current #GP handler, we will just change it
        push    dword [handler_table+(0Dh)*4]

        mov     dword [handler_table+(0Dh)*4],V86_Monitor
        pushfd

        ; Set the SS0:ESP0 in the TSS. This is the stack pointer we want to have
        ; when an exception hits. SS0 is always 0x10, so no need to set it.
        ; THIS IS THE CORRECT TIME TO DO IT. Otherwise, we will not be
        ; able to pop off the saved information
        mov     word [TSS+8],10h
        mov     [TSS+4],esp

        ; Now we will push to our stack an IRET frame that will enter SV86.
        ; These will be pushed off and totally discarded later.

        push    dword [ebx+_v86_GS]
        push    dword [ebx+_v86_FS]
        push    dword [ebx+_v86_DS]
        push    dword [ebx+_v86_ES]

        ; It may seem strange that we are pushing the EFLAGS register
        ; more than one time. This makes sense. After a real INT, the CPU
        ; will preserve the register, so yes we set it and save it.

        push    dword [ebx+_SS]
        push    dword [ebx+_ESP]
        push    dword [ebx+_EFLAGS]
        or      dword [esp],(1<<17)     ; VM=1, IOPL=0
        push    dword [ebx+_CS]
        push    dword [ebx+_EIP]

        ; Check here first

        ; Load registers
        mov     eax,[ebx+_EAX]
        mov     ecx,[ebx+_ECX]
        mov     edx,[ebx+_EDX]
        mov     esi,[ebx+_ESI]
        mov     edi,[ebx+_EDI]
        mov     ebp,[ebx+_EBP]
        mov     ebx,[ebx+_EBX]
        ; Bravo six, going dark.
        iret

        ; This is where the exception handler will resume.
        ; Before IRET, it already set EAX appropriately.
V86_CONT_POINT:
        ; xchg bx,bx
        ; Restore the handler
        popfd

        pop     dword [handler_table+0Dh*4]

        ; Restore the callee-saved registers
        mov     ebx,[saved_ebx]
        mov     esi,[saved_esi]
        mov     edi,[saved_edi]
        mov     ebp,[saved_ebp]

	ret

;
; This is a stage-2 handler.
; EDX points to the STDREGS.
; Only for SV86.
;
V86_Monitor:
        ; Ints should be on actually, currently they are left off
        ; with no way to restore. Really???

        ; NOTE, DO NOT PUSH ANYTHING WITHOUT POPPING BEFORE .END
	movzx   ebx,word [edx+_CS]
	movzx   ecx,word [edx+_EIP]
        shl     ebx,4

        add     ebx,[edx+_EIP]

        movzx   eax,byte[ebx]

        ; EBX = First byte of instruction

        cmp     al,0CDh         ; Is it INT
        jnz     .not_intxh

        ; Return the encountered vector in EAX.
        movzx   eax,byte[ebx+1]

        ; Leave the monitor.
        jmp     .end

.not_intxh:
        cmp     al,0CFh ; Is it iret?
        jnz     .not_iret

        ; EAX=FFFFFFFF to indicate that we are out of this INT level.
        mov     eax,~0
        jmp     .end

.not_iret:
        cmp     al,9Ch
        jnz      .not_pushf

        ; Pushf is IOPL-sensitive and faults on V86. This may be to emulate
        ; the value of IF for the monitor since it is pushed on real mode
        ; Here we run it normally and go back to the original context.

        inc     word[edx+_EIP]

        ; We just copy the current flags to the stack.
        ; There is no concern about VM or IOPL, they are never modified by pushf
        sub     word[edx+_ESP],2
        movzx   eax,word[edx+_EFLAGS]
        movzx   edi,word[edx+_ESP]
        movzx   ecx,word[edx+_SS]
        shl     ecx,4
        add     ecx,edi

        ; Can I reduce the boilerplate and caclculate only once?

        mov     word[ecx],ax

        jmp     .cont

.not_pushf:
        cmp     al,9Dh          ; Is it PUSHF
        jnz     .not_popf

        ; Emulate popf. The IF flag are copied as expected.

        inc     word [edx+_EIP]

        mov     ebx,[edx+_SS]
        shl     ebx,4
        add     word[edx+_ESP],2
        add     ebx,[edx+_ESP]

        ; TODO: use xor eax,eax; maybe reorder for speed
        movzx   eax,byte [ebx-2]

        mov     word[edx+_EFLAGS],ax
        jmp     .cont

.not_popf:
        cmp     al,0FBh         ; It is STI
        jnz     .not_sti

        ; Enable interrupts in the flags register. BIOS code usually never
        ; runs this because it never uses interrupts actually! But many drivers
        ; will and need an IRQ to communicate with devices.

        or      dword[edx+_EFLAGS],200h
        inc     dword[edx+_EIP]
        jmp     .cont

.not_sti:
        cmp     al,0FAh         ; Is it CLI
        jnz     .not_cli

        and     dword[edx+_EFLAGS],~(200h)
        inc     dword[edx+_EIP]
        jmp     .cont

.not_cli:
        mov     edx,8086DEADh
        jmp $ ;;;;;;;;;;;;;;

.end:
        ; To exit the monitor, we must continue ring-0 execution at the
        ; remaining code of V86xH. The problem is that the context pointed to
        ; at EDX needs to be propagated in the STDREGS argument and we cannot
        ; change it right now.

        ; Currently, the stack space used by V86xH initially is cleaned
        ; and reflects what it contained before it pushed anything, or at the
        ; start of the function minus any epilogue (there isnt one, a save
        ; buffer was used for EBP).
        ;
        ; To be safe, currently using saved_regbuffptr in case code changes.
        ; Future note: the current GP handler is saved on stack of V86xH.
        ;
        ; !!!!! EAX cannot be clobbered here. It is the return value of
        ; V86xH.

        ; Copy results back into V86xH argument using saved_regbuffptr as the
        ; argument.
        cld
        mov     esi,edx
        mov     edi,[saved_regbuffptr]
        mov     ecx,20
        rep     movsd

        ; Now we will tear down the stack and go straight into the rzero context
        ; of V86xH to continue.
        ; No IRET is needed.

        ; Clear out return value and argument
        add     esp,(20*4)+8

        ; Indicate? What?

        ; No return in this case, that is it.
        ; BTW the segment regsters were set already to kernel values.
        jmp     V86_CONT_POINT

.cont:
        ; Go on after emulating a non-break opcode
        ret

section .bss

        align 4
saved_ebp  RESD 1
saved_esi  RESD 1
saved_edi  RESD 1
saved_ebx  RESD 1
saved_ss0  RESD 1
saved_regbuffptr RESD 1
