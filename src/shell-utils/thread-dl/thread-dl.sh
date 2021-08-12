#!/bin/sh

set -e

cfg_list=$HOME/.config/thread-dl.list
cfg_404=$HOME/.config/thread-dl.404

if ! test -f "$cfg_list"
then
    touch "$cfg_list"
fi

if ! test -f "$cfg_404"
then
    touch "$cfg_404"
fi

add_line() {
    if ! grep -qFx "$1" "$2"
    then
	echo "$1" >> "$2"
    fi
}

list_threads() {
    ( cut -d' ' -f1 ~/.config/qutebrowser/bookmarks/urls | grep boards.4chan
      cat "$cfg_list" ) | cut -d'#' -f1 | sort -u | while read line
    do
	if ! grep -qFx "$line" "$cfg_404"
	then
	    echo "$line"
	fi
    done
}

is_404() {
    test "$(curl -s "$1" | hxnormalize -x | hxselect -c title)" = '4chan - 404 Not Found'
}

dl_thread() {
    echo "Downloading: $1"
    add_line "$1" "$cfg_list"
    if is_404 "$1"
    then
	add_line "$1" "$cfg_404"
    else
	( cd /nas/art/4chan && thread-archiver --runonce "$1" )
    fi
}

if test -z "$1"
then
    list_threads | while read url
    do
	dl_thread "$url"
    done
elif test "$1" = list
then
    list_threads
else
    for arg in "$@"
    do
	dl_thread "$arg"
    done
fi
