#
# Hash function used to generate unique number for each file name.
#
define hash
$(shell echo -n $1 | md5sum | cut -d' ' -f1)
endef

CC := ~/tch90/dm/bin/dmc.exe

# Assembler
AS := ~/tch90/uasm

SRCDIR := src/implement
OBJDIR := build/obj
BINDIR := build

BLOBDIR := src/include/blobs

CSOURCES := $(shell find $(SRCDIR) -type f -name "*.c")
ASSEMBLY := $(shell find $(SRCDIR) -type f -name "*.asm")

OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.c=.c.o))
OBJS += $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(ASSEMBLY:.asm=.asm.o))

EXEC := $(BINDIR)/OS90.DAT

INCLUDES := -include src/include/Type.h -I src/include/

CFLAGS := -c -mn -o -DPRINTF_INCLUDE_CONFIG_H

AFLAGS :=

LDCMD := /mnt/c/WATCOM/WLINK.EXE @wlink.lnk

.PHONY: all
all: switch $(EXEC)

$(EXEC): $(OBJS)
	@$(LDCMD) $<

$(OBJDIR)/%.c.o: $(CSOURCES)
	@mkdir -p $(dir $@)
	@echo [CC] $<
	@$(CC) $(CFLAGS) $< -o$(call hash, $@).obj

$(OBJDIR)/%.asm.o: $(SRCDIR)/%.asm
	@$(AS) $(AFLAGS) $< -o $(call hash, $@)

switch:
	nasm -i src/include -fbin src/implement/LL/switch.ASM -o src/include/BLOBS/switch.bin

clean:
	rm -rf $(BINDIR)/* $(OBJDIR)/*
