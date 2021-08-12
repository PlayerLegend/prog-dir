#!/bin/sh

_gallery_zip() {
    rm -f /tmp/download-archive
    tmp="$(mktemp -d)"
    gallery-dl -d "$tmp" --download-archive /tmp/download-archive "$1"
    if test "$(find "$tmp" -type f -name '*.json' | wc -l)" = 1
    then
	find "$tmp" -type f -not -name '*.json' | while read file
	do
	    cp "$file" ./
	done
    else
	( cd "$tmp" && find -type f | tr '\n' '\0' | xargs -0 create-zip )
	cp -i "$tmp"/*.zip ./
    fi
    
    rm -r "$tmp"
}

if ! test -z "$1"
then
    for link in "$@"
    do
	_gallery_zip "$link"
    done
else
    while read link
    do
	_gallery_zip "$link"
    done
fi
