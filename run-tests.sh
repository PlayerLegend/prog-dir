#!/bin/sh

set -e

for program in "$@"
do
    
    echo -n "Testing $program ... "

    fatal() {
	echo "$@"
	echo "stdout written to $program.stdout"
	echo "stderr written to $program.stderr"
	echo "$program FAILED"
	exit 1
    }

    if ! "$program" > "$program".stdout 2> "$program".stderr
    then
	fatal "Failed to run $program"
    fi

    for output in stdout stderr
    do
	outfile_name="$(basename "$program")"."$output"
	outfile="$(find src -name "$outfile_name")"

	if ! test -f "$outfile"
	then
	    fatal "No file named $outfile_name under src/"
	fi

	if ! cmp "$program"."$output" "$outfile"
	then
	    fatal "$output of program does not match what is expected in [$outfile]"
	fi
    done

    echo "PASSED"
done
