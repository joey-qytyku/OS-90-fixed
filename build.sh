#!/bin/bash

#
# Dependencies:
# - Open watcom 2.0
# - GCC 32-bit cross compiler
# - nasm
# - xxd
# - hexdump
#

source runconf/vars

#
# Find each directory in the root level and if it has an installation
# description file, list it.
#
packages=()

for x in */
do
	if [[ -f ${x}/1_pkg.sh ]]; then
		packages+=($x)
	fi
done

echo Packages: "${packages[*]}"

# Or just merge the source trees?

for x in "${packages[@]}"
do
	cd ${PROJECT}/${x}
	./1_pkg.sh Make
	if [ $? -ne 0 ]; then
		./1_pkg.sh Clean
	fi
done
