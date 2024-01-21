#!/bin/bash

set -e

echo "==| BUILDING BOOTLOADER |=="

cd boot
make all
cd ..

echo "==|   BUILDING KERNEL   |=="

cd kernel
make clean
make all
cd ..

dosbox -conf runconf/dosbox.conf

bochs -f runconf/bochsrc.txt
