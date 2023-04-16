CC=gcc

all:
	$(CC) $(CFLAGS) exetool.c -o exetool
	nasm -felf32 test.asm -o test.o
	ld -T ulink.ld test.o -m i386pe -o in.exe
	./exetool stub.exe EntryPoint 110000 StackStart out.exe in.exe N
