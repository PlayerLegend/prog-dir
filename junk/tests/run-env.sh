#!/bin/sh

if test -z "$PREFIX"
then
    export PREFIX="$(mktemp)"
    make clean
    make install
fi

export DEBUGGER='valgrind --fullpath-after='
