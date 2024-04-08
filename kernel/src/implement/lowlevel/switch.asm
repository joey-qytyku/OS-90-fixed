;-------------------------------------------------------------------------------
;
; This is a procedure that is called by an IRQ handler, but by address rather
; than name. It is located at 100400-100 and has a stack at 100600-1009FF.
;
; Interrupts are assumed to be off here.
; EDX is the interrupt vector to invoke.
;
; All registers are destroyed except ESP.
; SREGS are set to default kernel ones.
;
        align   16
        bits    32

        ;------------------------------------------
        ; Save stack pointer
        ;------------------------------------------
        mov     [SAVE_ESP],esp

        ;------------------------------------------
        ; Disable paging and PE.
        ;------------------------------------------

        mov     eax,cr0
        and     eax,0x7FFF_FFFE
        mov     cr0,eax

        bits    16

        ; The segment descriptor caches and segment registers are
        ; invalid at the moment and must be loaded with proper values.

        ;-------------------------------------------------
        ; Setup stack
        ; SS:ESP = FFFF:9F0+16
        ;-------------------------------------------------
        mov     ax,0FFFFh
        mov     ss,ax
        mov     esp,9F0h+16
        mov     ds,ax

        ; ES=0000
        xor     ax,ax
        mov     es,ax
        jmp     0xFFFF:+16+400h+cont
cont:

        ;-------------------------------------------------
        ; Save current IDTR.
        ;-------------------------------------------------
        sidt    [16+400h+SAVE_IDTR]

        ;-------------------------------------------------
        ; Set IDTR to point to zero with limit 1024
        ;-------------------------------------------------
        lidt    [16+400h+RM_IDTR]

        ;-------------------------------------------------
        ; Simulate INT. Self modifying code is not used.
        ;-------------------------------------------------

        pushfw
        call    FAR [ES:EDX*4]

        ;-------------------------------------------------
        ; Go to 16-bit protected mode.
        ;-------------------------------------------------

        mov     eax,cr0
        or      eax,8000_0001h
        mov     cr0,eax

        ;-------------------------------------------------
        ; Load valid selectors
        ;-------------------------------------------------

        jmp     8:1_0000h+NEW_CS
NEW_CS:
        ;-------------------------------------------------
        ; Now in 32-bit code segment.
        ;-------------------------------------------------
        bits    32
        mov     eax,10h
        mov     es,ax
        mov     ds,ax
        mov     ss,ax

        ;-------------------------------------------------
        ; Restore stack pointer and return to caller!
        ;-------------------------------------------------
        mov     esp,[1_0000h+SAVE_ESP]
        ret


        align   16
SAVE_ESP:       DD 0
SAVE_IDTR:
        DW      0,0,0,0
RM_IDTR:
        DW      0,0,1024,0
