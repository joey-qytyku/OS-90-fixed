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
	local removing=(find -L $THISPKG \( -name "*.o" -o -name "*.exe" -o -name "*.err" -o -name "*.bin" \) ) )
	echo Removing: $removing
	rm_ifexist removing
	return 0
}

function Make() {
	function mk_chkfail() {
		if [ $1 -ne 0  ]; then
			clean
		fi
	}

	cd src/krnl

	${KRNL_CC} ${KRNL_CFLAGS} -c *.c
	mk_chkfail $?

	${KRNL_LD} -T ${THISPKG}/1_link.ld --oformat=binary -o $BLD/krnl.bin
	mk_chkfail $?

	cd ../boot

	xxd -i ${THISPKG}/krnl.bin | sed -n '2,$ p' | sed -n '$ ! p' | sed -n '$ ! p' > data.h

	${WCL} -q -mc -lr -3 os90.c
	mk_chkfail $?

	cd $THISPKG

	Clean
}

if [ $(pwd) -ne "KRNL" ]; then
	echo "Error: please change directory to the package base"
fi

eval $1
