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

;-------------------------------------------------------------------------------
; This is a procedure that is called by an IRQ handler, but by address rather
; than symbol.
;
; On x86, the CPU must be in 16-bit protected or it will freeze. We will now
; be executing inside 103000:0000. We are in a tiny model region with a special
; code and data segment.
;
; To be compatible with real mode addressing, the base address of the PM
; segments are FFFF0. This means the base of the binary must be ORG 16+2000h
; to skip the HMA gap and get to the base address. This is also compatible
; with using a segment value of FFFF.
;
; Interrupts are assumed to be off here.
; EDX is the interrupt vector to invoke.
;
; All registers are destroyed except ESP.
; SREGS are set to default kernel ones.
;
        bits    16
        ORG 16+3000h
        ; What about EBP!!!! Save it outside of this maybe?
        ;------------------------------------------------------
        ; Calculate the real vector to invoke
        ;
        ; All IRQs #8 and above are relative to vector 70h in
        ; real mode.
        ;------------------------------------------------------

        cmp     dl,7
        ja      .above
        jmp     .else
.above:
        add     edx,70h-8
.else:
        add     edx,8   ; They start at #8 anyway

        ; Use the correct data and segments.
        mov     eax,30h
        mov     ds,ax
        mov     [SAVE_ESP],esp
        mov     ss,ax
        mov     es,ax

        ; Change the stack pointer to the correct location.
        ; We will also clear ESP just to be safe.
        mov     esp,3000h+2000h

        ; Save the interrupt mask register. For some reason, interrupts are
        ; re-enabled by real mode and the timer IRQ causes interference.
        ; To ensure such issues never happen, we will mask them all out later.

        in      al,21h
        mov     [OLD_MASK_M],al
        in      al,0A1h
        mov     [OLD_MASK_S],al

        ; Mask all interrupts
        mov     al,255
        out     21h,al
        out     0A1h,al

        ; Save the IDTR. The GDTR is not actually destroyed by real mode.
        ; Make sure the operation uses 32-bit operands so the full address is
        ; loaded.
        DB 66h
        sidt    [SAVE_IDTR]

        ; Switch to real mode
        mov     eax,cr0
        xor     eax,8000_0001h
        mov     cr0,eax

        ; Load the IDTR with something correct with real mode. Yes it works
        ; like this! The IDTR is valid in real mode.
        lidt    [RM_IDTR]

        ; Now in real mode. Make CS contain a proper segment so we can return
        ; from INT later.

        jmp     0FFFFh:cont
cont:
        mov     ax,0FFFFh
        mov     ds,ax
        mov     ss,ax

        ; Could this be a stack error? Maybe SP is to high?

        ; ES will address the IVT
        xor     ax,ax
        mov     es,ax
        mov     fs,ax
        mov     gs,ax

        xor     ebx,ebx
        xor     ecx,ecx
        xor     esi,esi
        xor     edi,edi
        xor     ebp,ebp

        ; Now we run the INT. But we do not want to use self-modifying code
        ; and cannot call an arbitrary int. The instruction will be simulated.

        ;mov     al,'I'
        ;out     0E9h,al

        pushf
        call    far [es:edx*4]

        ;mov     al,'O'
        ;out     0E9h,al

        mov     al,[OLD_MASK_M]
        out     21h,al
        mov     al,[OLD_MASK_S]
        out     0A1h,al

        mov     eax,cr0
        xor     eax,8000_0001h
        mov     cr0,eax

        mov     eax,10h
        mov     ss,ax
        mov     es,ax
        mov     ds,ax
        mov     esp,[dword 0FFFF0h + SAVE_ESP]

        DB 66h
        lidt    [dword 0FFFF0h + SAVE_IDTR]

        ; Perform a 32-bit far return. This will use 32-bit stack values despite
        ; the code segment being 16-bit. Yes x86 really is like that.
        retfd

        align   16
SAVE_ESP:       DD 0
SAVE_IDTR:
        DW      0,0,0,0,0,0
RM_IDTR:
        DW      1024,0,0,0,0
OLD_MASK_M: DB 0
OLD_MASK_S: DB 0
