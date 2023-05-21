%include "Kernel.inc"

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

; The data/stack segment enables the BIG bit
; so that Plug-and-play BIOS recognizes it as
; a 32-bit stack

        TYPE_DATA       EQU     0x12
        TYPE_CODE       EQU     0x1B

        TYPE_LDT        EQU     0x2
        TYPE_TSS        EQU     0x9

        IRQ_BASE        EQU     A0h
        ICW1            EQU     1<<4
        LEVEL_TRIGGER   EQU     1<<3
        ICW1_ICW4       EQU     1
        ICW4_8086       EQU     1
        ICW4_SLAVE      EQU     1<<3

; END EQUATES
;===============================================================================

;===============================================================================
; IMPORTS
        extern LowDivide0, LowDebug, LowNMI, LowBreakpoint, LowOverflow
        extern LowBoundRangeExceeded, LowInvalidOp, LowDevNotAvail
        extern LowDoubleFault, LowSegOverrun, LowInvalidTSS, LowSegNotPresent
        extern LowStackSegFault, LowGeneralProtect, LowPageFault, LowAlignCheck
        extern LowHandleIntV86Reflection

        extern Low7, Low15, LowRest

; END IMPORTS
;===============================================================================

;===============================================================================
; EXPORTS
        global  IaAppendAddressToDescriptor
        global  _SetIntVector
        global  GetESP0, SetESP0
        global  aqwGlobalDescriptorTable
        global  InitIA32

        global  IaUseDirectRing3IO
        global  IaUseVirtualRing3IO

        global  _KernelReserved4K
; END EXPORTS
;===============================================================================

        section .data

aqwGlobalDescriptorTable:
.null_segment:
        DQ  0

.kcode:
        DW      0FFFFh
        DW      0
        DB      0
        DB      ACCESS_RIGHTS(1,0,TYPE_CODE)
        DB      0CFh
.kdata:
        DW      0FFFFh
        DB      0,0,0
        DB      ACCESS_RIGHTS(1,0,TYPE_DATA)
        DB      0CFh
.ucode:
        DW      0FFFFh
        DB      0,0,0
        DB      ACCESS_RIGHTS(1,3,TYPE_CODE)
        DB      0CFh
.udata:
        DW      0FFFFh
        DB      0,0,0
        DB      ACCESS_RIGHTS(1,3,TYPE_DATA)
        DB      0CFh
.tss:
        DW      0FFFFh
        DB      0,0,0
        DB      ACCESS_RIGHTS(1,0,TYPE_TSS)
        DB      0CFh
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

        section .bss

aqwInterruptDescriptorTable:
        RESQ    256

aqwLocalDescriptorTable:
        RESQ    8192

adwTaskStateSegment:
        RESB    104

        ; It is in BSS so no need to zero it manually
awIoPermissionBitmap:
        RESB    8192

GdtInfo:
        DW      (GDT_ENTRIES*8)-1
        DD      aqwGlobalDescriptorTable
IdtInfo:
        DW      (256*8)-1
        DD      aqwInterruptDescriptorTable


        section .text

;Virtual 8086 runs in ring 3, and it may need to access IO ports directly.
;This cannot be done with IOPL. The IOPB is set to an invalid offset to deny
;all ports. It is set to 104 to allow all.
;
;Ring 3 processes must always use virtual IO. V86 calls to DOS/BIOS by
;the kernel have complete access to IO. The default state is to always
;virtualize IO in ring 3, so it must be reset after changing.

IaUseDirectRing3IO:
        mov     word [adwTaskStateSegment+64h],104
        ret
IaUseVirtualRing3IO:
        mov     word [adwTaskStateSegment+64h],0FFFFh
        ret

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
;       WORD    selector
;
;Assumes LDT, this does not work on GDT.
;
GetDescriptorBaseAddress:
        ; Descriptors are perfectly valid offsets to the LDT when the DPL
        ; and GDT/LDT bits are masked off.

        mov     ebx,[esp+4]
        and     ebx,~11b
        add     ebx,aqwLocalDescriptorTable

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
        mov     edx,byte [ebx+7]        ; 31..24
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
        push    byte 0Eh
        push    ecx
        call    _SetIntVector
        add     esp,12
        dec     ecx
        jnz     .ExceptLoop


        ; Offset 15..0 -----------------------------
        mov     ebx,qIdtValue
        mov     eax,LowHandleIntV86Reflection
        mov     [ebx],ax

        ; Selector ---------------------------------
        mov     word [ebx+2],(GDT_KCODE<<3)

        ; Attributes
        mov     [ebx+4],1_00_01110_000_00000b

        ; Offset 31..16 ----------------------------
        mov     eax,LowHandleIntV86Reflection
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

        extern LKR_END_PROG_DATA
        extern LKR_END
        extern KernelMain

        ;If DOS does not find the MZ signature in an EXE file
        ;it may load it as a COM file, which are terminated with
        ;a return instruction, the loader jumps over this
        ret

Begin:
        ; Zero the BSS section
        cld
        xor     eax,eax
        mov     ecx,LKR_END
        sub     ecx,LKR_END_PROG_DATA
        mov     edi,LKR_END_PROG_DATA
        rep     stosb

        ;EDX contains the program base segment of the bootloader
        ;we don't need the loader now. We will convert it to a valid
        ;linear address so the kernel can use it.
        shr     edx,4
        mov     [KernelReserved4K],edx

        lgdt    [GdtInfo]
        lidt    [IdtInfo]

        mov     ax,2<<3
        mov     ds,eax
        mov     es,eax
        mov     ss,eax
        mov     fs,eax
        mov     gs,eax

        jmp    8h:Cont
Cont:
        mov     esp,InitStack   ; Set up a stack
        call    KernelMain      ; GCC does not far return

        ;The kernel has completely initialized and the stack is now empty
        ;Interrupts are still off, but ready to be recieved
        ;We will now enter the first process
        ;The return value of KernelMain is the address to the PCB of the
        ;INIT program

        ;Create the IRET frame
        push    dword [eax+_ss]
        push    dword [eax+_esp]
        pushf                           ; We will enter with current flags
        or      dword [esp],200h        ; But interrupts will be enabled
        push    dword [eax+_cs]
        push    dword [eax+_eip]
        iret                            ; Goodbye!

section	.bss
        ;The initialization stack is used only for startup
        ;Processes get independent kernel stacks
        align   8

_KernelReserved4K:
        RESD    1

        RESB    8192
InitStack:


