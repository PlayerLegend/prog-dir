#!/bin/sh

if ! test -z "$1"
then
    $EDITOR "$(basename "$PWD") $(date +%Y-%m-%d -d "$@").txt"
else
    $EDITOR "$(basename "$PWD") $(date +%Y-%m-%d).txt"
fi
