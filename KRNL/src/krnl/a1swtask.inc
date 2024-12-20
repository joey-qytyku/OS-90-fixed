
; A two-dimensional array is used to index the approprate task switch handler.
; I can use E9 to debug this.

align 64
switch_table:
;       To R0   To R3   To VM
;-------------------------------;
DD      r0_r0,  r0_r3,  r0_vm,0 ; From R0
DD      r3_r0,  r3_r3,  r3_vm,0 ; From R3
DD      vm_r0,  vm_r3,  vm_vm,0 ; From VM

; How can we make the table simpler?

; Add a zero value so we can pitch down by *16

;
; In pseudo-C it would be like this:
;       goto [current.index + next.index*3]
;

;
; Also calculate the RB base before jumping.
;

	align   32
IRQ0:
        xchg bx,bx;;;;

        push    eax
        mov     al,20h
        out     20h,al
        pop     eax

        push    ebx
        push    ecx

        mov     ebx,esp
        and     ebx,~4095

        ;
        ; What follows is extremely pipeline hostile code.
        ; Can I make it any better?
        ; Perhaps use more registers
        ;

        ; Set ECX to the next task address
        mov     ecx,ebx
        mov     ecx,[ss:ebx+_next]

        ; Now get the switch action of the next task
        mov     ecx,[ss:ecx+_switch_action]


        ; goto: (next*4) | current

        ret     ;;;; FAILS HERE


        align   64
r0_r0:

        ; Save the general registers to the current TB so we can use them.
        ; DS is already set, no need for an override.

        pop     dword [ebx+_ECX]
        mov     [ebx+_EAX],eax
        mov     [ebx+_EDX],edx
        mov     [ebx+_ESI],esi
        mov     [ebx+_EDI],edi

        mov     [ebx+_EBP],ebp

        ; Save the stack but without EBX and the rest.
        mov     [ebx+_ESP],esp
        add     dword [ebx+_ESP],16     ; EBX,CS,EIP,EFLAGS

        pop     dword [ebx+_EBX]

        ; We do not need an IRET at all. The stack must change, which is not
        ; possible with IRET from ring-0 going into ring-0.

        ; The IRET frame on the stack is EFLAGS, CS, EIP and nothing else.
        ; We only need to save it.

        ; Load the registers from the next thread
        ; EBX is still our current task block.
        mov     ebx,[ebx+_tb_size]
        mov     eax,[ebx+_EAX]
        mov     ecx,[ebx+_ECX]
        mov     edx,[ebx+_EDX]
        mov     esi,[ebx+_ESI]
        mov     edi,[ebx+_EDI]
        mov     ebp,[ebx+_EBP]

        ; Switch the stack to the next thread's
        mov     esp,[ebx+_ESP]

        ; Build an IRET frame for entry

        push    dword [ebx+_EFLAGS]
        push    dword [ebx+_CS]
        push    dword [ebx+_EIP]

        ; Load EBX with the next value, don't need it now.
        mov     ebx,[ebx+_EBX]
        xchg bx,bx;;;;
        iret

        align   32
r0_r3:
        pop     ecx

; Switching to ring-3 protected mode will involve direct segment loads
; and an IRET with the full stack frame. The ring-3 RPL of the CS will tell
; the CPU to switch.
;

        ; Save the current DS register. I do nto want to waste bytes on stack
        ; segment overrides because that harms cache density.

        mov     [ss:ebx+_EAX],eax
        mov     [ss:ebx+_ECX],ecx
        mov     [ss:ebx+_EDX],edx
        mov     [ss:ebx+_ESI],esi
        mov     [ss:ebx+_EDI],edi

        ; Pop EBX from the stack and save it
        pop     dword [ss:ebx+_EBX]

        ; Save the stack pointer. Clean the stack too, EBX is not on it.
        add     esp,12
        mov     [ss:ebx+_ESP],esp

        ; All registers saved, get next task block
        mov     ebx,[ss:ebx+_next]



        ; Load the next
        mov     esp,[ss:ebx+_ESP]

        ;
        ; To the stack: SS, ESP, EFLAGS, CS, EIP
        ;

;        push


r0_vm:

r3_r0:
r3_vm:
r3_r3:

vm_r0:
vm_r3:
vm_vm:

current_task:   DD 0
system_uptime:  DQ 0
preempt_count:  DD 0

