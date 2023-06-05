%ifndef KERNEL_INC
%define KERNEL_INC

STRUC IFRAME
._eax:  RESd 1
._ebx:  RESd 1
._ecx:  RESd 1
._edx:  RESd 1
._esi:  RESd 1
._edi:  RESd 1
._ebp:  RESd 1

._eip:   RESd 1
._cs:    RESd 1
._eflags:RESd 1

._esp:   RESd 1
._ss:    RESd 1

._es:   RESd 1
._ds:   RESd 1
._fs:   RESd 1
._gs:   RESd 1
._size:
ENDSTRUC

STRUC PcbThreadStates
.u_es            RESd    1
.u_ds            RESd    1
.u_fs            RESd    1
.u_gs            RESd    1
.u_ss            RESd    1

.u_eax           RESd    1
.u_ebx           RESd    1
.u_ecx           RESd    1
.u_edx           RESd    1
.u_esi           RESd    1
.u_edi           RESd    1
.u_ebp           RESd    1
.u_esp           RESd    1
.u_eflags        RESd    1
.u_eip           RESd    1

.k_eax           RESd    1
.k_ebx           RESd    1
.k_ecx           RESd    1
.k_edx           RESd    1
.k_esi           RESd    1
.k_edi           RESd    1
.k_ebp           RESd    1
.k_esp           RESd    1
.k_eflags        RESd    1
.k_eip           RESd    1



ENDSTRUC


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

%endif