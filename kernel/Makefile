.PHONY: pre-build

pre-build:
	@mkdir -p $(DIRS)

TCHAIN := ~/opt/cross1/bin

AS=nasm -felf -Ox -I include/
CC=$(TCHAIN)/i386-linux-gcc
LINK=$(TCHAIN)/i386-linux-ld

CFLAGS := -I ./include/ -O2 -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mpreferred-stack-boundary=2 -march=i386 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99 -Wextra

CSOURCE := $(shell find . -type f -name '*.c' -printf "%P\n")
ASOURCE := $(shell find . -type f -name '*.asm' -printf "%P\n")

DIRS := $(shell find . -type d \( -name "include" -o -name "LinkScripts" -o -name "build" \) -prune -o -type d -printf "build/%P ")

CTARGETS := $(addprefix build/, $(patsubst %.c,%.c.o, $(CSOURCE)))
ATARGETS := $(addprefix build/, $(patsubst %.asm,%.asm.o, $(ASOURCE)))

$(CTARGETS): build/%.c.o: %.c
	@echo [CC] $< => $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(ATARGETS): build/%.asm.o: %.asm
	@echo [AS] $< => $@
	@$(AS) $< -o $@

all: pre-build $(CTARGETS) $(ATARGETS)
	@echo Linking kernel binary
	$(LINK) -T LinkScripts/Kernel.ld $(CTARGETS) $(ATARGETS) -o build/KERNL386.EXE

cl:
	rm -rf build/*
