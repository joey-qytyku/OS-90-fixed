#!/bin/bash

THISPKG=$(realpath .)

BLD=$THISPKG/bld

function Clean() {}

function Make() {
	# create the entire source tree folder structure
	local SOURCES=$(find -L . -name *.c)

	cd ${BLD}

}

function Test() {
}

eval $1