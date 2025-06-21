#!/bin/bash

export WATCOM=$HOME/Desktop/open-watcom-v2/rel
export LIBDOS=$WATCOM/lib286
export LIB=$LIBDOS:$WATCOM/lib386
export INCLUDE=$(realpath $LIBDOS/../h)

# Disable long file names in libraries. This reduces the code size for when
# it is not needed. May be overriden in other scripts.
export LFN=N

export "PATH=$PATH:$HOME/Desktop/open-watcom-v2/rel/armo64"

# Update according to file extension and platform
# export "WCL=$WATCOM/armo64/wcl"

$HOME/Desktop/open-watcom-v2/cmnvars.sh

wcl main.c
