%if 0
History:
2022-07-22
        Completed the driver
        - Nevermind -
        It turns out that all I have to do is get the default value of the
        interrupt vector and check if has changed from default.

%endif

;IRQ grabber for OS/90
;Records the modification of interrupt vectors.
;Must be loaded before ANYTHING else
;Small conventional memory footprint

;DOS has a change vector function and also allows vectors to be modified by software.
;If a driver sets a vector to implement an IRQ, the kernel must detect it and call
;the 16-bit driver when the IRQ is recieved.

GINT EQU 35h
SINT EQU 25h
BASE_MASTER EQU 8
BASE_SLAVE  EQU 70h

Header:
        DW      0
        DW      0
        DW      8000h       ; "Character device"
        DW      Strategy
        DW      Interrupt
        DB      "@#O9!_~2" ;Nonsense name so it cannot be called

EndOfDriverCode:


;This will be placed at the very end of conventional memory
;sometimes the last 1K is used for BIOS data, I am not sure
WherePut:
        DW      0
        DW      9FBCh
Master:
        DW      0
        DW      BASE_MASTER*4
Slave:
        DW      0
        DW      BASE_SLAVE*4

Strategy:
        retf

Error_Called:
        jmp $

Interrupt:
        ;According to the MS-DOS Programmer's Refference
        ;the INIT call can use 25H and 35H, which are GINT and SINT

        ;Is this driver being called for anything except INIT
        ;if so, there is an error
        cmp     al,0
        jnz     Error_Called

        ;ES:BX point to the request packet
        ;+14 is a SEG:OFF pointer to the end of the
        ;driver memory, allows for removal of dead code

        mov     word [es:bx+14],EndOfDriverCode
        mov     word [es:bx+18],cs

        mov     bx,Master

        lds     si,[bx]
        les     di,[WherePut]
        movsd
        add     di,32
        lds     si,[bx+4]
        movsd

        retf
