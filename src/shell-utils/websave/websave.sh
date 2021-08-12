#!/bin/sh

set -e

archive_4chan() {
    cd /datasets/art/4chan/ && thread-archiver --runonce "$1"
}

archive_gallery() {
    gallery-dl "$1"
}

archive_video() {
    (cd ~/youtube-dl && youtube-dl "$1" --audio-quality 320k --write-description --write-info-json --write-annotations --ignore-errors --no-playlist)
}

archive_link_artist() {
    cd $HOME/downloads/lewd && ( echo "$1" | sh $ENV get_art_here )
}

archive_gallery_zip() {
    cd $HOME/downloads/lewd && gallery-zip "$1"
}

archive_gallery_tag() {
    cd $HOME/downloads/lewd && gallery-tag "$1"
}

cmd_archive() {
    record() {
	sources='/datasets/art/sources.txt'
	echo "$@" >> "$sources"
	sort -u -o "$sources" "$sources"
    }

    record "$@"

    echo "Downloading $@"
    archive_"$@"
}

list_archivers() {
    sort -u <<EOF
4chan
gallery
video
link_artist
gallery_zip
EOF
}

fuzzy() {
    if test -z "$1"
    then
	return 1
    fi

    for check in "$@"
    do
#	echo check "$check in $QUTE_URL"
	if echo "$url" | grep -is "$check"
	then
	    return 0
	fi
    done

    return 1
}

cmd_terminal() {
    while ! ("$@")
    do
	echo
	echo "FAILED"
	echo -n "press enter to try again"
	read asdf
    done

    echo
    echo "DONE"
    read -t 5 asdf
}

terminal() {
    xterm -e "$0" terminal "$@"
}

is_image() {
    fuzzy 'jpg$' 'png$' 'gif$' 'webp$' 'jpeg$' '\..*:large$'
}

if test -z "$1"
then
    ( xclip -o; echo ) | if read url
    then
	echo "url: $url"
	if fuzzy sankaku booru twi 'pixiv.net/en/users'
	then
	    terminal "$0" archive gallery "$url"
	elif fuzzy 'pixiv.net/en/artworks'
	then
	    terminal "$0" archive gallery_tag "$url"
	elif fuzzy '4chan'
	then
	    terminal "$0" archive 4chan "$url"
	elif fuzzy 'tube'
	then
	    terminal "$0" archive video "$url"
	else
	    choice="$(list_archivers | rofi -dmenu -i -p "Choose an archiver for $url")"
	    if ! test -z "$choice"
	    then
		terminal "$0" archive "$choice" "$url"
	    fi
	fi
    fi
else
    cmd_"$@"
fi
