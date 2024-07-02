#
# I used chatgpt to generate this makefile a while ago, but I have changed most things.
#

TOOLCHAIN := ~/opt/cross1/bin

CC := $(TOOLCHAIN)/i386-linux-gcc

# Assembler
AS := nasm

SRCDIR := src/implement
OBJDIR := build/obj
BINDIR := build

BLOBDIR := src/include/blobs

SOURCES := $(shell find $(SRCDIR) -type f -name "*.c")
ASSEMBLY := $(shell find $(SRCDIR) -type f -name "*.asm")

OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.c=.c.o))
OBJS += $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(ASSEMBLY:.asm=.asm.o))

EXEC := $(BINDIR)/OS90.DAT

OPTS:= -Os -mtune=i386 -fconserve-stack
DEBUG_OPTIONS := -DPRINTF_INCLUDE_CONFIG_H -Wextra -Werror=implicit-function-declaration

# Add regparm?
ABI := -fcall-used-esi -fcall-used-edi -mgeneral-regs-only -mpreferred-stack-boundary=2 -mregparm=3 -mrtd

ARCH := -march=i386

CENV := -nostdlib -nostartfiles -nodefaultlibs -ffreestanding

INCLUDES := -include src/include/Type.h -I src/include/

CFLAGS := $(OPTS) $(ABI) $(ARCH) $(INCLUDES) $(DEBUG_OPTIONS) $(CENV)

AFLAGS := -f elf32 -Ox -i src/include/

LD := $(TOOLCHAIN)/i386-linux-ld

.PHONY: all
all: switch $(EXEC)

$(EXEC): $(OBJS)
	@mkdir -p $(BINDIR)
	@$(LD) --trace --oformat binary -T LinkScripts/Kernel.ld $^ -o $@
	@$(LD) -T LinkScripts/Kernel.ld $^ -o sizetest.o

$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@echo [CC] $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.asm.o: $(SRCDIR)/%.asm
	@mkdir -p $(dir $@)
	@echo [AS] $<
	@$(AS) $(AFLAGS) $< -o $@

switch:
	$(AS) -i src/include -fbin src/implement/ll/switch.ASM -o src/include/blobs/switch.bin

clean:
	rm -rf $(BINDIR)/* $(OBJDIR)/*
