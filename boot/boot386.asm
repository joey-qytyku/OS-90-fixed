%if 0

BOOT386, the OS/90 bootloader written by Joey Qytyku

This program is released to the public domain, feel free to
use and modify it to load other kernels.

- HISTORY -

22 April 2022:
        Added in and fixed Enable-A20 code
23 April 2022:
        Converted to NASM code
20 April 2022:
        Switched to XMS and .COM program
        Removed enable A20 code, XMS used instead
        Updated file macros

1 May 2022:
        Updated enter protected mode routine
        Fully removed A20 code
7 May 2022:
        Fixed A20 handling code
        Added page table copies
        HMA cannot longer be shared
        Removed old error messages
11 May 2022
        Fixed many bugs
        Changed debugging macros
        Added in some file load code
13/14 May 2022
        Added loader code
        Fixed bug with XMS calls
        It appears to be loading into memory
        Fixed GDT limit

        The first two bytes seem to repeat in the kernel

21 May 2022
        This code is working and kernel loads
        It was a linker error, fixed approx. 2 weeks ago

%endif

;-----------------------------
; Equates

%define X2LN 0CDh
%define LNE 10,13,'$'

PAGE_SHIFT      EQU	12
PUTSTR          EQU	9
EXIT            EQU	4Ch
OPEN_RO         EQU	3D00h
CLOSE           EQU	3Eh
READ            EQU	3Fh

SEEK_SET	EQU	4200h
SEEK_CUR	EQU	4201h
SEEK_END	EQU	4202h

SETBLOCK        EQU     4Ah
ALLOC           EQU     48h
        ORG	100h
        jmp	Main

Weclome:        DB      "Starting OS/90",10,13
times 14        DB      X2LN
DB      LNE

;----------------------------
; Error message strings

MnoXMS          DB	"[!] OS/90 requires an XMS driver.",LNE
OpenErr         DB      "[!] Error opening KERNL386.SYS, reinstall OS/90.",LNE
A20Error	DB      "[!] Error enabling A20 gate.",LNE
HMA_Error       DB	"[!] Could not get entire HMA.",LNE
ExtMemErr       DB      "[!] Could not allocate extended memory",LNE
Machine         DB      "[!] 2MB of memory is required",LNE
KernelFile      DB      "[!] The kernel image appears corrupted.",LNE
FileIO_Error    DB      "[!] Error opening or reading from kernel image",LNE
MoveError       DB      "[!] Error copying from conventional to extended memory",LNE

;----------------------------
; Debug message strings

%macro ERROR 1
        mov     ah,9
        mov	dx,%1
        int	21h
        mov	ax,4CFFh
        int	21h
%endmacro

;Greedy macro parameters
%macro MESSAGE 1+
        pusha
        jmp     %%c

%%m:    DB      %1
%%c:    mov     ah,9
        mov     dx,%%m
        int     21h
        popa
%endmacro

%macro xms 1
        call far %1
%endmacro

Main:
        cld     ; Direction will be 0 for rest of program
        ;Clear screen with mode switch
        mov     ax,3
        int     10h

        ;Print welcome message
        mov	ah,9
        mov	dx,Weclome
        int	21h

        ;XMS present
        mov	ax,4300h
        int	2Fh
        cmp	al,80h
        je	Present

        ; NO XMS PRESENT: ERROR
        ERROR   MnoXMS
Present:
        ;Acquire XMS far pointer
        push    es
        mov	ax,4310h
        int	2Fh

        mov	bp,XMS
        mov	[bp],bx
        mov	[bp+2],es
        pop     es

        ;Query A20, is it already enabled
        mov     ah,7
        xms     [bp]
        cmp     al,0
        jz      EnableA20       ; Not already enabled
        jmp     A20AlreadyOn

EnableA20:
        ;Global enable A20 gate
        mov     ah,3
        xms     [bp]
        cmp     al,1
        je      A20Enabled

        ERROR   A20Error

A20AlreadyOn:
        MESSAGE "[i] A20 is already enabled",LNE
A20Enabled:
        MESSAGE "[i] Aquiring the high memory area",LNE
        mov     ah,1
        mov     dx,0FFFFh
        xms     [bp]
        cmp     al,1
        je      HMA_OK
        ERROR   HMA_Error
HMA_OK:
        ;How much extended memory - HMA
        ;Other functions will be used to get
        ;a more precise reading of extended memory
        ;for the purposes of the kernel
        mov     ah,8
        xms     [bp]

        ;Allocate all available extended memory
        ;It will be at most 64M like the ISA memory hole
        ;either way this is enough for the kernel
        mov     dx,ax
        mov     ah,9
        xms     [bp]
        cmp     al,1
        je      ExtAllocSuccess
        ERROR   ExtMemErr

ExtAllocSuccess:
        ;Save EMB handle to extended move struct
        mov     [XMM.deshan],dx

        ;Lock the EMB, this ensures it does not move
        mov     ah,0Ch
        xms     [bp]

        ;Address of the EMB in DX:BX
        mov     ax,bx
        mov     cx,dx

        ;Align CX:AX to page boundary
        add     ax,4095
        adc     cx,0
        and     ax,~4095

        ; Store the aligned address, I will use it later
        mov     [KernelAddr],ax
        mov     [KernelAddr+2],cx

        ;Aligned minus Original is the offset
        ;in order to load at a page boundary
        sub     cx,dx
        sbb     ax,bx


        ;CX:AX is the offset to move to EMB
        ;Copy it to memory move structure
        mov     [XMM.desoff],ax
        mov     [XMM.desoff+2],cx

LoadKernel:
        ;Open kernel file
        mov     ax,OPEN_RO
        mov     dx,Path
        int     21h

        jnc     .OpenGood
        ERROR   FileIO_Error
.OpenGood:

        ;Handle is in AX, it will not be clobbered in BX
        mov     bx,ax

        ;Seek to end to get size
        mov     ax,SEEK_END
        xor     cx,cx
        xor     dx,dx
        int     21h

        ;File byte size in DX:AX
        ;The kernel image is page granular
        ;aka it is in 4096 byte blocks
        shrd    ax,dx,PAGE_SHIFT

        ;AX now contains the page count, there is no
        ;way it does not fit in a 16-bit register (256M)
        test    ax,ax   ;Kernel should not be zero pages :)
        je      Corrupted
        mov     di,ax   ;DI will be the loop counter

        ;Seek back
        mov     ax,SEEK_SET
        xor     cx,cx
        xor     dx,dx
        int     21h

        ;Set conventional memory pointer
        mov     word[XMM.srcoff],Buffer
        mov     [XMM.srcoff+2],ds

        ;BX remains the file pointer
        ;DI is the loop counter
        ;SI is a pointer to the XMM structure
        mov     si,XMM

        ;I am out of registers, so I will move 4096
        ;manually wherever needed
.loadloop:
        ;Read 4096 bytes into buffer
        mov     ah,READ
        mov     cx,4096
        mov     dx,Buffer
        int     21h
        jc      Corrupted

        ;Copy to extended memory
        ;HIMEM seems to zero BL on success
        push    bx
        mov     ah,0Bh
        xms     [bp]
        cmp     al,1
        je      .copy_success
        ERROR   MoveError
.copy_success:
        pop     bx


        ;Add 4096 to the extended move offset
        add     dword [XMM.desoff],4096

        dec     di      ; This will not work for only one page
        jnz     .loadloop

.end:
        ;Close the file, this will flush all buffers
        mov     ah,CLOSE
        int     21h

PageSetup:
        ; We will make the HMA look like this
        ; 100000h:
        ;       Page directory
        ; 101000h:
        ;       Page table for low memory
        ; 102000h:
        ;       Page table for kernel memory
        ;
        ; We also want the 1M section to be marked as ring 3
        ; so that virtual 8086 mode and the kernel can access it.

        MESSAGE "[i] Creating page tables",LNE
        push    es
        mov     ax,0FFFFh
        mov     es,ax

        mov     bx,16

        ;Create the page directory
        mov     dword [es:bx],       (101h<<PAGE_SHIFT)|7h
        mov     dword [es:bx+768*4], (102h<<PAGE_SHIFT)|3h

        ;Copy the IDMAP page table to HMA
        mov     cx,256
        mov     si,IDMap
        lea     di,[bx+4096]
        rep     movsd

        lea     di,[bx+8192]
        ;DI points to next page table
        ;Map the kernel to high memory
        ;Note, one nibble will be zero but does not count as
        ;part of the address
        mov     eax,[KernelAddr]
        and     eax,0FFFFF000h
        mov     al,3h   ;Page attributes
        ;Loop: Copy eax into second page table, page ++
        mov     cx,256
.hh:
        stosd
        add     eax,1<<PAGE_SHIFT
        loop    .hh

        ;CR3 lower bits are reserved, best to not touch them
        mov     eax,cr3
        and     eax,~(0FFFh)
        or      eax,100000h
        mov     cr3,eax

AllocKernelReserved:
        ;COM programs get all of the remaining contiguous conventional
        ;memory. This is bad. We must reallocate so that protected mode
        ;software can allocate conventional memory. Now that we no longer
        ;need the extra memory, it is safe to do this
        push    cs
        pop     es
        mov     ah,SETBLOCK
        mov     bx,4096/16      ; Kernel gets 4096 bytes, bootloader is trashed
        int     21h
        mov     [SavedProgBase],es

GotoKernel:
        cli
        ;Figure out the address of the GDT
        mov     eax,ds
        shl     eax,4
        add     eax,_GDT
        mov     [_GDTR+2],eax        ;Put it in the new GDTR

        ;GDT is loaded segment relative
        lgdt    [_GDTR]

        movzx   edx,word[SavedProgBase]

        ;Switch to protected mode
        mov     eax,cr0
        or      eax,8000_0001h
        mov     cr0,eax
        jmp     $+2     ; Clear prefetch cache so CPU does not crash

        ;Bravo six, going dark
        mov     ax,10h
        mov     ds,ax
        mov     es,ax
        mov     ss,ax

        jmp     dword 8:0C000_0001h  ;Yes, this is a thing
        ;Jumps over the protective RET
        ;See StartK.asm for more information

Corrupted:
        ERROR   KernelFile

;#############################
;############Data#############
;#############################

Path:   DB      "\OS90\KERNL386.EXE",0
SavedProgBase: DW 0
KernelAddr:
        DD      0

_GDTR:
        DW      _GDT_end - _GDT - 1
        DD      0       ;Will figure out
_GDT:
        DQ	0
        ; Flat code segment
        DB      0FFh,0FFh,0,0,0,1_00_11010b,11_001111b,0
        ; Flat data segment
        DB      0FFh,0FFh,0,0,0,1_00_10010b,11_001111b,0
_GDT_end:

XMS:    DD      0
Handle: DW      0

XMM:    ;Extended memory move
.len:   DD      4096
.srchan:DW      0       ;Must be zero so that srcoff is a seg:off pair
.srcoff:DD      0       ;Figured out later
.deshan:DW      0
.desoff:DD      0

;Page tables are zeroed before this is copied in
;Note the different page flags here. User pages are required
;for V86
IDMap:
%assign i 0
%rep 256
        DD      (i << PAGE_SHIFT) | 7h
        %assign i i+1
%endrep

; COM programs get 64K, so this should be safe
Buffer:
