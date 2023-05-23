%include "Kernel.inc"

;===============================================================================
;                               E x p o r t s
;===============================================================================

        global  EnterCriticalRegion
        global  ExitCriticalRegion

        ;This requires memory fencing and atomic access.
        ;A special macro is used.

        global  _wPreemptCount

;===============================================================================
;                                  C o d e
;===============================================================================

;
; This function disables interrupts while it is running because it will
; break if reentered by another kernel thread and the atomicity of the
; increments will be jeopardized.

;
; EXCEPTION NO-OP!
;
EnterCriticalRegion:
        pushf
        cli

        mov     eax,[esp+4]
        test    al,CR_NI
        jz      .Skip0                  ; Skip if not set
        inc     word [wIntsOffCount]
.Skip0:
        test    al,CR_NT
        jz      .Skip1
        inc     word [_wPreemptCount]
.Skip1:
        cmp     word [wIntsOffCount],1
        jne      .IntsUnaffected

        popf
        cli
        ret
.IntsUnaffected:
        popf
        ret

ExitCriticalRegion:
        pushf
        cli

        mov     eax,[esp+4]
        test    al,CR_NI
        jz      .Skip0
        dec     word [wIntsOffCount]
.Skip0:
        test    al,CR_NT
        jz      .Skip1
        dec     word [_wPreemptCount]
.Skip1:
        popf
        ret

;===============================================================================
;                         U n i n i t i a l i z e d
;===============================================================================

        section .bss

        align   2
_wPreemptCount:
        RESW    1

wIntsOffCount:
        RESW    1
