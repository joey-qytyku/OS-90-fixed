; Load kernel.bin in low memory
; At least 384K of memory must be free, or 6 segments
; All conventional memory is reserved when this happens
; Because that is what COM programs do when they run

	org	100h
	mem	EQU 60000h

main:
	;------------------------------------------------
	; DOS memory allocations are not page aligned
	; This means that the resulting allocation block
	; address will likely need to be aligned
	; This is done with <<8, that means at most
	; 256 bytes may be unused
	; So the required memory is actually mem+256
	;------------------------------------------------

	mov	ah,9
	mov	dx,WelcomeMSG
	int	21h

	mov	ah,7
	int	21h

	cmp	al,' '
	jnz	Cancel

	; Allocate the memory
	mov	ah,48h
	mov	bx,(mem + 256/16) >> 4
	jc	MemError

	; AX=Segment, now align it to 4096
	mov	bx,~255
	add	ax,bx
	and	ax,bx
	mov	[LdSeg],ax
	; Save the aligned segment

	; Open file as read only
	mov	ah,3Dh
	mov	al,0
	mov	dx,KrnlPath
	; BX=File handle, never clobbered

	; Seek to end
	mov	ax,4202h
	xor	cx,cx
	mov	dx,cx
	int	21h

	; DX:AX is the size of the file
	; Convert to a number of paragraphs
	shrd	ax,dx,4
	; AX=Number of paragraphs to copy
	; DX=Definetly zero
	push	ax
	; Seek back to begining
	mov	ax,4200h
	xor	cx,cx
	mov	dx,cx
	int	21h
	pop	ax
	; AX=Paragrpahs

	mov	si,[LdSeg]
	mov	di,ax
	push	ds
L:
	; Read 16 bytes to [segment]
	mov	ds,si
	mov	ah,3Fh
	mov	cx,16
	int	21h

	; Seek forward by 16
	mov	ax,4201h
	mov	dx,cx
	xor	cx,cx
	int	21h

	; Increase segment
	inc	si

	dec	di
	jnz	L
	pop	ds
	jmp $
	; Close the file
	mov	ah,3Eh
	int	21h
MemError:
Cancel:
	ret

;---------------
; Data Section
WelcomeMSG: DB	"Press space to continue or ^C to cancel boot",10,13,'$'

MemMSG:	DB	"384 KB of free convention memory required",10,13,'$'



KrnlPath:	DB	"\ND\KERNL386.EXE"
LdSeg:	DW	0

;-------------
; Page tables

