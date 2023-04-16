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

        IRQ_BASE        EQU     20h
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
        DD      LowDivide0
        DD      LowDebug
        DD      LowNMI
        DD      LowBreakpoint
        DD      LowOverflow
        DD      LowBoundRangeExceeded
        DD      LowInvalidOp
        DD      LowDevNotAvail
        DD      LowDoubleFault
        DD      LowSegOverrun
        DD      LowInvalidTSS
        DD      LowSegNotPresent
        DD      LowStackSegFault
        DD      LowGeneralProtect
        DD      LowPageFault
        DD      0
        DD      LowAlignCheck

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

%if 0
static STATUS SetupX87(VOID)
{
    // The Native Exceptions bit, when set, the FPU
    // exception is sent to the dedicated vector
    // otherwise, an IRQ is sent. IRQ#13 is hardwired

    DWORD cr0;
    __asm__ volatile ("mov %%cr0, %0":"=r"(cr0)::"memory");

    // If the EM bit is turned on at startup it
    // is assumed that the FPU is not meant to be used

    BOOL fpu_not_present = cr0 & CR0_EM != 0;

    // The 80287 did not native exceptions (obviously), so the NE bit
    // should not be set if that is installed

    BOOL using_80387_or_better = cr0 & CR0_ET != 0;

    if (fpu_not_present)
        return 0;

    if (using_80387_or_better)
    {
        // Are native exceptions supported?
        // If so, enable and mask IRQ#13
    }
}
%endif

;Virtual 8086 runs in ring 3, and it may need to access IO ports directly.
;This cannot be done with IOPL. The IOPB is set to an invalid offset to deny
;all ports. It is set to 104 to allow all.
;
;Ring 3 processes must always use virtual IO. V86 calls to DOS/BIOS by
;the kernel have complete access to IO. The default state is to always
;virtualize IO in ring 3, so it must be reset after changing.

IaUseDirectRing3IO:
        mov word [adwTaskStateSegment+64h],104
        ret
IaUseVirtualRing3IO:
        mov word [adwTaskStateSegment+64h],0FFFFh
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
        ;ECX=Vector, EDX=EIP, AL=Attributes
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

FillIDT:
        push    esi
        mov     esi,_SetIntVector

        mov     ecx,17
.ExceptLoop:
        push    dword [Exceptions + ecx*4]
        push    byte 0Fh
        push    ecx
        call    esi
        add     esp,12
        dec     ecx
        jnz     .ExceptLoop

        mov     ecx,16
.IrqLoop:
        mov     eax,IRQ_BASE
        add     eax,ecx

        push    LowRest
        push    byte 0Eh
        push    eax
        call    esi
        add     esp,12
        dec     ecx
        jnz     .IrqLoop

        ;Set the potentially spurious ones
        push    Low7
        push    byte 0Eh
        push    IRQ_BASE+7
        call    esi

        push    Low15
        push    byte 0Eh
        push    IRQ_BASE+15
        call    esi

        pop     esi
        ret

InitIA32:
        call    FillIDT
        call    RemapPIC

        push    edi

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

    ._eip:   RESD 1
    ._cs:    RESD 1
    ._eflags:RESD 1

    ._esp:   RESD 1
    ._ss:    RESD 1


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


