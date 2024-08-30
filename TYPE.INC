; TODO
%assign __insetblock__ 0

%macro LONG 1+
	%assign __oldseg__ __?SECT?__

	%if %0 == 1
		segment _DATA align=4096 class='BSS' use32 combine
		%1:
		RESD 1
	%else
		segment _DATA align=4096 class='DATA' use32 combine
		%1:
		%rep %0
			%rotate 1
			DD %1
		%endrep
	%endif
%endmacro

segment _DATA align=4096 class='DATA' use32 public combine

mov esi, 0 wrt _DATA
jmp $

