CC=~/Desktop/djgpp/bin/i586-pc-msdosdjgpp-gcc
CFLAGS=-O0 -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wold-style-cast -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=1 -Wundef -Wno-unused -Wno-parentheses -fdiagnostics-show-option -Wconversion -Wrestrict

djgpp:
	$(CC) $(CFLAGS) printf_tester.c
	$(CC) $(CFLAGS) printf_tester.c -c -o sizetest.o
	dosbox-x .

ia16:
	docker exec -it ia16 ia16-elf-gcc -Os $(subst $(shell realpath ~),/mnt,$(shell realpath printf_tester.c) ) -o /mnt/b.exe
	dosbox-x ~
