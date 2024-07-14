#!/bin/bash

COMPILER=~/tch90/dm/bin/dmc.exe

set -e

# Clean all objects

rm -rf build/*

# Build a list of things to compile
CSRC := find src -type f -name "*.c"
ASRC := find src -type f -name "*.asm"

# DMC supports compiling multiple objects with one single command line

cd build



cd ..
