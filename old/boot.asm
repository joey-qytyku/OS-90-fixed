; OBSOLETE! ;


err_alloc = 'A'

bootinfo	STRUCT
	bootdsk	BYTE	?
	memconv	WORD	?
	memextn	WORD	? ; Number of pages

	apmpres	BYTE	?
	cs32	WORD	? ; Real mode segment of APM
	ip32	DWORD	?; Offset to the APM interface

	cs16	WORD	?
	ip16	WORD	?
	apmds	WORD	? ; Data segment base, 16/32



	cslen	DWORD	?; Length16:Length32
	dslen	WORD	?
	free4k	WORD	? ; Segment of the allocated data

bootinfo	ENDS

dos_fcb	STRUCT
	drive	BYTE	?
	fname	QWORD	?
	fext	BYTE	?,?,?
	osfsize	DWORD	?
	osdate	WORD	?
	ostime	WORD	?
	resv	QWORD	?
	currec	BYTE	?
	rndrec	DWORD	?
main:
	mov	ax, @DATA
	mov	ds, ax

	; Allocate free 4K for SMP trampoline
	; This MUST be successfully allocated
	mov	ah, 48h
	mov	bx, 256
	int	21h
	jnc	successful_alloc
	jmp	error

successful_alloc:
	; Save it
	mov	[bi+bootinfo.free4k], ax

setup_apm:
	; APM installation check
	mov	ax, 5300h
	xor	bx, bx
	int	15h
	jc	noapm

	; Disconnect APM
	mov	ax, 5304h
	xor	bx, bx
	int	15h
	jc	error

	; Connect to 32-bit protected mode interface
	mov	ax, 5303h
	xor	bx, bx
	int	15h
	setnc	bi.apmpres
	jc	noapm

	; Load the values
	mov	[bi+bootinfo.cs32], ax
	mov	[bi+bootinfo.ip32], ebx
	mov	[bi+bootinfo.cs16], cx
	mov	[bi+bootinfo.dseg], dx
	mov	[bi+bootinfo.cslen], esi
	mov	[bi.dslen], di
	; No need for 16-bit offset because I am not calling it with that
noapm:

	; Enable A20 gate through the keyboard

	sti

	; Get drive letter
	mov	ah, 19h ; Goes in AL as a number
	int	21h
	mov	bi+bootinfo.bootdsk, al

	; FCB requires drive letter
	mov	[kernel_fcb+dos_fcb.drive], al

	int	12h
	mov	bi.memconv, ax

	; Get extended memory size (including HMA)
	mov	ah, 88h
	int	15h

	; AX=1K blocks, change to pages for convenience
	shl	ax, 2
	mov	[bi.memextn], ax

	; Open the kernel FCB
	mov	ah, 0Fh
	mov	dx, OFFSET kernel_fcb
	int	21h

	; Allocate a 4K buffer for the kernel
	mov	ah, 48h
	mov	bx, 4096
	int	21h

	; Enter unreal mode to extend GS
	; First, I must get the linear address of the GDT
	mov	ax, @DATA
	shr	eax, 4
	add	eax, OFFSET gdt
	mov	[gdtr+2], eax
	lgdt	[]

	; Kernel is copied page-by-page (4k)

; The algo
	; Read 4k
	; Increase file offset 4K
	; Cannot seek anymore? We are done
; The end


	; Deallocate the buffer once it is loaded


	;######################
	;# Configure the FPU  #
	;######################
	cli
	; Lazy switching is not used by the OS
	; By default, the FPU registers are saved by
	; a sperate interrupt because the TSS does not store them
	; (I think). Sending a speparate interrupt seems unoptimal
	clts
	;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Not the carry rotate
	; Can I use rotation and decrement to turn of bits?
	;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	; Turn off floating point emulation and enable native exceptions
	mov	eax, cr0
	mov	ebx, eax
	or	bl, 1<<2
	not	ebx
	and	eax, ebx	; Bit 2 cleared
	or	al, 1<<5	; Set bit 5
	mov	cr0, eax

	; Catch divide by zero and invalid operands
	fldcw	[fpu_def]

	; Remap the PIC to int 32/20h
remap_pic:
	; Send ICW1
	mov	al, 13h
	out	20h, al
	out	0A0h, al

	; ICW2 is the vector
	mov	al, 20h
	out	21h, al
	mov	al, 30h
	out	0A1h, al

	; ICW3 for master: irq2 has cascade
	mov	al, 4h
	out	21h, al

	; ICW3 for slave: master IRQ number
	mov	al, 2
	out	0A1h, al

	; ICW4 for master, x86 mode, normal EOI, master mode
	mov	al, 0Dh
	out	21h, al
	; ICW4 for slave, .., slave buffered mode
	mov	al, 9h
	out	0A1h, al

getinfo:
	; Get 32-bit address of the boot information
	xor	edx, edx
	mov	ax, ds
	shl	eax, 4
	add	eax, OFFSET bi
	jmp	0xFFFF,16

write_8042:
	; AL=Data, DX=Port
	push	ax
l:
	in	al, 64h
	bt	al, 1
	jc	SHORT PTR l
	pop	ax
	out	64h, al
	ret

read_60h_once:
	in	al, 64h
	shr	al, 1
	jnc	read_60h_once
	ret

error:	mov	ah, 9h
	mov	dx, LENGTHOF error_msg
	int	21h
	mov	ax, 4CFFh
	int	21h

CODE	ENDS

DATA	SEGMENT

error_msg	DB	"BOOTLOADER ERROR!",10,13
kernel_name DB	"KERNEL.BIN", 0

bi	DB	0 DUP(SIZEOF bootinfo)

fpu_def	DW	37A

kernel_fcb	DB	0 DUP(SIZEOF dos_fcb)

	; Alignment is recommended by Intel(R)
	ALIGN	8
	; Null segment
gdt:	DQ	0
	; 4G data segment
	DQ	0x008F92000000FFFF
gdt_end:
gdtr:
	DW	gdt_end - gdt - 1
	DD	gdt_end


; When in real mode, DPL seems to have no effect
; Tested on qemu-system-i386
DATA	ENDS

END
