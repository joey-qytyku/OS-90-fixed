%include "Asm/Kernel.inc"

;===============================================================================
; DEFINES

%define ACCESS_RIGHTS(present, ring, type) (present<<7 | ring<<6 | type)

; END DEFINES
;===============================================================================

;===============================================================================
; EQUATES
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
; utilized much more wisely. Here, we will allocate 1024 bytes to the LDT.

        LDT_SIZE        EQU     128

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

; END EQUATES
;===============================================================================

;===============================================================================
; IMPORTS
%assign i 0
%rep 17
        extern  LowE %+ i
        %assign i i+1
%endrep

        extern  LowSwi
        extern  Low7, Low15, LowRest

; END IMPORTS
;===============================================================================

;===============================================================================
; EXPORTS
        global  IaAppendAddressToDescriptor
        global  _SetIntVector

        ; Functions for local descriptor table management
        global  GetLdescBaseAddress
        global  GetLdescExtAttr
        global  GetLdescAttr

        global  aqwGlobalDescriptorTable
        global  aqwLocalDescriptorTable

        global  GetESP0, SetESP0
        global  IaUseDirectRing3IO
        global  IaUseVirtualRing3IO
        global  InitIA32

        global  abIoPermissionBitmap
        global  _KernelReserved4K
; END EXPORTS
;===============================================================================
        section .bss
        ;The initialization stack is used only for startup
        ;Processes get independent kernel stacks
        alignb  16
        RESB    1024
InitStack:

aqwInterruptDescriptorTable:
        RESQ    256

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
        align 8
.null_segment:
        DQ  0

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

Exceptions:
        %assign i 0
%rep 17
        DD      LowE %+ i
        %assign i i+1
%endrep

GdtInfo:
        DW      (GDT_ENTRIES*8)-1
        DD      aqwGlobalDescriptorTable
IdtInfo:
        DW      (256*8)-1
        DD      aqwInterruptDescriptorTable

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
; Read mask by default?
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
;Arguments on cdecl stack:
;       PVOID   address
;
;
;Assumes LDT, this does not work on GDT.
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

IaAppendAddressToDescriptor:;(PVOID gdt_entry, DWORD address)
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
        mov     esi, [esp + 4]
        mov     dx, [esp + 8]

        mov     word [esi], dx
        mov     word [esi + 6], dx

        mov     ah, byte [esi + 6]
        shl     ax, 8
        mov     byte [esi + 2], ah
        ret

_SetIntVector:
        mov     edx,[esp+12]
        movzx   eax,byte[esp+8]
        mov     ecx,[esp+4]

        lea     ebx,[aqwInterruptDescriptorTable + ecx*8]
        mov     byte[ebx+5],al
        mov     byte[ebx+4],0
        mov     [ebx+2],cs
        mov     [ebx+0],ax
        shr     edx,16
        mov     [ebx+6],ax
        ret

        ; This procedure is not cdecl compliant, but it is called by
        ; InitIA32, so it has to save some registers
FillIDT:
        push    edi

        mov     ecx,17
.ExceptLoop:
        push    dword [Exceptions + ecx*4]
        push    byte 1_11_01110b
        push    ecx
        call    _SetIntVector
        add     esp,12
        dec     ecx
        jnz     .ExceptLoop


        ; Offset 15..0 -----------------------------
        mov     ebx,qIdtValue
        mov     eax,LowSwi
        mov     [ebx],ax

        ; Selector ---------------------------------
        mov     word [ebx+2],(GDT_KCODE<<3)

        ; Attributes, changed to word?
        mov     word [ebx+4],1_11_01110_000_00000b

        ; Offset 31..16 ----------------------------
        mov     eax,LowSwi
        shr     eax,16
        mov     [ebx+6],ax

        mov     ecx,256-17
        mov     edi,aqwInterruptDescriptorTable+(17*8)
        cld
.SwiLoop:
        ;For the reflection handler, we will write the exact same value every
        ;time to each entry after the exceptions. We do not need _SetIntVector
        ;in order to do this. We will generate two double words and just write
        ;write them directly

        mov     eax,[ebx]
        stosd
        mov     eax,[ebx+4]
        stosd

        dec     ecx
        jnz     .SwiLoop

.IrqLoop:
        mov     eax,IRQ_BASE
        add     eax,ecx

        push    LowRest
        push    byte 0Eh
        push    eax
        call    _SetIntVector
        add     esp,12
        dec     ecx
        jnz     .IrqLoop

        ;Set the potentially spurious ones
        push    Low7
        push    byte 0Eh
        push    IRQ_BASE+7
        call    _SetIntVector

        push    Low15
        push    byte 0Eh
        push    IRQ_BASE+15
        call    _SetIntVector

        pop     edi
        ret

qIdtValue: DD 0,0

InitIA32:
        call    FillIDT
        call    RemapPIC

        ;Set IOPB in the TSS to an out-of-bounds offset.
        ;This will make all ring-3 IO port access a security violation,
        ;allowing for emulation. Direct access is never permitted.
        ;This gives a warning for some reason. Ignore.
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

        pop     edi
        ret

        section .init

        extern END_RODATA
        extern BSS_SIZE
        extern KernelMain

        ;If DOS does not find the MZ signature in an EXE file
        ;it may load it as a COM file, which are terminated with
        ;a return instruction, the loader jumps over this
        ret
        extern mychar

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
        lidt    [IdtInfo]

        mov     ax,2<<3
        mov     ds,ax  ; Here
        mov     es,ax
        mov     ss,ax
        mov     fs,ax
        mov     gs,ax

        jmp     8h:Cont
Cont:
        mov     esp,InitStack   ; Set up a stack
        call    KernelMain      ; GCC does not far return
        jmp $
