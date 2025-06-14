# This is only for testing.

CC=~/Desktop/djgpp/bin/i586-pc-msdosdjgpp-gcc
CFLAGS=-O2 -pg -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wold-style-cast -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wundef -Wno-unused -Wno-parentheses -fdiagnostics-show-option -Wconversion -Wrestrict -mgeneral-regs-only

djgpp:
	$(CC) $(CFLAGS) printf_tester.c
	# $(CC) $(CFLAGS) printf_tester.c -c -o sizetest.o
	dosbox-x .

intconv: intconv.asm intconv_test.c
	nasm -Ox -fcoff intconv.asm -o intconv.o
	~/Desktop/djgpp/bin/i586-pc-msdosdjgpp-gcc intconv_test.c -c -o intconv_test.o
	~/Desktop/djgpp/bin/i586-pc-msdosdjgpp-gcc intconv_test.o intconv.o -o intconv
	dosbox-x .
