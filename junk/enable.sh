#!/bin/sh

tmp="$(mktemp -d)"

ls available | sort > "$tmp"/available
ls enabled | sort > "$tmp"/enabled

echo "Disabled tests:"
echo
comm -23 "$tmp"/available "$tmp"/enabled

rm -r "$tmp"

prompt() {
    echo
    echo -n "Enable a test> "
}

prompt

while read name
do
    if test -e enabled/"$name"
    then
	echo "'$name' is already enabled"
	prompt
	continue
    elif ! test -e available/"$name"
    then
	echo "'$name' does not exist"
	prompt
	continue
    fi

    ln -vrs available/"$name" enabled/"$name"

    prompt
done

echo
