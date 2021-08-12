#!/bin/sh

export LANG=en_US.UTF-8

if ! ffmpeg -h > /dev/null
then
    exit 1
fi

if ! test $LANG = en_US.UTF-8
then
    echo LANG is $LANG, not en_US.UTF-8
    exit 1
fi

dl() {
    LANG=en_US.UTF-8 youtube-dl \
	--download-archive archived.txt \
	--audio-quality 320k \
	--write-description \
	--write-info-json \
	--write-annotations \
	--ignore-errors \
	--playlist-reverse \
	-f bestvideo+bestaudio/best \
	-o 'by-id/%(extractor)s/%(id)s/%(id)s.%(ext)s' \
	"$@"
}

remove_special() {
    echo "$1" | sed 'sA/A_Ag'
}

linkup() {
    find by-id -name '*.json' | tr '\n' '\0' | xargs -0 cat | jq -r '._filename,.uploader,.title,.playlist,.playlist_index,.upload_date' \
	| while read filename && read uploader && read title && read playlist && read playlist_index && read upload_date
    do
	if test "$playlist" != null
	then
	    destination=human-readable/"$(remove_special "$uploader [$upload_date] - $playlist: $playlist_index - $title")"
	else
	    destination=human-readable/"$(remove_special "$uploader [$upload_date] - $title")"
	fi

	if ! test -e "$destination"
	then
	    mkdir -p "$(dirname "$destination")"
	    ln -vs ../"$(dirname "$filename")" "$destination"
	fi
    done
}

grep -v -e '^$' -e '^#' links.txt | while read name
do
    if ! dl "$name"
    then
	exit 1
    fi
done

linkup
