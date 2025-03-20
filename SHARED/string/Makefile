AR=~/Desktop/djgpp/bin/i586-pc-msdosdjgpp-ar

CC=~/Desktop/djgpp/bin/i586-pc-msdosdjgpp-gcc
CFLAGS=-O2 -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wold-style-cast -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=1 -Wundef -Wno-unused -Wno-parentheses -fdiagnostics-show-option -Wconversion -Wrestrict


TESTERS=$(shell find *_test.c)

TESTO=$(patsubst .c,.o, $(TESTERS))

# FUNCSO=$(patsubst)

# Why not compile the whole library and link it for testing

lib:
	@$(foreach X, $(wildcard *.asm), nasm -DTESTING -fcoff $(X); )
	$(CC) $(CFLAGS) *.c -c
	$(AR) rcs libstr.a *.o
	-rm *.o

test: lib
	$(CC) $(CFLAGS) tests/_test.c -o a.exe -L libstr.a
	dosbox-x .
