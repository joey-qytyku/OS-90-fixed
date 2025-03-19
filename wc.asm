Open Watcom build environment ( version=0)
Module: main
GROUP: 'DGROUP' CONST,CONST2,_DATA

Segment: _TEXT PARA USE32 00000058 bytes
0000				convert_:
0000  53				push		ebx
0001  51				push		ecx
0002  56				push		esi
0003  57				push		edi
0004  55				push		ebp
0005  89 C3				mov		ebx,eax
0007  85 D2				test		edx,edx
0009  7C 47				jl		L$3
000B  89 D1				mov		ecx,edx
000D				L$1:
000D  85 D2				test		edx,edx
000F  0F 9C C2				setl		dl
0012  BD 0A 00 00 00			mov		ebp,0x0000000a
0017  89 D7				mov		edi,edx
0019  31 F6				xor		esi,esi
001B  81 E7 FF 00 00 00			and		edi,0x000000ff
0021  8D 80 00 00 00 00			lea		eax,[eax]
0027  8D 92 00 00 00 00			lea		edx,[edx]
002D  8D 40 00				lea		eax,[eax]
0030				L$2:
0030  89 C8				mov		eax,ecx
0032  31 D2				xor		edx,edx
0034  F7 F5				div		ebp
0036  83 C2 30				add		edx,0x00000030
0039  89 C8				mov		eax,ecx
003B  88 53 FF				mov		-0x1[ebx],dl
003E  31 D2				xor		edx,edx
0040  F7 F5				div		ebp
0042  4B				dec		ebx
0043  46				inc		esi
0044  89 C1				mov		ecx,eax
0046  39 EE				cmp		esi,ebp
0048  72 E6				jb		L$2
004A  89 F8				mov		eax,edi
004C  5D				pop		ebp
004D  5F				pop		edi
004E  5E				pop		esi
004F  59				pop		ecx
0050  5B				pop		ebx
0051  C3				ret
0052				L$3:
0052  89 D1				mov		ecx,edx
0054  F7 D9				neg		ecx
0056  EB B5				jmp		L$1

Routine Size: 88 bytes,    Routine Base: _TEXT + 0000

No disassembly errors

Segment: CONST DWORD USE32 00000000 bytes

Segment: CONST2 DWORD USE32 00000000 bytes

Segment: _DATA DWORD USE32 00000000 bytes

