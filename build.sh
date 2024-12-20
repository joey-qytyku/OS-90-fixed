#!/bin/bash

#
# Dependencies:
# - Open watcom 2.0
# - GCC 32-bit cross compiler
# - nasm
# - xxd
# - hexdump
#

echo Building OS/90

source runconf/vars

#
# Find each directory in the root level and if it has an installation
# description file, list it.
#
packages=()

for x in */
do
	if [ test -f ${x}/1_instl.sh ]; then
		packages+=($x)
	fi
done

# We need to build KRNL first

echo Packages: $packages[*]

# Or just merge the source trees?

for x in $packages
do
	cd $x
	./1_build.sh
	./1_instl.sh
	cd $PROJECT
done


