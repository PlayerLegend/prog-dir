#!/bin/sh

select_type() {
    list_types() {
	printf '%s\n' center max fill tile
    }

    list_types | rofi -dmenu -i
}

file_extension() {
    echo "$1" | sed 's|.*\.||g'
}

file_sum() {
    sha256sum "$1" | cut -d' ' -f1
}

stored_name() {
    echo $HOME/.backgrounds/"$(file_sum "$1")"."$(file_extension "$1")"
}

type="$(select_type)"

if test -z "$type"
then
    exit
fi

name="$(stored_name "$1")"

mkdir -p "$(dirname "$name")"
cp "$1" "$name"
feh --bg-$type "$name"
