#
# I used chatgpt to generate this makefile.
#

TOOLCHAIN := ~/opt/cross1/bin

CC := $(TOOLCHAIN)/i386-linux-gcc

# Assembler
AS := nasm

SRCDIR := src/implement
OBJDIR := build/obj
BINDIR := build

SOURCES := $(shell find $(SRCDIR) -type f -name "*.c")
ASSEMBLY := $(shell find $(SRCDIR) -type f -name "*.asm")

OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.c=.o))
OBJS += $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(ASSEMBLY:.asm=.o))

EXEC := $(BINDIR)/KERNL386.EXE

OPTS:= -Os -mtune=i386

CFLAGS := -I ./src/include/ $(OPTS) -DPRINTF_INCLUDE_CONFIG_H -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mpreferred-stack-boundary=2 -march=i386 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99 -Wextra -fcall-used-esi -fcall-used-edi

AFLAGS := -f elf32 -Ox -i src/include/

LD := $(TOOLCHAIN)/i386-linux-ld

.PHONY: all
all: $(EXEC)

$(EXEC): $(OBJS)
	@mkdir -p $(BINDIR)
	$(LD) --trace --oformat binary -T LinkScripts/Kernel.ld $^ -o $@
	$(LD) -T LinkScripts/Kernel.ld $^ -o sizetest.o
	python3 LinkScripts/genksyms.py

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@echo [CC] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	@mkdir -p $(dir $@)
	@echo [AS] $<
	@$(AS) $(AFLAGS) $< -o $@

clean:
	rm -rf $(BINDIR)/* $(OBJDIR)/*
