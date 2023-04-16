%ifndef KERNEL_INC
%define KERNEL_INC

STRUC TF
    ._eax:   RESD 1
    ._ebx:   RESD 1
    ._ecx:   RESD 1
    ._edx:   RESD 1
    ._esi:   RESD 1
    ._edi:   RESD 1
    ._ebp:   RESD 1
    .__esp:  RESD 1

    ._eip:   RESD 1
    ._cs:    RESD 1
    ._eflags:RESD 1

    ._esp:   RESD 1
    ._ss:    RESD 1

    ._es:    RESD 1
    ._ds:    RESD 1
    ._fs:    RESD 1
    ._gs:    RESD 1

    ._size:  RESD 1
ENDSTRUC

%endif

%define KERNEL_DSEC section .data
%define KERNEL_CSEC section .text

%define END_KERNEL_DSEC
%define END_KERNEL_CSEC


%macro retif 1
        j%-1    %%Skip
        ret
  %%Skip:
%endmacro

%macro callif 2
        j%-1 %%Skip
        call    %2
%%Skip:
%endmacro

%macro pushm 1-*
        %rep %0
        push %1
        %rotate -1
        %endrep
%endmacro

%macro popm 1-*
        %rep %0
        push %1
        %rotate 1
        %endrep
%endmacro

%macro CInvoke 1-*

        %rotate -1
        %rep %0-1
            %ifstr %1
                jmp %%OverString
                %%TehString: DB  %1
                %%OverString:
                push    %%TehString
            %else
                push    %1
            %endif
        %endrep
        call    %1
        sub     esp, (%0-1)*4
%endmacro

%macro PROC 1-*
        %assign _PARAMS %0

        %rotate -1

        %rep %0-1
        %endrep
%endmacro
