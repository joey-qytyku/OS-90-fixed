#!/bin/bash

COMPILER=~/tch90/dm/bin/dmc.exe

export INCLUDE=~/OS-90-fixed/kernel/src/include

set -e

# Clean all objects

rm -rf build/*.obj

# Build a list of things to compile
CSRC=$(find src -type f -name "*.c")
ASRC=$(find src -type f -name "*.asm")

# DMC supports compiling multiple objects with one single command line

~/tch90/dm/bin/dmc.exe -o -Isrc/include -Hsrc/include/Type.h -mn $CSRC
~/tch90/uasm -zcw -zze -WX -Fw=/dev/null $ASRC

mv *.obj build
