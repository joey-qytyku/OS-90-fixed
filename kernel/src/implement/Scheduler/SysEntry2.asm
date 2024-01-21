;-------------------------------------------------------------------------------
; S y s t e m   E n t r y   I m p l e m e n t a t i o n   i n   A s s e m b l y
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;                               I m p o r t s

extern  System_Exit_Point,\
        _dwErrorCode

;                         E n d   o f   I m p o r t s
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;                               E q u a t e s

; Idea, maybe create the IDT in unscrambled format and have code that
; converts it to the proper format. That way you do not need to copy anything.

EV_INT equ 0

;                        E n d   o f   E q u a t e s
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;                           S t r u c t u r e s

;                    E n d   o f   S t r u c t u r e s
;-------------------------------------------------------------------------------

Int_Sep_Do_V86:
        ret

Int_Sep_Do_PM:
        ret

;-------------------------------------------------------------------------------
; INPUT:
;       EDX = Event code
;       EBP = Process control block of calling thread
;       ESP = Last value pushed to trap frame
;
System_Entry_Point:
        cmp     dl,EV_INT

.handle_EXC:

handle_INT:
        ; call    [edi+PCB.intsepdo]

        jmp     System_Exit_Point
