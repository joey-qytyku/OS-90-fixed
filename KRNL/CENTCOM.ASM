extern TSS

global EXC_0

global ISR_1, IRQ0

global EXC_0, ISR_REST, IRQ0, RemapPIC, SetIrqMask
global V86_Monitor, INTxH, EnterV86

	_EAX    EQU 0   * 4
	_EBX    EQU 1   * 4
	_ECX    EQU 2   * 4
	_EDX    EQU 3   * 4
	_ESI    EQU 4   * 4
	_EDI    EQU 5   * 4
	_EBP    EQU 6   * 4
	_pm_ES  EQU 7   * 4
	_pm_DS  EQU 8   * 4
	_pm_FS  EQU 9   * 4
	_pm_GS  EQU 10  * 4
	_EIP    EQU 11  * 4
	_CS     EQU 12  * 4
	_EFLAGS EQU 13  * 4
	_ESP    EQU 14  * 4
	_SS     EQU 15  * 4
	_v86_ES EQU 16  * 4
	_v86_DS EQU 17  * 4
	_v86_FS EQU 18  * 4
	_v86_GS EQU 19  * 4

        _tss_ESP0 EQU 4
        _tss_SS0 EQU 8

        ; I think IRET is IOPL sensitive and it is returning to the wrong place.

%macro Save 0
        push    GS
        push    FS
        push    DS
        push    ES

        push    EBP
        push    EDI
        push    ESI

        push    EDX
        push    ECX
        push    EBX
        push    EAX
%endmacro

%macro Restore 0
        pop     EAX
        pop     EBX
        pop     ECX
        pop     EDX

        pop     ESI
        pop     EDI
        pop     EBP

        pop     ES
        pop     DS
        pop     FS
        pop     GS
%endmacro

%macro Exc_nocode 1
        align   16
        global EXC_%+%1

EXC_%+%1:
        push %1
        jmp EXC_CONT
%endmacro

%macro Exc_code 1
        align   16
        global EXC_%+%1
EXC_%+%1:
        pop     dword [ss:EXC_CODE]
        push    %1
        jmp     EXC_CONT
%endmacro

IRQ_BASE        EQU     0A0h
ICW1            EQU     1 << 4
LEVEL_TRIGGER   EQU     1 << 3
ICW1_ICW4       EQU     1
ICW4_8086       EQU     1
ICW4_SLAVE      EQU     1 << 3

section .text

;============================================================================
; System entry dispatching
;============================================================================

; todo make irq0 have its own IDT vector
; seems like a better idea.

; Give these a predictable alignment
; That may they can be inserted to the IDT procedurally.

Exc_nocode 0
Exc_nocode 1
Exc_nocode 2
Exc_nocode 3
Exc_nocode 4
Exc_nocode 5
Exc_nocode 6
Exc_nocode 7
Exc_code   8
Exc_nocode 9
Exc_code   10
Exc_code   11
Exc_code   12
Exc_code   13
Exc_code   14
Exc_nocode 15
Exc_nocode 16
Exc_code   17
Exc_nocode 18
Exc_nocode 19

; Note, current layout for exceptions is technically not reentrant.
; I need to change that later. Error code should be passed on stack.

;
; Things are working. The real problem is that after the INT the code causes
; a fault because the segment limit is exceeded. That is why the opcode is 00.
; Other than that registers work 100% correctly.
;
; It is running something incorrect. Maybe has something to do with my
; input parameters.
;


        align   64
EXC_CONT:
        pop     dword [ss:EXC_INX]
        Save

        mov     eax,10h
        mov     ds,ax
        mov     es,ax

        ; If the stage-2 handler is written in assembly, it can use EDX
        ; to access the saved registers.

        mov     edx,esp

        ; Interrupts are currently off, enable
        ; sti

        ; xchg bx,bx

        ;;;;;;;;;;;;; Problem probably here.
        ; V86 monitor barely changed. Entrace process is correct and has no
        ; errors. I think that the return process is wrong and it has something
        ; to do with the fact that we bypass the entire exception return
        ; mechanism.

        ; Especially not the argument being pushed along with the return addr.

        push    edx
        mov     ebx,[EXC_INX]
        call    [handler_table+ebx*4]
        add     esp,4

        Restore
        iret

        align 4
EXC_CODE DD 0
EXC_INX  DD 0

        align 4
        ; Set the handler? It is not set!
handler_table:
        times 20 DD 0FFFF_FFFFh

;============================================================================
; IRQ Related Routines
;============================================================================

; TODO, save ebx

        align   16
ISR_1:  mov     byte [ss:actual_irq],1
        jmp     ISR_REST

        align   16
ISR_2:  mov     byte [ss:actual_irq],2
        jmp     ISR_REST

        align   16
ISR_3:  mov     byte [ss:actual_irq],3
        jmp     ISR_REST

        align   16
ISR_4:  mov     byte [ss:actual_irq],4
        jmp     ISR_REST

        align   16
ISR_5:  mov     byte [ss:actual_irq],5
        jmp     ISR_REST

        align   16
ISR_6:  mov     byte [ss:actual_irq],6
        jmp     ISR_REST

        align   16
ISR_7:
        mov     byte [ss:actual_irq],7
        jmp     ISR_REST

ISR_8:  mov     byte [ss:actual_irq],8
        jmp     ISR_REST

        align   16
ISR_9:  mov     byte [ss:actual_irq],9
        jmp     ISR_REST

        align   16
ISR_A:  mov     byte [ss:actual_irq],10
        jmp     ISR_REST

        align   16
ISR_B:  mov     byte [ss:actual_irq],11
        jmp     ISR_REST

        align   16
ISR_C:  mov     byte [ss:actual_irq],12
        jmp     ISR_REST

        align   16
ISR_D:  mov     byte [ss:actual_irq],13
        jmp     ISR_REST

        align   16
ISR_E:  mov     byte [ss:actual_irq],14
        jmp     ISR_REST

        align   16
ISR_F:  mov     byte [ss:actual_irq],15
        jmp     ISR_REST

        align   16
ISR_REST:
        ; OS/90 kernel mode does not define the contents of GS and FS in a
        ; thread-local context.
        ; It is not saved by IRQs consequently.

        Save


        ; TODO: Handle spurious interrupts

.CONT:
        ;--------------------------------------------------------------------
        ; This was not a spurious interrupt.
        ;--------------------------------------------------------------------

        mov     ebx,[actual_irq]

        ;--------------------------------------------------------------------
        ; If the interrupt has no 32-bit handler, reflect to DOS/BIOS
        ;--------------------------------------------------------------------
        cmp     dword [stage2_isr_list+ebx*4],0
        jz      .REFLECT

        ;--------------------------------------------------------------------
        ; Call interrupt handler and send appropriate EOI. Handlers take the
        ; trap frame pointer. ESP did not change, so pass it directly.
        ;--------------------------------------------------------------------
        mov     eax,esp
        call    [stage2_isr_list+ebx*4]

        ; IRQ index is still in EBX

        ; Are the first 3 bits 0? If so it must be on the slave PIC.
        test    bl,7
        jz      .EOI_BOTH

.REFLECT:
        ;--------------------------------------------------------------------
        ; Call the reflection entry point in HMA and exit without EOI.
        ; The real mode ISR will send EOI.
        ;--------------------------------------------------------------------
        mov     edx,ebx
        call    28h:16+3000h
        ; That just trashed all the registers, but we dont care now.
        jmp     .NO_EOI

.SPUR:
        ;--------------------------------------------------------------------
        ; actual_irq contains the IRQ index since ISR is zero.
        ; If the spurious IRQ came slave, both get the EOI.
        ;--------------------------------------------------------------------
        cmp     BYTE [actual_irq],15
        jnz     .NO_EOI

        align   16
.EOI_BOTH:
        mov     al,20h
        out     0A0h,al
	jmp $+2

.EOI_MASTER:
        mov     al,20h
        out     20h,al
	jmp $+2

.NO_EOI:
        Restore
        iret
        align 4
actual_irq: DD 0

        align   16
SetIrqMask:
        pushf
        push    ebx

        cli
        mov     eax,[esp+12]
        xchg    ah,al
        mov     EBX,EAX
        and     EAX,255
        out     21h,AL
	jmp $+2

        shr     EBX,8
        mov     EAX,EBX
        out     0A1h,AL
	jmp $+2

        pop     ebx
        popf
        ret

        align   16
GetIrqMask:
        pushf
        push    ebx

        cli
        xor     eax,eax
        in      al,21h
	jmp $+2
        mov     ebx,eax

        in      al,0A1h
	jmp $+2

        shl     ebx,8
        lea     eax,[eax+ebx]

        pop     ebx
        popf
        ret

        align   16
GetInService:           ; Prob wrong or todo remove?
        pushf
        push    ebx
        cli
        xor     eax,eax

        mov     al,0x0b
        out     20h,al
	jmp $+2
        out     0A0h,al
	jmp $+2

        in      al,20h
	jmp $+2
        mov     ebx,eax

        in      al,0A0h
	jmp $+2

        shl     eax,8
        or      eax,ebx

        pop     ebx
        popf
        ret

RemapPIC:
	call    GetIrqMask
	push    eax
	; Different bits tell OCWs and ICWs appart in CMD port
	; Industry standard architecture uses edge triggered interrupts
	; 8-BYTE interrupt vectors are default (ICW[:2] = 0)
	; which is correct for protected mode.

	;ICW1 to both PIC's
	mov     al,(ICW1 | ICW1_ICW4) ;;;;
	out     20h,al
	jmp $+2
	out     0A0h,al
	jmp $+2

	;Set base interrupt vectors
	mov     al,IRQ_BASE
	out     21h,al
	jmp $+2
	mov     al,IRQ_BASE+8
	out     0A1h,al
	jmp $+2

	;ICW3, set cascade
	mov     AL,4        ; For master it is a bit mask
	out     21h,al
	jmp $+2

	mov     al,2        ; For slave it is an INDEX
	out     0A1h,al
	jmp $+2

	;Send ICW4
	mov     al,ICW4_8086
	out     21h,al
	jmp $+2

	mov     AL,ICW4_8086 | ICW4_SLAVE ; Assert PIC2 is slave
	out     0A1h,al
	jmp $+2

	; Rewrite mask register with mask on stack
	call    SetIrqMask

        ; Remove the mask value on the stack and return
        add     esp,4
        ret

;============================================================================
; Task switching and basic scheduler operations
;============================================================================

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

        align   16
stage2_isr_list:
        DD      IRQ0
        times 15 DD 0

; TODO make these regparm?
; VOID S_Terminate(PTASK pt)
; VOID S_ExecKernelThread(KTHREAD_PROC kp, PVOID pass_args)
; PTASK S_NewTask(VOID);
; VOID S_Yield(VOID)
; VOID S_Sched(PTASK pt)
; VOID S_Deactivate(PTASK pt)

S_Terminate:
S_ExecKernelThread:
S_NewTask:
S_Yield:
S_Sched:
S_Deactivate:

; CDECL saved: EBX, ESI, EDI, EBP

        ; Later, I will need to change it.
        ; Hooks cannot really work like this.
        ; I will have to pass the CS:IP.
        ; Or I get it from the STDREGS. Yes.

        ; Are my IRQ vectors set properly?

EnterV86:
        inc     dword [preempt_count]
        mov     [saved_ebx],ebx
        mov     [saved_esi],esi
        mov     [saved_edi],edi
        mov     [saved_ebp],ebp

        ; xchg bx,bx

        mov     ebx,[esp+4]     ; stdregs ptr
        add     ebx,4           ; Why needed?

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
        xchg bx,bx
        iret

        ; This is where the exception handler will resume.
        ; Before IRET, it already set EAX appropriately.
V86_CONT_POINT:
        xchg bx,bx
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
        ; with no way to restore.

        ; NOTE, DO NOT PUSH ANYTHING WITHOUT POPPING BEFORE .END
	movzx   ebx,word [edx+_CS]
	movzx   ecx,word [edx+_EIP]
        shl     ebx,4

        add     ebx,[edx+_EIP]

        movzx   eax,byte[ebx]

        ; EBX = First byte of instruction

        cmp     al,0CDh         ; Is it INT
        jnz     .not_intxh

        ; Now we must return to caller in kernel mode.
        ; This is complicated because there are things
        ; still on the stack.

        ; We don't go back by letting IRET take place.
        ; The context saved on the stack does NOT get
        ; restored, but it does have to be copied to the
        ; location specified by the V86xH call.
        ; We know it is a full context.

        cld
        mov     esi,edx
        mov     edi,[saved_regbuffptr]
        mov     ecx,20
        rep     movsd

        ; Return the encountered vector in EAX.
        movzx   eax,byte[ebx+1]

        ; Leave the monitor.
        jmp     .end

.not_intxh:
        cmp     al,0CFh ; Is it iret?
        jnz     .not_iret

        ; IRET is:
        ; Pop IP, CS, and EFLAGS

        mov     esi,[edx+_ESP]
        mov     ebx,[edx+_SS]
        shl     ebx,4
        add     esi,ebx

        ; The first location we want to pop is already pointed to by SS:SP.
        ; No need to offset by 2.

        xor     eax,eax

        std
        lodsw
        mov     word [edx+_EIP],ax
        lodsw
        mov     word [edx+_CS],ax
        lodsw
        mov     word [edx+_EFLAGS],ax

        add     word [edx+_ESP],6

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
        cmp     al,9Dh
        jnz     .not_popf

        ; Emulate popf. The IF flag are copied as expected.

        inc     word [edx+_EIP]

        mov     ebx,[edx+_SS]
        shl     ebx,4
        add     word[edx+_ESP],2
        add     ebx,[edx+_ESP]

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

saved_ebp  RESD 1
saved_esi  RESD 1
saved_edi  RESD 1
saved_ebx  RESD 1
saved_ss0  RESD 1

saved_regbuffptr RESD 1


section .data

        align   4
current_task:   DD 0
system_uptime:  DQ 0
preempt_count:  DD 0
int_level:      DD 0