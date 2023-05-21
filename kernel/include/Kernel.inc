%ifndef KERNEL_INC
%define KERNEL_INC

CR_NT   EQU     01b
CR_NI   EQU     10b


STRUC IFRAME
._eax:  RESD 1
._ebx:  RESD 1
._ecx:  RESD 1
._edx:  RESD 1
._esi:  RESD 1
._edi:  RESD 1
._ebp:  RESD 1

._eip:   RESD 1
._cs:    RESD 1
._eflags:RESD 1

._esp:   RESD 1
._ss:    RESD 1

._dsegs:
._es:   RESD 1
._ds:   RESD 1
._fs:   RESD 1
._gs:   RESD 1
._size:
ENDSTRUC

STRUC RSAVE
       ; User Context
.u_dsegs:
.u_es:   RESD 1
.u_ds:   RESD 1
.u_fs:   RESD 1
.u_gs:   RESD 1

.u_eax:  RESD 1
.u_ebx:  RESD 1
.u_ecx:  RESD 1
.u_edx:  RESD 1
.u_esi:  RESD 1
.u_edi:  RESD 1
.u_ebp:  RESD 1

.u_eip:   RESD 1
.u_cs:    RESD 1
.u_eflags:RESD 1
.u_esp:   RESD 1
.u_ss:    RESD 1
.u_was_v86: RESD 1

    ; Kernel context
.k_eax:  RESD 1
.k_ebx:  RESD 1
.k_ecx:  RESD 1
.k_edx:  RESD 1
.k_esi:  RESD 1
.k_edi:  RESD 1
.k_ebp:  RESD 1

.k_esp:   RESD 1
.k_eflags:RESD 1
.k_eip:   RESD 1
ENDSTRUC

STRUCT DREGW
.ax:    RESW    1
.bx:    RESW    1
.cx:    RESW    1
.dx:    RESW    1
.si:    RESW    1
.di:    RESW    1
.bp:    RESW    1
.sp:    RESW    1

._es:    RESW    1
._ds:    RESW    1

._ip:    RESW    1
._cs:    RESW    1
._flags: RESW    1
._sp:    RESW    1
._ss:    RESW    1
ENDSTRUC

%endif

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

%macro Invoke 1-*

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
        add     esp, (%0-1)*4
%endmacro
