#!/bin/bash

COMPILER=~/tch90/dm/bin/dmc.exe

set -e

# Clean all objects

rm -rf build/*

# Build a list of things to compile
CSRC := find $(SRCDIR) -type f -name "*.c"
ASRC := find $(SRCDIR) -type f -name "*.asm"

# DMC supports compiling multiple objects with one single command line
# The only problem is that this will not support
