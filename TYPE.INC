; TODO?
%assign __insetblock__ 0

%assign count 0

;
; Data declaration and assignment operators with support for
; segment rearangement.
;
; TODO: Add one for a block of standard-ly initialized items.
;

%macro __type_decl 2-*
	; %assign __oldseg__ __SECT__
	%deftok __DEF %strcat("D", %1)
	%deftok __RES %strcat("RES", %1)

	%if %0 == 1
		segment _DATA%+count align=4 class='BSS' use32 combine
		%2:
		__RES%+%1 1
	%else
		segment _DATA%+count align=4 class='DATA' use32 combine
		%2:
		; Ignore first arguments
		%rep %0 - 2
			%rotate 1
			__DEF %1
		%endrep
	%endif
	%assign count count+1
%endmacro

%macro LONG 1+
	__type_decl D, %1, {%2}
%endmacro

%macro SHORT 1+
	__type_decl W, %1, {%2}
%endmacro

%macro BYTE 1+
	__type_decl B, %1, {%2}
%endmacro

; BYTE x, 1

;
; Import and export tables to avoid annoying EXTERNs.
; ImportList
; 	Sym1
; 	Sym2
; EndImports
;

%macro ImportList 1
	; extern %
%endmacro

%macro EndImports 1
%endmacro

%macro ExportList 1
%endmacro

%macro EndExports 1
%endmacro

%define TEST(x)

;
; Use a macro to create another macro?
;
