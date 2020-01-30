#!/bin/sh

set -e

if test -z "$1"
then
    ls enabled | xargs -d'\n' sh "$0"
    exit
fi

export DEBUGGER='valgrind --fullpath-after='
export TEST_FILES="$(realpath ../test-files)"
export TARGET="$(mktemp -d)"
mkdir -p /tmp/test-output

make -C .. clean
make -C .. debug

title_output_bad() {
    echo "$1"
    shift
    echo "===="
    echo "$@"
    echo "===="
}

title_output_good() {
    echo "$1"
    shift
    echo "++++"
    echo "$@"
    echo "++++"
}


for name in "$@"
do
    echo
    echo "Running test '$name'"
    export tmp="$(mktemp -d)"
    if (
	    set -e
	    cd available/"$name"
	    output=/tmp/test-output/"$name".txt
	    sh run.sh > "$output"
	    
	    if test $? != 0
	    then
		echo "Program exited with error $?"
		title_output_bad "Program output" "$(cat "$output")"
		exit 1
	    fi
	    
	    if test -f expected.txt
	    then
		if ! test "$(cat "$output")" = "$(cat expected.txt)"
		then
		    echo "Program generated incorrect output"
		    title_output_bad "Program output" "$(cat "$output")"
		    title_output_bad "Expected output" "$(cat expected.txt)"
		    output_file="$(mktemp)"
		    echo "$(cat "$output")" > "$output_file"
		    title_output_bad "Diff" "$(diff "$output_file" expected.txt)"
		    rm "$output_file"
		    exit 1
		else
		    echo "Program generated correct output"
		    title_output_good "Program output" "$output"
		fi
	    else
		title_output_good "Program output" "$output"
	    fi )
    then
	echo "Passed '$name'"
	rm -r "$tmp"
    else
	echo "Failed on '$name'"
	rm -r "$TARGET" "$tmp"
	exit 1
    fi
done

rm -r "$TARGET"
