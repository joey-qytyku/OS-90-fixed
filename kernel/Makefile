.PHONY: pre-build

pre-build:
	mkdir -p $(addprefix "build/", $(DIRS))
	@echo $(OBJ)

SHELL=/bin/bash

TCHAIN := ~/opt/cross1/bin

CFLAGS := -I ./include/ -O2 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mpreferred-stack-boundary=2 -march=i386 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99 -Wextra

CC=$(TCHAIN)/i386-linux-gcc
LINK=$(TCHAIN)/i386-linux-ld

CSOURCE=$(shell find . -type f -name '*.c' -printf "%P ")
ASOURCE=$(shell find . -type f -name '*.asm' -printf "%P ")

OBJ= $(patsubst %.c, build/%.c.o, $(CSOURCE)) $(patsubst %.asm, build/%.asm.o, $(ASOURCE))

DIRS=$(shell find . -type d \( -name "include" -o -name "LinkScripts" -o -name "build" \) -prune -o -type d -printf "%P ")

AS=nasm
ASFLAGS=-felf -Ox -I include/

# -Ox will enable multipass assembly for more optimized code
#$(info  $(OBJ))

%.c.o: $(CSOURCE)
	@echo [CC] $<
	@$(CC) $(CFLAGS) -c $< -o $@

%.asm.o: %.asm
	@echo [AS] $<
	@$(AS) $(ASFLAGS) $< -o $@

KERNL386.EXE: $(OBJ)
	@$(LINK) [LD] --oformat binary -T LinkScripts/Kernel.ld $(addprefix build/, $(OBJ)) -o build/KERNL386.EXE
	@$(LINK) [LD] --oformat elf32-i386 -T LinkScripts/Kernel.ld $(addprefix build/, $(OBJ)) -o build/sizetest.o

all: pre-build KERNL386.EXE

cl:
	rm -rf build/*
