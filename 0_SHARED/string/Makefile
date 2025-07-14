AR=~/Desktop/djgpp/bin/i586-pc-msdosdjgpp-ar

include ../../make.cfg

TESTERS=$(shell find *_test.c)

TESTO=$(patsubst .c,.o, $(TESTERS))

# FUNCSO=$(patsubst)

# Why not compile the whole library and link it for testing

lib:
	@$(foreach X, $(wildcard *.asm), $(AS) $(AFLAGS) -felf $(X); )
	$(CC) $(CFLAGS) *.c -c
	$(AR) rcs libstr.a *.o
	-rm *.o

# Need to update this to compile with DJGPP instead.
test: lib
	$(CC_DOS) $(CFLAGS) tests/_test.c -o a.exe -L libstr.a
	dosbox-x .
