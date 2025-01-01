	; Use nasm -fobj to assemble this file.

	segment CODE class=TGROUP

; Use nasm? Yes use nasm.
; I also do not need the far data thing.

; Saves all registers. Only changes flags.
; Returns handle in AX or -1 if failed.
; USE 32-bit OPERAND!
xms_alloc:
	push    edx

	mov     ah,89h
	mov     edx,[esp+4+2]
	call    far [xms]

	cmp     al,1
	jz      .good

	; Allocation fail is NOT a total fail condition. It depends.
	or      eax,-1
.good:
	mov     eax,edx
	pop     edx

	ret     4

xms_largest_block_size:
	clc
	jmp     __xms_cont
xms_last_byte_addr:
	stc
__xms_cont:
	push    ebx
	push    ecx
	push    edx

	mov     ah,88h
	call    far [xms]

	jnc     .end
	mov     eax,ecx

	.end:

	pop     edx
	pop     ecx
	pop     ebx
	ret

..start:
	mov     ax,DATA
	mov     ds,ax
	mov     es,ax

	; Check if XMS is available
	mov     ax,4300h
	int     2Fh

	cmp     al,80h
	jz      HasXMS

	jmp     ERROR
ERROR:
	mov     ah,9
	mov     dx,err_msg
	int     21h
	mov     ax,4C01h
	int     21h
HasXMS:
	cld     ; Clear direction flag. It will stay cleared.

	xor     ax,ax   ; Set ES to zero so we can access the IDT later.
	mov     es,ax   ; It is used to convey Base:Size information to krnl.

	call    xms_last_byte_addr
	cmp     eax,(1024*1024)*15

	ja      more_than_15M

	; In this case, we only allocate ONE block and place the info in B0:B1
	; and B2:B3, while B4:B5 and B6:B7 are both zeroed.

	mov     dword [es:0B2h*4],0
	mov     dword [es:0B3h*4],0

	call    xms_largest_block_size
	call    xms_alloc

	;

	jmp     continue1

more_than_15M:
	; Here, we allocate TWO blocks. The largest possible will be in
	; B2:B3.


continue1:

err_msg DB "[!] Failed to load. Ensure an XMS driver is loaded.$"
hello   DB "Loading OS/90", 10, 13
	times 80 DB '-'
	DB 10,13
xms     DD ?

	segment KRNL

kernel_data:
	include data.h
	align   16
kernel_end: