#!/bin/sh

list_files() {
    find "$tmp" -type f -not -name '*.json' | sort -V
}

count_files() {
    list_files | wc -l
}

_gallery_tag() {
    download_archive="$(mktemp)"
    tmp="$(mktemp -d)"
    xterm -e gallery-dl -d "$tmp" --download-archive "$download_archive" "$1"

    first_path="$(find "$tmp" -type f -not -name '*.json' | sort -V | head -1)"
    first="$(echo "$first_path" | rev | cut -d/ -f1,2 | rev)"
    echo "$first"

    if test "$(count_files)" -gt 1
    then
	list_files | tr '\n' '\0' | TYPE="gallery" TAG="$first" xargs -0 tag add files
    fi

    list_files | tr '\n' '\0' | xargs -0 tag-interactive viewimage

    rm -r "$tmp" "$download_archive"
}

if ! test -z "$1"
then
    for link in "$@"
    do
	_gallery_tag "$link"
    done
else
    while read link
    do
	_gallery_tag "$link"
    done
fi
