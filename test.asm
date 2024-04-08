start:
        mov     ax,40h
        mov     es,ax

        mov     al,0Fh
        out     70h,al

        mov     al,0Ah
        out     71h,al

        mov     [es:67h],dword 7C00h + dothis
        ; mov     [es:72h],word 1234h

        jmp     0FFFFh:0

        jmp $

dothis:
        mov ah,0Eh
        mov al,'A'
        int 10h
        jmp $

times 510-($-$$) DB 0
DW 0xAA55