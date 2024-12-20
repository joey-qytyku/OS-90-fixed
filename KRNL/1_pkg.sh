#!/bin/bash

THISPKG=$(realpath .)

function uninstall() {
	mdel c:/OS90/OS90.EXE
	return 0
}

function install() {
	mcopy ../os90.exe c:/OS90/OS90.EXE
	return 0
}

function clean() {
	local removing=( $(find ${THISPKG} | grep -i "(\\.o$)|(os90\.err)" ) )
	echo Removing: $removing
	rm_ifexist removing
	return 0
}

function make() {
	function mk_chkfail() {
		if [ $1 -ne 0  ]; then
			clean
		fi
	}

	cd src/krnl

	${KRNL_CC} ${KRNL_CFLAGS} -c *.c
	mk_chkfail $?

	${KRNL_LD} -T ${THISPKG}1_link.ld --oformat=binary -o ${THISPKG}/krnl.bin
	mk_chkfail $?

	cd ../boot

	xxd -i ${THISPKG}/krnl.bin | sed -n '2,$ p' | sed -n '$ ! p' | sed -n '$ ! p' > data.h

	${WCL} -q -mc -lr -3 os90.c
	mk_chkfail $?

	cd $THISPKG

	clean
}

if [ $(pwd) -ne "KRNL" ]; then
	echo "Error: please change directory to the package base"
fi

eval $1
