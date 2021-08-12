#!/bin/sh

set -e

if test -z "$STORE_DIR"
then
    >&2 echo "Environment variable STORE_DIR is undefined"
    exit 1
fi

set_path() {
    path="$STORE_DIR"/"$(echo "$1" | cut -c1,2)"/"$(echo "$1" | cut -c3,4)"/"$1"
}

check_path() {
    if ! test "$(sha256sum "$path" | cut -d' ' -f1)" = "$(basename "$path")"
    then
	>&2 echo "File exists for $(basename "$path"), but it is corrupt: $path"
	exit 1
    fi
}

cmd_digest() {
    digest="$(sha256sum "$1" | cut -d' ' -f1)"
    if test -z "$digest"
    then
	exit 1
    fi
    echo "$digest"
}

cmd_path() {

    if test -z "$1"
    then
	while read path
	do
	    cmd_path "$path"
	done

	return
    fi
    
    set_path "$1"
    
    if ! test -e "$path"
    then
	>&2 echo "Digest is not in directory $STORE_DIR"
	exit 1
    fi

    if ! check_path
    then
	exit 1
    fi

    echo "$path"
}

cmd_add() {
    if test -d "$1"
    then
	find "$1" -type f | while read file
	do
	    cmd_add "$file"
	done

	return
    fi
    
    digest="$(cmd_digest "$1")"

    set_path "$digest"

    mkdir -p "$(dirname "$path")"

    if test -e "$path"
    then
	if ! check_path
	then
	    exit 1
	else
	    echo "$digest"
	    return
	fi
    else
	cp -lL "$1" "$path" 2>/dev/null || cp -L "$1" "$path"
	chmod 400 "$path"
	echo "$digest"
    fi
}

cmd_cat() {
    set_path "$1"

    if test -e "$path"
    then
	if ! check_path
	then
	    exit 1
	else
	    cat "$path"
	fi
    else
	>&2 echo "File is not in store dir"
	exit 1
    fi
}

cmd_symlink() {
    set_path "$1"

    if test -e "$path"
    then
	if ! check_path
	then
	    exit 1
	else
	    ln -vs "$path" "$2"
	fi
    else
	>&2 echo "File is not in store dir"
	exit 1
    fi
}

cmd_"$@"
