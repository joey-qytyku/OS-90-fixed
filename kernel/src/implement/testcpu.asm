;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                           ;;
;;                     Copyright (C) 2023, Joey Qytyku                       ;;
;;                                                                           ;;
;; This file is part of OS/90 and is published under the GNU General Public  ;;
;; License version 2. A copy of this license should be included with the     ;;
;; source code and can be found at <https://www.gnu.org/licenses/>.          ;;
;;                                                                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; To detect CPUID, we have to check if the CPUID flag is modifiable.
; If it is, we can use CPUID. The value of the bit itself is
; not relevant.
;
SupportCPUID:
        ; We will write the flags, so we must save it
        pushf
        pushf
        ; Flip the bit on the flags register on the stack
        xor     dword[esp],1<<21
        ; Save the current value to register
        mov     eax,[esp]
        ; Write the one on the stack to EFLAGS
        popf
        pushf
        pop     ebx
        ; Are they the same? If so, there is no CPUID
        cmp     eax,ebx
        setne   al      ; If NOT the same, set AL to 1 for SUPPORTED
        popf
        ret
