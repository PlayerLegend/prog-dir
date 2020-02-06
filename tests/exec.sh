#!/bin/sh

if test "$(basename "$0")" = exec-interactive.sh
then
    export interactive=true
fi

export DEBUGGER='valgrind --fullpath-after='
if test -z "$PREFIX"
then
    export PREFIX="$(mktemp -d)"
    owns_prefix=true

    if ! make -C .. clean
    then
	exit 1
    fi
fi

clean() {
    val="$1"
    shift
    if test "$interactive" = true
    then
	echo "Press enter to run again, ctrl+d to stop"
	if read
	then
	    sh "$0" "$@"
	fi
    fi
    
    if test "$owns_prefix" = true
    then
	echo cleaning
	make -C .. clean
	rm -r "$PREFIX"
    fi

    exit "$val"
}

if ! (make -C .. debug && make -C .. install)
then
    echo "Building failed"
    clean 1 "$@"
fi

print_file() {
    echo
    echo "$2":
    echo '['
    cat "$1"
    echo ']'
}

echo listing
ls "$PREFIX"

run_test() {
    test="$1"

    ( if ! cd "$test"
      then
	  return 1
      fi

      if ! sh run.sh > output.txt
      then
	  echo "Test '$test' failed"
	  print_file output.txt 'Failed output'
	  return 1
      fi

      if test -f expected.txt && test "$(cat output.txt)" != "$(cat expected.txt)"
      then
	  echo "Test produced incorrect output"
	  print_file expected.txt 'Expected output'
	  print_file output.txt 'Test output'
	  diff --color=always expected.txt output.txt
	  return 1
      else
	  print_file output.txt 'Test output'
	  return 0
      fi
    )
}

if ! test -z "$1"
then
    for test in "$@"
    do
    if ! run_test "$test"
    then
	echo "Test '$test' failed"
	echo
	clean 1 "$@"
    fi
    done
else
    for test in enabled/*/
    do
	if ! run_test "$test"
	then
	    echo "Test '$test' failed"
	    echo
	    clean 1 "$@"
	fi
    done
fi

echo "All tests passed"

clean 0 "$@"
