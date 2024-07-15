#!/bin/bash

set -e

echo "==| BUILDING BOOTLOADER |=="

cd boot
make all
cd ..

echo "==|   BUILDING KERNEL   |=="

cd kernel
./Build.sh
cd ..

dosbox -conf runconf/dosbox.conf

bochs -f runconf/bochsrc.txt
