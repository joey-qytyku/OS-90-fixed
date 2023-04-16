;---------------------------------------------
; Loader for kuDOS, runs in MS-DOS
; Can be loadfixed. XMS should be disabled.
;---------------------------------------------
; The kernel is loaded "cold turkey", it must
; handle information gathering on its own
	; Loads higher half kernel at physical address 110000h
SEEK_SET	equ	0
SEEK_CUR	equ	1
SEEK_END	equ	2
LOAD	equ	110000h

main:
.check_8086:
	; Set invalid opcode exception, this exist in 80186+
	mov	ax,2506h
	mov	dx,error
	int	21h
	; 186 was never used for personal computers
	; If an 80186 instruction is supported, CPU >= 80286

	; Size prefix overrides do not work on the i286
	; If this fails, then this is a 286
	mov	eax,4300h

	; XMS equires at least a 286, so checking after CPU
	int	2Fh
	cmp	al,80h
	je	error

	call	enA20
	call	load_kernel

	jmp $

	; Go to kernel
goto_kernel:
	cli
	lgdt	[temp_gdt]

	mov	ax,cs
	movzx	eax,ax
	shl	eax,4
	add	eax,init_pd
	mov	cr3,eax

	mov	eax,cr0
	or	eax,80000001h
	mov	cr0,eax
	; *Hacker voice* I'm in

	mov	ax,10h
	mov	ds,ax
	mov	ss,ax
	mov	es,ax

	mov	eax,0C0000000h
	jmp	eax

error:
	mov	ah,9
	mov	dx,.msg
	int	21h	; Print error message
	int	20h
msg:	DB	"Error loading kuDOS",10,13,'$'

;----------------|Bootstrap routines|----------------

load_kernel:
	; Set the 24-bit address in the destination GDT
	mov	eax,0FF000000h
	mov	bx,cs
	movzx	ebx,bx	; Don't know what EBX[16:32] is
	shl	ebx,4
	add	ebx,buffer
	or	eax,ebx
	mov	[bios_gdt.srcptr],eax

	xor	eax,eax
	xor	ebx,ebx

	mov	ah,3Dh
	mov	al,0
	mov	dx,kernel
	int	21h
	jc	error
	; AX now has file handle

	; Get size of the kernel image
	mov	bx,ax
	mov	ax,4202h
	xor	cx,cx
	mov	dx,cx
	int	21h
	jc	error
	; DX:AX contains new position

	; Convert from bytes to number of pages to read
	shrd	ax,dx,12

	; AX=Pages, DX=Definetly zero

	; BIOS and DOS call utilized here to not use DI
	; DI will serve as a loop counter
	mov	di,ax
	; Seek back to start, BX remains the file handle
	mov	ax,4200h
	xor	cx,cx
	mov	dx,cx
	int	21h

	; BX=File handle
	; DI=Loop counter

	; The loading process:
	;------------------------------------------------
	; The algorithm:
	; If current seek equals file size, return
	; * Read a page
	; * Copy to extended memory using INT 15H
	; * Increase extended memory pointer by 4096
	;------------------------------------------------

	; ! Seeking past the file does not generate an error

	; Issues:
	; Last data in buffer after loop is the first block
	; of the kernel image
.load_loop:
	; Read 4096 bytes from kernel image
	mov	ah,3Fh
	mov	cx,4096
	mov	dx,buffer
	int	21h

	; Copy the buffer into extended memory
	mov	ah,87h
	mov	cx,4096
	mov	si,bios_gdt
	int	15h
	jc	error

	mov	ax,4201h ; SEEK_CUR
	xor	cx,cx
	mov	dx,4096
	int	21h

	; Increase extended memory pointer by 4096
	add	word [bios_gdt.dstptr],4096
	adc	word [bios_gdt.dstptr+2],0

	dec	di
	jnz	.load_loop


	mov	bx,buffer
	jmp $
.end:
	mov	ah,9
	mov	dx,.msg
	int	21h
	ret

.msg:	DB	"Kernel Loaded",10,13,'$'

enA20:	cli
	mov	si,64h
	mov	di,60h

	; Command: Disable keyboard
	mov	al,0ADh
	mov	dx,si ; 64h
	call	write_8042

	; Command: read output port register
	mov	al,0D0h
	mov	dx,si ; 64h
	call	write_8042

	; Now in 60h, set bit 1
	call	read_60h_once
	or	al,2
	push	ax

	; Command: write to output port
	mov	al,0D1h
	mov	dx,si
	call	write_8042
	pop	ax	; Get ouptput port byte back

	; Send it to 60h
	mov	dx,di
	call	write_8042

	; Command: re-enable the keyboard
	mov	al,0AEh
	mov	dx,64h
	call	write_8042
	sti
	ret

write_8042:
	; AL=Data, DX=Port
	push	ax
.l:
	in	al,64h
	shr	al,2
	jc	.l
	pop	ax
	out	64h,al
	ret

read_60h_once:
	in	al,64h
	shr	al,1
	jnc	read_60h_once
	ret

;-----------------Data-----------------

%assign i_pt0 0
	; IDK the physical addresses of the page tables
	; they are computed with the segment registers
init_pd:	ALIGN	4096
times 1024	DD	0

; This page table is attached to entry 768 of the page directory
init_pt0:	ALIGN	4096
%rep 1024
	DD	19 | (LOAD + (i_pt0<<12))
	%assign i_pt0 i_pt0+1
%endrep

init_pt1:
	ALIGN 4096
init_pt2:

;---------------------------
; For loading the kernel
buffer: 	times 4096 DB 0

	; Kernel name, prefix required
kernel:	DB	"\nd\kernel.bin",0

file_size:	DD	0

bios_gdt:
	; Null segment
	DQ	0
	; Data section with GDT, set to zero
	DQ	0
	; Source GDT

	DB	0FFh	; Limit
.srcptr:	DB	0FFh,0,0,0  ; 24-bit Address + limit
	DB	93h	; Access rights
	DW	0	; Reserved in 286

	; Destination GDT
	DB	0FFh	; Limit
.dstptr:	DD	0FF110000h
	DB	93h	; Access rights
	DW	0FF0Fh

	; BIOS code, stack, temp user code and BIOS code
	DQ	0,0,0,0

temp_gdt:
	DQ	0
	DQ	0,0
