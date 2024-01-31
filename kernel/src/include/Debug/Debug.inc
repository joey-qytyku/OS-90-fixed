%ifndef DEBUG_INC
%define DEBUG_INC

%include "Macro.inc"

%define ANSI_COLOR_RED     `\x1b[31m`
%define ANSI_COLOR_GREEN   `\x1b[32m`
%define ANSI_COLOR_YELLOW  `\x1b[33m`
%define ANSI_COLOR_BLUE    `\x1b[34m`
%define ANSI_COLOR_MAGENTA `\x1b[35m`
%define ANSI_COLOR_CYAN    `\x1b[36m`
%define ANSI_COLOR_RESET   `\x1b[0m`

IMPORT printf_

%macro ASSERT 3
IF %1,%2,%3
        ; If correct do nothing
ELSE
        INVOKE printf_, "Failed ASM assert at %s:%i\n", __FILE__, __LINE__
ENDIF
%endmacro

; Requires brackets for multiple parameters after format string
%macro KLogf 1-2+
        pusha
        %if %0 == 1
                %strcat fmt ANSI_COLOR_RED "%s:%i >>" %1 ANSI_COLOR_RESET
                INVOKE printf_, fmt, __FILE__, __LINE__
        %else
                %strcat fmt ANSI_COLOR_RED "%s:%i >>" %1 ANSI_COLOR_RESET
                INVOKE printf_, fmt, __FILE__, __LINE__, %2
        %endif
        popa

%endmacro

%endif

