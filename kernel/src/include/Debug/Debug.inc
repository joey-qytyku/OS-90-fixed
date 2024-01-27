%ifndef DEBUG_INC
%define DEBUG_INC

%include "nasmx.inc"

%define ANSI_COLOR_RED     1bh,"[31m"
%define ANSI_COLOR_GREEN   1bh,"[32m"
%define ANSI_COLOR_YELLOW  1bh,"[33m"
%define ANSI_COLOR_BLUE    1bh,"[34m"
%define ANSI_COLOR_MAGENTA 1bh,"[35m"
%define ANSI_COLOR_CYAN    1bh,"[36m"
%define ANSI_COLOR_RESET   1bh,"[0m"


%macro ASSERT 3
IF %0,%1,%2
        ; If correct do nothing
ELSE
        INVOKE printf, "Failed ASM assert at %s:%i\n", __FILE__, __LINE__
ENDIF
%endmacro

; Requires brackets for multiple parameters after format string
%macro KLogf 2
        INVOKE printf ANSI_COLOR_YELLOW "%s%s:%i>>" %0 ANSI_COLOR_RESET, __FILE__, __LINE__
%endmacro



%endif

