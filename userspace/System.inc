%define SYSCALL_VECTOR 80h

%macro SysInvoke 1-6
        mov     eax,%1
        %rotate -1

        %rep %0-1

        %ifstr %1
                jmp     %%OverString
        %%NewString:
        DB      %1
        %%OverString:
        %endif

        mov     eax,%1
        %rotate -1
        %endrep

        int     SYSCALL_VECTOR
%endmacro
