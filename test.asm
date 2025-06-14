table:
%assign i 0
%rep 1000
	db '0'+((i/100   ))
	db '0'+((i / 10) %% 10)
	db '0'+(i %% 10 ), 10
	%assign i i+1
%endrep
