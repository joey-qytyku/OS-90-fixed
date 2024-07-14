        .386
        .model flat, stdcall

        public  memcpy

        .CODE
        align  64
memcpy:
        push    esi
        push    edi
        cld

        mov      edi,[esp+8+(8)]
        mov      esi,[esp+8+(12)]
        mov      edx,[esp+8+(16)]

        cmp      edx,32
        jbe      small_block

        mov      ecx,edx
        shr      ecx,2
        rep      movsd

        ; Copy remaining bytes
        mov      ecx,edx
        and      ecx,11b
        rep      movsb
        jmp      done

small_block:
        mov      ecx,edx
        rep      movsb
done:
        pop      edi
        pop      esi
        ret      12

        align  64
memcmp:

        END
