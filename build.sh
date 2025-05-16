#!/bin/bash
set -e

export MTOOLSRC=$(realpath runconf/mtoolsrc)
export PROJECT=$(realpath .)

cd BOOT
make all
make install-unix
cd ..

cd SHARED/string
make lib
cd ../../

cd KRNL
make all
make install-unix
cd ..

bochs -f runconf/bochsrc.bxrc
