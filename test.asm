%macro debug 1-*
	%rep %0
		%rotate -1
		%ifstr %1
			section .data
				%%fmt: DB %1,0
			section .text
		%else
			push %%fmt
		%endif
	%endrep
	call	printf
%endmacro

debug "%i %i", 5, 4
