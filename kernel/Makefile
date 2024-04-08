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

EXEC := $(BINDIR)/OS90.DAT

OPTS:= -Os -mtune=i386
DEBUG_OPTIONS := -DPRINTF_INCLUDE_CONFIG_H -Wextra -Werror=implicit-function-declaration

# Add regparm?
ABI := -fcall-used-esi -fcall-used-edi -fcall-used-ebx -mgeneral-regs-only -mpreferred-stack-boundary=2 -mregparm=3 -mrtd

ARCH := -march=i386

CENV := -nostdlib -nostartfiles -nodefaultlibs -ffreestanding

INCLUDES := -include src/include/C/Type.h -iquote src/include/C

CFLAGS := $(OPTS) $(ABI) $(ARCH) $(INCLUDES) $(DEBUG_OPTIONS) $(CENV)

AFLAGS := -f elf32 -Ox -i src/include/ASM

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
