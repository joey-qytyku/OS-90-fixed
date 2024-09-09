#!/bin/bash

BOOTDSK=$HOME/FreeDOS.img

set -e
export PATH=$PATH:$HOME/opt/cross/bin

cd boot
bash 1_BUILD.SH
bash 1_INSTL.SH $BOOTDSK
cd ..

cd KRNL
bash 1_BUILD.SH
bash 1_INSTL.SH $BOOTDSK
cd ..
