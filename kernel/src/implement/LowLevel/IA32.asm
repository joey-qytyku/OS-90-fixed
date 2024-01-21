;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                           ;;
;;                     Copyright (C) 2023, Joey Qytyku                       ;;
;;                                                                           ;;
;; This file is part of OS/90 and is published under the GNU General Public  ;;
;; License version 2. A copy of this license should be included with the     ;;
;; source code and can be found at <https://www.gnu.org/licenses/>.          ;;
;;                                                                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%include "Asm/Kernel.inc"

;===============================================================================
;                                D E F I N E S

%define ACCESS_RIGHTS(present, ring, type) (present<<7 | ring<<6 | type)

;                           E N D   D E F I N E S
;===============================================================================

;===============================================================================
;                               E Q U A T E S
GDT_NULL        EQU     0
GDT_KCODE       EQU     1
GDT_KDATA       EQU     2
GDT_UCODE       EQU     3
GDT_UDATA       EQU     4
GDT_TSSD        EQU     5
GDT_LDT         EQU     6
GDT_PNPCS       EQU     7
GDT_PNP_OS_DS   EQU     8
GDT_PNP_BIOS_DS EQU     9
GDT_ENTRIES     EQU     10

; We are not wasting 64K for LDT entries that will never be used except in
; bizzare circumstances. 64K is a TON of memory that could be
; utilized much more wisely.

; This value must be consistent with the one in IA32/Segment.h
LDT_SIZE        EQU     64

; The data/stack segment enables the BIG bit
; so that Plug-and-play BIOS recognizes it as
; a 32-bit stack

TYPE_DATA       EQU     0x12
TYPE_CODE       EQU     0x1B

TYPE_LDT        EQU     0x2
TYPE_TSS        EQU     0x9

IRQ_BASE        EQU     0A0h            ; Wait, is it?
ICW1            EQU     1<<4
LEVEL_TRIGGER   EQU     1<<3
ICW1_ICW4       EQU     1
ICW4_8086       EQU     1
ICW4_SLAVE      EQU     1<<3

;                           E N D   E Q U  A T E S
;===============================================================================

;===============================================================================
;                               I M P O R T S

extern Interrupt_Descriptor_Table

;                           E N D   I M P O R T S
;===============================================================================

;===============================================================================
;                               E X P O R T S

global  IaAppendAddressToDescriptor,\
        IaGetBaseAddress,\
        IaAppendLimitToDescriptor,\
        aqwGlobalDescriptorTable,\
        aqwLocalDescriptorTable,\
        GetESP0, SetESP0,\
        IaUseDirectRing3IO,\
        IaUseVirtualRing3IO,\
        InitIA32,\
        abIoPermissionBitmap,\
        _KernelReserved4K

;                           E N D   E X P O R T S
;===============================================================================
        section .bss
        ;The initialization stack is used only for startup
        ;Processes get independent kernel stacks
        alignb  16
        RESB    512
InitStack:

aqwLocalDescriptorTable:
        RESQ    LDT_SIZE

adwTaskStateSegment:
        RESB    104
abIoPermissionBitmap:
        RESB    8192

_KernelReserved4K:
        RESD    1


;===============================================================================
        section .data
;===============================================================================

aqwGlobalDescriptorTable:
        align 16
.null_segment:
        DD      0,0
.kcode:
        DW      0FFFFh  ; Limit 0:15
        DW      0       ; Base 0:15
        DB      0       ; Base 16:23
        DB      ACCESS_RIGHTS(1,0,TYPE_CODE)
        DB      0CFh
        DB      0
.kdata:
        DW      0FFFFh
        DB      0,0,0
        DB      ACCESS_RIGHTS(1,0,TYPE_DATA)
        DB      0CFh
        DB      0
.ucode:
        DW      0FFFFh
        DB      0,0,0
        DB      ACCESS_RIGHTS(1,3,TYPE_CODE)
        DB      0CFh
        DB      0
.udata:
        DW      0FFFFh
        DB      0,0,0
        DB      ACCESS_RIGHTS(1,3,TYPE_DATA)
        DB      0CFh
        DB      0
.tss:
        DW      0FFFFh
        DB      0,0,0
        DB      ACCESS_RIGHTS(1,0,TYPE_TSS)
        DB      0CFh
        DB      0
.ldt:
        DW      103
        DB      0,0,0
        DB      ACCESS_RIGHTS(1,0,TYPE_LDT)

GdtInfo:
        DW      (GDT_ENTRIES*8)-1
        DD      aqwGlobalDescriptorTable
IdtInfo:
        DW      (256*8)-1
        DD      Interrupt_Descriptor_Table

;===============================================================================
        section .text
;===============================================================================

GetESP0:
        mov     eax,[adwTaskStateSegment+4]
        ret

SetESP0:
        mov     dword[adwTaskStateSegment+4],eax
        ret

; Different bits tell OCWs and ICWs appart in CMD port
; Industry standard architecture uses edge triggered interrupts
; 8-byte interrupt vectors are default (ICW[:2] = 0)
; which is correct for protected mode.
;
;
; State of PIC?
; Read mask by default? <--------------------
;

RemapPIC:
        ;ICW1 to both PIC's
        mov     al,ICW1 | ICW1_ICW4
        out     20h,al
        out     0A0h,al

        ;Set base interrupt vectors
        mov     al,IRQ_BASE
        out     21h,al
        mov     al,IRQ_BASE+8
        out     0A1h,al

        ;ICW3, set cascade
        mov     al,4        ; For master it is a bit mask
        out     21h,al
        mov     al,2        ; For slave it is an INDEX
        out     0A1h,al

        ;Send ICW4
        mov     al,ICW4_8086
        out     21h,al
        mov     al,ICW4_8086|ICW4_SLAVE ; Assert PIC2 is slave
        out     0A1h,al
        ret

;-------------------------------------------------------------------------------
; Arguments on cdecl stack:
;       PVOID   address
;
IaGetBaseAddress:
        ; Descriptors are perfectly valid offsets to the LDT when the DPL
        ; and GDT/LDT bits are masked off.

        mov     ebx,[esp+4]

        ; EBX is now the address of our descriptor

        ; Let us put the segment descriptor address confusion to rest
        ; The first address field is 16-bit.
        ; It is equal to TheFullAddress & 0xFFFF. Nuff said.
        ;
        ; The last two fields are bytes. To make the high address word
        ; we left shift by 8 the 31..24 field
        ; then we OR it with the 23..16
        mov     eax,[ebx+2]
        movzx   ecx,byte [ebx+3]
        movzx   edx,byte [ebx+7]        ; 31..24
        shl     edx,8
        or      ecx,edx
        or      eax,ecx

        ;EAX now contains the address, we are done.
        ret

; Don't remember too well but ChatGPT did help me a bit with this one
IaAppendAddressToDescriptor:
        ;(PVOID gdt_entry, DWORD address)

        push    ebp
        mov     ebp,esp

        mov     ebx,[ebp+8]

        movzx   eax, word [ebp+12]
        mov     [ebx+2],ax

        movzx   eax, byte [ebp+12+2]
        mov     [ebx+4],al

        movzx   eax, byte [ebp+12+3]
        mov     [ebx+7],al
        leave
        ret

IaAppendLimitToDescriptor:
        mov     esi, [esp+4]
        mov     dx, [esp+8]

        mov     word [esi], dx
        mov     word [esi+6], dx

        mov     ah, byte [esi +6]
        shl     ax, 8
        mov     byte [esi+2], ah
        ret

_SetIntVector:
        mov     edx,[esp+12]
        movzx   eax,byte[esp+8]
        mov     ecx,[esp+4]

        ; lea     ebx,[aqwInterruptDescriptorTable + ecx*8]
        mov     byte[ebx+5],al
        mov     byte[ebx+4],0
        mov     [ebx+2],cs
        mov     [ebx+0],ax
        shr     edx,16
        mov     [ebx+6],ax
        ret

FillIDT:
        ; See Intr_trap.asm for more info.
        ; To make the IDT valid, we have to exchange +0 with +6

        mov     ecx,256
        mov     ebx,Interrupt_Descriptor_Table

Loop:
        movzx   eax,word[ebx+6]
        movzx   edx,word[ebx+2]
        mov     [ebx+2],ax
        mov     [ebx+6],dx

        add     ebx,8
        dec     ecx
        jnz     Loop

        xchg bx,bx

        ret
extern printf
InitIA32:
        xchg bx,bx
        call    FillIDT
        call    RemapPIC

        ;Set IOPB in the TSS to an out-of-bounds offset.
        ;This will make all ring-3 IO port access a security violation,
        ;allowing for emulation. Direct access is never permitted.
        ;This gives a warning for some reason. Ignore.

        ; WAIT REALLY?
        mov     word[adwTaskStateSegment+100], 0FFFFh

        ;Insert address to LDT descriptor
        push    aqwLocalDescriptorTable
        push    aqwGlobalDescriptorTable + 8 * GDT_LDT
        call    IaAppendAddressToDescriptor
        add     esp,8
        ; Set LDTR
        mov     ax,GDT_LDT<<3
        lldt    ax

        ;Insert address to TSS descriptor
        push    adwTaskStateSegment
        push    aqwGlobalDescriptorTable + 8 * GDT_TSSD
        call    IaAppendAddressToDescriptor
        add     esp,8
        ; Set TSSD
        mov     ax,GDT_TSSD<<3
        ltr     ax

        ; The PIC will send the ISR from command port reads
        ; The kernel does not use anything else from the PIC
        mov     al,0Bh
        out     20h,al
        out     0A0h,al

        ; After return, starts executing at address zero in IVT.
        ; Suspected stack corruption.
        ret

;===============================================================================
        section .init
;===============================================================================

        extern END_RODATA
        extern BSS_SIZE
        extern Kernel_Main

        ;If DOS does not find the MZ signature in an EXE file
        ;it may load it as a COM file, which are terminated with
        ;a return instruction, the loader jumps over this
        ret

Begin:
        ; Zero the BSS section
        cld
        xor     eax,eax
        mov     ecx,BSS_SIZE
        mov     edi,END_RODATA
        rep     stosb

        ;EDX contains the program base segment of the bootloader
        ;we don't need the loader now. We will convert it to a valid
        ;linear address so the kernel can use it.

        shr     edx,4
        mov     [_KernelReserved4K],edx
        lgdt    [GdtInfo]

        mov     ax,2<<3
        mov     ds,ax
        mov     es,ax
        mov     ss,ax
        mov     fs,ax
        mov     gs,ax

        xchg bx,bx

        lidt    [IdtInfo]

        jmp     8h:Cont
Cont:
        mov     esp,InitStack
        call    Kernel_Main
        jmp $

;===============================================================================

