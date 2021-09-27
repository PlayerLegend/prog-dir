#!/bin/sh

while ! test -e Makefile && ! test "$PWD" = "/"
do
    cd ..
done

make "$@"
