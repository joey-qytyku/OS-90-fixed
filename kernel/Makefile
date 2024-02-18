#
# I used chatgpt to generate this makefile.
#

TOOLCHAIN := ~/opt/cross1/bin

CC := $(TOOLCHAIN)/i386-linux-gcc

# Assembler
AS := nasm

#KERNEL.IMG

SRCDIR := src/implement
OBJDIR := build/obj
BINDIR := build

SOURCES := $(shell find $(SRCDIR) -type f -name "*.C")
ASSEMBLY := $(shell find $(SRCDIR) -type f -name "*.ASM")

OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.C=.o))
OBJS += $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(ASSEMBLY:.ASM=.o))

EXEC := $(BINDIR)/KERNL386.EXE

OPTS:= -Os -mtune=i386
DEBUG_OPTIONS := -DPRINTF_INCLUDE_CONFIG_H -Wextra -Werror=implicit-function-declaration
ABI := -fcall-used-esi -fcall-used-edi -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -mpreferred-stack-boundary=2
ARCH := -march=i386
CENV := -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -std=c99

CFLAGS := -x c -I ./src/include/C/ $(OPTS) $(ABI) $(ARCH) $(DEBUG_OPTIONS) $(CENV)

AFLAGS := -f elf32 -Ox -i src/include/ASM/NASMX -i src/include/ASM

LD := $(TOOLCHAIN)/i386-linux-ld

.PHONY: all
all: $(EXEC)

$(EXEC): $(OBJS)
	@mkdir -p $(BINDIR)
	$(LD) --trace --oformat binary -T LinkScripts/Kernel.ld $^ -o $@
	$(LD) -T LinkScripts/Kernel.ld $^ -o sizetest.o
	python3 LinkScripts/genksyms.py

$(OBJDIR)/%.o: $(SRCDIR)/%.C
	@mkdir -p $(dir $@)
	@echo [CC] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.ASM
	@mkdir -p $(dir $@)
	@echo [AS] $<
	@$(AS) $(AFLAGS) $< -o $@

clean:
	rm -rf $(BINDIR)/* $(OBJDIR)/*
