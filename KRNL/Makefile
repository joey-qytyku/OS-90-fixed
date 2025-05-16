#############################################################################
##                     Copyright (C) 2022-2025, Joey Qytyku                ##
##                                                                         ##
## This file is part of OS/90.                                             ##
##                                                                         ##
## OS/90 is free software. You may distribute and/or modify it under       ##
## the terms of the GNU General Public License as published by the         ##
## Free Software Foundation, either version two of the license or a later  ##
## version if you chose.                                                   ##
##                                                                         ##
## A copy of this license should be included with OS/90.                   ##
## If not, it can be found at <https:##www.gnu.org/licenses/>              ##
#############################################################################

include ../make.cfg

CC := i686-elf-gcc

COMMON_FLAGS += -D EXCEPTIONS_SUPPORTED=20 -D__KRNL__

WARNINGS := -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=1 -Wundef -Wno-unused -Wno-parentheses -fdiagnostics-show-option -Wconversion -Wrestrict -Wstrict-aliasing=2

CFLAGS += $(COMMON_FLAGS) -mgeneral-regs-only -O2 -ffreestanding -nostdlib -nodefaultlibs -nostartfiles $(WARNINGS) -include "type.h" -flimit-function-alignment

# GCC supports the T option for linking. We HAVE to use this in the event
# of a libgcc call even in a freestanding environment. BTW libgcc is not
# that big. Most features are not used very often and are not linked.

all: clean
	$(CC) $(CFLAGS) -c *.c

	for x in *.asm; do nasm -felf $$x; done;

	$(CC) $(CFLAGS) *.o -Wl,-u,EntryPoint,--oformat=binary,-T,1_link.ld,-L,../SHARED/string/libstr.a -o KERNEL.BIN

clean:
	-rm *.o

install-unix: clean
	mcopy KERNEL.BIN C:/OS90/kernel.bin

uninstall-unix: clean
	rm KERNEL.BIN
	mdel c:/OS90/KERNEL.BIN

install-dos: clean
	mv KERNEL.BIN /OS90/

uninstall-dos: clean
	rm KERNEL.BIN
