extern TSS

global EXC_0

global EXC_0, ISR_15, ISR_7, ISR_REST, IRQ0, RemapPIC, SetIrqMask
global V86_Monitor

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

        align   16
EXC_CONT:
        ; If the stage-2 handler is written in assembly, it can use EDX
        ; to access the saved registers.

        mov     edx,esp
        mov     ebx,[EXC_CODE]

        ; Interrupts are currently off, enable
        sti

        call    [handler_table+ebx*4]
        Restore
        iret

        align 4
EXC_CODE DD 0

        align 4
handler_table:
        DD 0 times 20

;============================================================================
; IRQ Related Routines
;============================================================================

; TODO, save ebx

        align   16
ISR_15:
        mov     BYTE  [ss:actual_irq],15
        jmp     ISR_REST

        align   16
ISR_7:
        mov     BYTE [ss:actual_irq],7
        jmp     ISR_REST
        align   16
ISR_REST:
        Save
        ;--------------------------------------------------------------------
        ; Get in-service register, but as a bit mask, not an index.
        ;--------------------------------------------------------------------
        call    GetInService

        ;--------------------------------------------------------------------
        ; Is the in service zero? If so, IRQ is spurious.
        ; Otherwise, continue as usual.
        ; Spurious is unlikely.
        ;--------------------------------------------------------------------
        test    eax,eax
        jnz     .SPUR

.CONT:
        ;--------------------------------------------------------------------
        ; This was not a spurious interrupt.
        ;--------------------------------------------------------------------
        ; Convert the bit mask to an index. It is known that mask != 0.
        ;--------------------------------------------------------------------
        bsf     ebx,eax

        ;--------------------------------------------------------------------
        ; If the interrupt has no 32-bit handler, reflect to DOS/BIOS
        ;--------------------------------------------------------------------
        cmp     dword [stage2_isr_list+eax*4],0
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
        mov     edx,eax
        DB      9Ah,0,0,0,0,38h,00
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

        align   16
.EOI_MASTER:
        mov     al,20h
        out     20h,al
	jmp $+2

        align   16
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
GetInService:
        pushf
        push    ebx
        cli
        xor     eax,eax

        mov     AL,00001010b
        out     20h,AL
	jmp $+2
        out     0A0h,AL
	jmp $+2

        in      al,20h
	jmp $+2
        mov     ebx,eax

        in      al,0A0h
	jmp $+2

        shl     ebx,8
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

        ; This is where I would do the scheduling stuff

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
        xchg bx,bx
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

V86xH: ;BYTE v, PSTDREGS r
        inc     dword [preempt_count]
        mov     [saved_ebx],ebx
        mov     [saved_ebp],esi
        mov     [saved_ebp],edi
        mov     [saved_ebp],ebp

        mov     ebx,[esp+4]     ; stdregs ptr
        mov     esi,[esp+8]     ; vector

        mov     [saved_regbuffptr],ebx

        ; EDI = the stack address being used for SV86
        mov     edi,[ebx+_SS]
        shl     edi,4
        add     edi,[ebx+_ESP]

        shl     esi,2

        std

        ; Push FLAGS. It will be loaded.
        movzx   eax,word [ebx+_EFLAGS]
        lodsw

        ; CS
        movzx   eax,word [ebx+_CS]
        lodsw

        ; IP
        mov     eax,word [ebx+_EIP]

        ; Update ESP value accurately
        add     dword [ebx+_ESP],6

        ;
        ; The SV86 stack provided in the context block now contains
        ; an IRET frame pointing to the provided context CS:IP.
        ; It does not need to be valid all the time, but in case we want
        ; to resume the context to run a further interrupt, this is
        ; necessary.
        ;

        ; Increment the INT counter?
        inc     dword [int_level]

        ; Set the SS0:ESP0 in the TSS. This is the stack pointer we want to have
        ; when an exception hits. SS0 is always 0x10, so no need to set it.
        mov     [TSS+4],esp

        ; Save the current #GP handler, we will just change it
        push    dword [handler_table+0Dh*4]

        ; Now we will push to our stack an IRET frame that will enter SV86.
        ; These will be pushed off and totally discarded later.

        push    dword [ebx+_v86_GS]
        push    dword [ebx+_v86_FS]
        push    dword [ebx+_v86_DS]
        push    dword [ebx+_v86_ES]

        ; It may seem strange that we are pushing the EFLAGS register
        ; more than one time. This makes sense. After a real INT, the CPU
        ; will preserve the register, so yes we set it and save it.

        push    dword [ebx+_ESP]
        push    dword [ebx+_EFLAGS]
        or      dword [esp],(1<<17)|(3<<12)     ; VM=1, IOPL=3
        push    dword [ebx+_CS]
        push    dword [ebx+_EIP]

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
        ; Restore the handler
        pop     [handler_table+0Dh*4]

        dec     dword [int_level]

        ; Restore the callee-saved registers
        mov     ebx,[saved_ebx]
        mov     esi,[saved_ebp]
        mov     edi,[saved_ebp]
        mov     ebp,[saved_ebp]

	ret
        ; Need an int level counter?

INTxH:
        ret

;
; This is a stage-2 handler.
; EDX points to the STDREGS.
;
V86_Monitor:
        mov     ebx,[eax+_EIP]
	movzx   eax,word [esp+_CS]
        shl     eax,4
        add     ebx,eax

        ; EBX = First byte of instruction

        movzx   eax,byte[ebx]

        cmp     al,0CDh         ; Is it INTxH
        jnz     .not_intxh

        ;
        ; Write the INT vector requested to saved EAX
        ;
        movzx   eax,byte[ebx+1]
        mov     [edx+_EAX],eax

        ; Push CS:IP and eflags of current context on the stack so that
        ; iret
        ; can return if needed. We will not actually continue running.

        mov     edi,[edx+_SS]
        shl     edi,4
        add     edi,[edi+_ESP]

        std
        movzx   eax,word[edi+_EFLAGS]
        stosw

        movzx   eax,word[edi+_CS]
        stosw

        movzx   eax,word[edi+_EIP]
        add     eax,2
        stosw

        sub     word[edi+_ESP],6

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

        jmp     .end

.not_intxh:
        cmp     al,0CFh ; Is it iret?
        jnz     .not_iret

        ;
        ; Pop FLAGS, CS, and IP from the stack.
        ;
        jmp $

        jmp     .end

.not_iret:

        jmp $ ;;;;;;;;;;;;;;
.end:
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