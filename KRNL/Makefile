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

CPU=i386

COMMON_FLAGS := -D EXCEPTIONS_SUPPORTED=20 -D__KRNL__ -D CPU="$(CPU)"

CFLAGS += -mgeneral-regs-only

LDFLAGS += -T 1_link.ld

# GCC supports the T option for linking. We HAVE to use this in the event
# of a libgcc call even in a freestanding environment. BTW libgcc is not
# that big. Most features are not used, e.g. 64-bit division.



all:

