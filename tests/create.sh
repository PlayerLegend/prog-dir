#!/bin/sh

set -e

prompt() {
    echo -n "$@"
    echo -n "> "
}

yn() {
    echo -n "$@" '[y/n] '
    while read ans
    do
	if test "$ans" = y
	then
	    return 0
	elif test "$ans" = n
	then
	    return 1
	else
	    echo -n "Answer y or n> "
	fi
    done
}

prompt "Enter a name for the test"

read test_name

if test -d available/"$test_name"/
then
    if ! yn "'$test_name' exists, edit it?"
    then
	exit 0
    fi
fi

mkdir -p available/"$test_name"
cd available/"$test_name"

if ! test -f run.sh
then
    echo '#!/bin/bash -e' > run.sh
    echo >> run.sh
fi

$EDITOR run.sh

if yn "Record the output of this script?"
then
    while true
    do
	echo "Running program ..."
	export TARGET="$(mktemp -d)"
	export TEST_FILES="$(realpath ../../../test-files)"
	export DEBUGGER='valgrind --fullpath-after='
	export tmp="$(mktemp -d)"
	make -C ../../..
	sh run.sh > expected.txt
	rm -r "$TARGET" "$tmp"
	echo "Done"

	echo "Got output:"
	echo "===="
	cat expected.txt
	echo "===="

	if yn "Use this output?"
	then
	    break
	fi
    done
elif test -f expected.txt
then
    if yn "Output already exists, would you like to delete it?"
    then
	rm expected.txt
    fi
fi

cd ..
mkdir -p ../enabled
if ! test -d ../enabled/"$test_name"/
then
    ln -vrs "$test_name" ../enabled/
fi
