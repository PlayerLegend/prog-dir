#!/bin/sh

if test -z "$1"
then
    name="$(basename "$PWD")"
    zip -r "$name".zip *
    exit
fi

for file in "$@"
do
    basename "$file"
done | sort | if read name
then
    if zip -r "$name".zip "$@"
    then
	mkdir -p .zipped/"$name"
	mv "$@" .zipped/"$name"/
    fi
fi
