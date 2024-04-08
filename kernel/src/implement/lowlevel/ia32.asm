;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                           ;;
;;                     Copyright (C) 2023-2024, Joey Qytyku                  ;;
;;                                                                           ;;
;; This file is part of OS/90 and is published under the GNU General Public  ;;
;; License version 2. A copy of this license should be included with the     ;;
;; source code and can be found at <https://www.gnu.org/licenses/>.          ;;
;;                                                                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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
GDT_RM_CS       EQU     10      ; TODO, may not need.
GDT_RM_DS       EQU     11
GDT_ENTRIES     EQU     12

;---------------------------------------------------------------------------
; Real mode CS and DS segments are used for switching to physical real mode.
;
;---------------------------------------------------------------------------


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

IRQ_BASE        EQU     0A0h
ICW1            EQU     1<<4
LEVEL_TRIGGER   EQU     1<<3
ICW1_ICW4       EQU     1
ICW4_8086       EQU     1
ICW4_SLAVE      EQU     1<<3

;                           E N D   E Q U  A T E S
;===============================================================================

;===============================================================================
;                               I M P O R T S

extern IDT

;                           E N D   I M P O R T S
;===============================================================================

;===============================================================================
;                               E X P O R T S

global i386GetDescriptorAddress
global i386SetDescriptorAddress
global i386SetDescriptorLimit

;                           E N D   E X P O R T S
;===============================================================================

        section .bss

LDT:
        RESQ    LDT_SIZE

TSS:
        RESB    104
IOPB:
        RESB    8192

_KernelReserved4K:
        RESD    1


;===============================================================================
                                section .data
;===============================================================================

        align 16
GDT:
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
        DD      GDT

;===============================================================================
                                section .text
;===============================================================================

;-------------------------------------------------------------------------------
; Arguments on cdecl stack:
;       PVOID   address
;
i386GetDescriptorAddress:
        ; Descriptors are perfectly valid offsets to the LDT when the DPL
        ; and GDT/LDT bits are masked off.

        mov     EBX,[ESP+4]

        ; EBX is now the address of our descriptor

        ; Let us put the segment descriptor address confusion to rest
        ; The first address field is 16-bit.
        ; It is equal to TheFullAddress & 0xFFFF. Nuff said.
        ;
        ; The last two fields are BYTEs. To make the high address word
        ; we left shift by 8 the 31..24 field
        ; then we OR it with the 23..16
        mov     EAX,[EBX+2]
        movzx   ECX,BYTE [EBX+3]
        movzx   EDX,BYTE [EBX+7]        ; 31..24
        shl     EDX,8
        or      ECX,EDX
        or      EAX,ECX

        ;EAX now contains the address, we are done.
        ret

; Don't remember too well but ChatGPT did help me a bit with this one

i386SetDescriptorAddress: ;(VOID *gdt_entry, DWORD address)

        push    ebp
        mov     ebp,esp

        mov     ebx,[ebp+8]

        movzx   eax, word [ebp+12]
        mov     [ebx+2],ax

        movzx   eax, BYTE [ebp+12+2]
        mov     [ebx+4],al

        movzx   eax, BYTE [ebp+12+3]
        mov     [ebx+7],al
        leave
        ret

; EAX, EDX, ECX
i386SetDescriptorLimit:
        mov     esi, [esp+4]
        mov     dx, [esp+8]

        mov     word [esi], dx
        mov     word [esi+6], dx

        mov     ah, BYTE [esi +6]
        shl     ax, 8
        mov     BYTE [esi+2], ah
        ret

        ; Clobbers: EAX, ECX, EBX, EDX
SetIntVector:
        mov     edx,[esp+12]    ; Base address
        movzx   eax,BYTE[esp+8] ; Info
        mov     ebx,[esp+4]     ; Index of descriptor

        ; EBX is not the address yet, only index. Calculate.
        lea     ebx,[ebx*8+IDT]

        mov     BYTE[ebx+5],al
        mov     BYTE[ebx+4],0
        mov     [ebx+2],cs
        mov     [ebx+0],ax
        shr     edx,16
        mov     [ebx+6],ax
        ret

%macro ConfPIC
        ;---------------------------
        ; C o n f i g u r e  P I C
        ;---------------------------

        ; Different bits tell OCWs and ICWs appart in CMD port
        ; Industry standard architecture uses edge triggered interrupts
        ; 8-BYTE interrupt vectors are default (ICW[:2] = 0)
        ; which is correct for protected mode.

        ;ICW1 to both PIC's
        mov     AL,ICW1 | ICW1_ICW4
        out     20h,AL
        out     0A0h,AL

        ;Set base interrupt vectors
        mov     AL,IRQ_BASE
        out     21h,AL
        mov     AL,IRQ_BASE+8
        out     0A1h,AL

        ;ICW3, set cascade
        mov     AL,4        ; For master it is a bit mask
        out     21h,AL
        mov     AL,2        ; For slave it is an INDEX
        out     0A1h,AL

        ;Send ICW4
        mov     AL,ICW4_8086
        out     21h,AL
        mov     AL,ICW4_8086|ICW4_SLAVE ; Assert PIC2 is slave
        out     0A1h,AL
%endmacro

%macro SetupLDTD_LDTR 0
        ;---------------------------
        ; Create LDT descriptor
        ;---------------------------

        push    LDT
        push    GDT + 8*GDT_LDT
        call    i386SetDescriptorAddress
        add     esp,8

        ;---------------------------
        ; Set LDTR
        ;---------------------------
        mov     ax,GDT_LDT<<3
        lldt    ax

%endmacro

%macro SetupTSSD_IOPB 0
        ;---------------------------------
        ; Set TSS to deny-all by default.
        ;---------------------------------

        mov     word[TSS+100], 0FFFFh
        ;----------------------------------
        ; Insert address to TSS descriptor
        ;----------------------------------
        push    TSS
        push    GDT + 8*GDT_TSSD
        call    i386SetDescriptorAddress
        add     esp,8

        mov     ax,GDT_TSSD<<3
        ltr     ax
%endmacro



%macro ZeroBSS 0
        cld
        xor     eax,eax
        mov     ecx,BSS_SIZE
        mov     edi,END_CODE
        rep     stosb
%endmacro

%macro SetupSegments 0
        lgdt    [GdtInfo]
        mov     ax,2<<3
        mov     ds,ax
        mov     es,ax
        mov     ss,ax
        mov     fs,ax
        mov     gs,ax
        jmp     8h:%%Cont
%%Cont:
%endmacro

;===============================================================================
                                section .init
;===============================================================================

        extern END_CODE
        extern BSS_SIZE
        extern KernelMain

Begin:
        ZeroBSS
        SetupSegments
        ConfPIC

        SetupLDTD_LDTR
        SetupTSSD_IOPB

        mov     esp,100000h+65520
        call    KernelMain
        jmp $

;===============================================================================

