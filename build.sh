#!/bin/bash

export BXSHARE=/opt/homebrew/share/bochs
export PROJECT=$(realpath .)

BOOTDSK=$PROJECT/runconf/dos.img

set -e
export PATH=$PATH:$HOME/opt/cross/bin
export MTOOLSRC=$PROJECT/runconf/mtoolsrc

cd boot
bash 1_build.sh
bash 1_instl.sh $BOOTDSK
cd ..

cd KRNL
bash 1_build.sh
bash 1_instl.sh $BOOTDSK
cd ..

bochs -f runconf/bochsrc.bxrc
