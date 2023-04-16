#!/bin/bash

set -e

echo "==| BUILDING BOOTLOADER |=="

cd boot
make all
cd ..

echo "==|   BUILDING KERNEL   |=="

cd kernel
make all
cd ..

# No need to use a loopback, I use DOSBox
# We will copy the template from the home directory so it
# resets every time
# I also cannot include the MS-DOS image because of copyright.

cp ../msdos.img ./

dosbox

#qemu-system-i386 -hda msdos.img\
#    -m 4\
#    -no-reboot\
#    -no-shutdown\
#    -monitor stdio

bochs

rm msdos.img
