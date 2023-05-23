SHELL=/bin/bash

GCCVER := 10.1.0

TCHAIN := ~/opt/cross1/bin

#INCLUDE=~/gcc-$(GCCVER)-nolibc/i386-linux/lib/gcc/i386-linux/$(GCCVER)/include
CFLAGS := -I ./include/ -Os -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mpreferred-stack-boundary=2 -march=i386 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99 -Wextra

CC=$(TCHAIN)/i386-linux-gcc
LINK=$(TCHAIN)/i386-linux-ld

CSOURCE=$(wildcard *.c)
ASOURCE=$(wildcard *.asm)

OBJ=$(patsubst %.c,%.c.o,$(wildcard *.c)) $(patsubst %.asm,%.asm.o,$(wildcard *.asm))

AS=nasm
ASFLAGS=-felf -Ox -I include/Asm

# -Ox will enable multipass assembly for more optimized code

%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o build/$@

%.asm.o: %.asm
	$(AS) $(ASFLAGS) $< -o build/$@

KERNL386.EXE: $(OBJ)
	echo Linking object files: [$(OBJ)]
	$(LINK) --oformat binary -T Kernel.ld $(addprefix build/, $(OBJ)) -o build/KERNL386.EXE
	$(LINK) --oformat elf32-i386 -T Kernel.ld $(addprefix build/, $(OBJ)) -o build/sizetest.o

all: KERNL386.EXE

cl:
	echo Deleting object files: [$(OBJ)]
	rm build/*.o
