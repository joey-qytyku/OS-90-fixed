enA20:
        cli
        mov	si,64h
        mov	di,60h
        mov	bp,Write8042

        ;Command: Disable keyboard
        mov	al,0ADh
        mov	dx,si	; 64h
        call	bp	;Write8042

        ;Command: read output port register
        mov	al,0D0h
        mov	dx,si	;64h
        call	bp	;Write8042

        ;Now in 60h, set bit 1
        call	Read60hOnce
        or	al,2
        push	ax

        ;Command: write to output port
        mov	al,0D1h
        mov	dx,si
        call	bp	;Write8042
        pop	ax	;Get ouptput port byte back

        ;Send it to 60h
        mov	dx,di
        call	bp	;Write8042

        ;Command: re-enable the keyboard
        mov	al,0AEh
        mov	dx,64h
        call	bp	;Write8042
        sti
        ret


Write8042:
        ; AL=Data DX=Port
        push	ax
.l:	; OBF is checked for both 64h and 60h
        in	al,64h
        shr	al,2
        jc	.l
        pop	ax
        out	dx,al
        ret

Read60hOnce:
        in	al,64h
        shr	al,1
        jnc	Read60hOnce
        ret
