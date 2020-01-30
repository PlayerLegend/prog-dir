#!/bin/sh

test="$1"

export tmp="$(mktemp -d)"

fail() {
    rm -r "$tmp"
    exit 1
}

succeed() {
    rm -r "$tmp"
    exit 0
}

if ! cd enabled/"$tmp"
then
    fail
fi

if ! sh run.sh > output.txt
then
    exit
fi

succeed
