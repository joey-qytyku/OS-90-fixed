%ifndef MACRO_INC
%define MACRO_INC

; This is the header that includes nasmx.inc
; It creates aliases to all of its features to avoid the repetative
; NASMX_... naming.

%ifdef __NASMX_INCLUDED__
%error "NASMX Should be included by Macro.inc"
%endif

%include "nasmx.inc"

; We cannot undefine struc and endstruc because they are internal
; so we just use STRUCT/ENDSTRUCT/ON

%defalias ON              NASMX_AT
%defalias ENDSTRUCT       NASMX_ENDSTRUC
%defalias ENDUNION        NASMX_ENDUNION
%defalias IENDSTRUCT      NASMX_IENDSTRUC
%defalias ISTRUCT         NASMX_ISTRUC
%defalias PRAGMA          NASMX_PRAGMA
%defalias RES             NASMX_RESERVE
%defalias STRUCT          NASMX_STRUC
%defalias TCHAR           NASMX_TCHAR
%defalias TEXT            NASMX_TEXT
%defalias UNION           NASMX_UNION

; Alias the NASMX number types with OS/90 definitions

%defalias I8 int8_t
%defalias U8 uint8_t
%defalias I16 int16_t
%defalias U16 uint16_t
%defalias I32 int32_t
%defalias U32 uint32_t
%defalias I64 int64_t
%defalias U64 uint64_t

%endif
