CC=~/Desktop/djgpp/bin/i586-pc-msdosdjgpp-gcc
CFLAGS=-O2 -pg -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wold-style-cast -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=1 -Wundef -Wno-unused -Wno-parentheses -fdiagnostics-show-option -Wconversion -Wrestrict

lib:

memcmp-test:
	$(CC) memcmp_test.c -c -o memcmp_test.o
	nasm -fcoff memcmp.asm -o memcmp.o
	$(CC) memcmp_test.o memcmp.o
	dosbox-x .

