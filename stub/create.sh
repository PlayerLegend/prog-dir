#!/bin/sh

echo -n "Name this program> "

read program_name

if test -z "$program_name"
then
    echo "A program name is required"
    exit 1
fi

stub_name=available/"$(basename "$program_name")".makefile

if test -e "$stub_name"
then
    echo "error: stub '$stub_name' already exists!"
    exit 1
fi

echo -n "Add object files> "

read program_obj

if test -z "$program_obj"
then
    echo "Program objects are required"
    exit 1
fi

echo -n "Add pkg-config packages> "

read program_pkg

echo -n "Add cflags> "

read program_cflags

echo -n "Add ldlibs> "

read program_ldlibs

# build the file

tmp="$(mktemp)"

echo 'PROGRAM_NAME = '"$program_name" >> "$tmp"
echo 'PROGRAM_OBJ = '"$program_obj" >> "$tmp"
echo >> "$tmp"
if ! test -z "$program_cflags"
then
    echo 'CFLAGS += '"$program_cflags" >> "$tmp"
fi

if ! test -z "$program_ldlibs"
then
    echo 'LDLIBS += '"$program_ldlibs" >> "$tmp"
fi

if ! test -z "$program_pkg"
then
    echo >> "$tmp"
    echo 'PKG_LDLIBS != pkg-config --libs '"$program_pkg" >> "$tmp"
    echo 'PKG_CFLAGS != pkg-config --cflags '"$program_pkg" >> "$tmp"
    echo >> "$tmp"
    echo 'CFLAGS += $(PKG_CFLAGS)' >> "$tmp"
    echo 'LDLIBS += $(PKG_LDLIBS)' >> "$tmp"
fi

echo "Created a stub file:"
echo
cat "$tmp"
echo
echo -n "Use this file? [y/n] "

while read ans
do
    if test "$ans" = y
    then
	cp -v "$tmp" "$stub_name"
	ln -vrs "$stub_name" enabled/
	break
    elif test "$ans" = n
    then
	break
    else
	echo -n "Answer y or n> "
    fi
done

rm "$tmp"

echo
