%ifndef STRUCTURES_INC
%define STRUCTURES_INC

%include "Macro.inc"
;===============================================================================
;                   S t r u c t u r e   d e f i n i t i o n s
;===============================================================================

;-------------------------------------------------------------------------------
;               U n i v e r s a l   R e g i s t e r   D u m p
;-------------------------------------------------------------------------------

        STRUCT  RD

RES _eax,       U32
RES _ebx,       U32
RES _ecx,       U32
RES _edx,       U32
RES _esi,       U32
RES _edi,       U32
RES _ebp,       U32
RES _pm_es,     U32
RES _pm_ds,     U32
RES _pm_fs,     U32
RES _pm_gs,     U32
RES _cs,        U32
RES _eip,       U32
RES _eflags,    U32
RES _esp,       U32
RES _ss,        U32
RES _es,        U32
RES _ds,        U32
RES _fs,        U32
RES _gs,        U32
RES size,       U32

        ENDSTRUCT

;-------------------------------------------------------------------------------
;                   P r o c e s s   C o n t r o l   B l o c k
;-------------------------------------------------------------------------------

        STRUCT PCB

RES rdRegisters,  U32, RD.size/4
RES pNext,        U32
RES pLast,        U32
RES lPSP,         U32
RES bThreadState, U8
RES bProgBitness, U8
RES _,            U16
RES pIntAction,   U32
RES pExceptAction,U32
RES pFakeIrqAction,U32
RES arlLocalIDT,U32

        ENDSTRUCT

;===============================================================================

;===============================================================================
; S y s t e m   E n t r y   P o i n t   A c t i o n s   I n f o r m a t i o n
;===============================================================================
;
; To reduce conditional branching, the PCB uses a polymorphic-dispatched
; jump pointer for handling events under different circumstances.
;
; Some SEP actions are composed with others. For example, a real mode fake IRQ
; and also a protected mode reflection to a local DOS INT vector both enter the
; same point so that they can do real mode stack emulation after preparing the
; process. There is no need for procedures since it never goes beyond 1 level.
;
; Each SEP action terminates by jumping to SXP.
;
;===============================================================================

; Maybe merge Intr_Trap?

%endif
