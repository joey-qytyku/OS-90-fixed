#!/bin/bash

THISPKG=$(realpath .)

BLD=$THISPKG

function Uninstall() {
	mdel c:/OS90/OS90.EXE
	return 0
}

function Install() {
	mcopy ../os90.exe c:/OS90/OS90.EXE
	return 0
}

function Clean() {
	local removing=$(find -L $THISPKG \( -name "*.o" -o -name "*.exe" -o -name "*.err" -o -name "*.bin" \) )

	rm_ifexist ${removing[*]}
	return 0
}

function mk_chkfail() {
	if [ $1 -ne 0  ]; then
		Clean
		exit 1
	fi
}

function Make() {

	cd src/krnl

	for x in *.asm; do
		nasm -felf $x
	done

	${KRNL_CC} -include type.h ${KRNL_CFLAGS} -c *.c
	mk_chkfail $?

	${KRNL_LD} -T ${THISPKG}/1_link.ld --oformat=binary -o $BLD/krnl.bin *.o
	mk_chkfail $?

	cd ../boot

	output_asm_bin ${THISPKG}/krnl.bin > data.h

	${WCL} -q -mc -lr -3 os90.asm
	mk_chkfail $?

	cd $THISPKG

	Clean
}

if [ ["$(pwd)" == "KRNL"] ]; then
	echo "Error: please change directory to the package base"
fi

eval $1
