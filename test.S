.global main

.text

main:
        int $0

        mov     $60,%eax
        syscall
