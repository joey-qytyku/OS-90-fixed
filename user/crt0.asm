
_start:
    ; Get the program segment prefix of this program
    mov     ah,62h
    xor     bx,bx
    int     21h
    ; BX = segment
    shl     ebx,4

    ;Arguments are at

    call    main
