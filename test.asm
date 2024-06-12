
mov al,17h
out 70h,al
in al,71h
mov ah,al

mov al,18h
out 70h,al
in al,71h

jmp $

times 510-($-$$) DB 0
DB 55h, 0AAh