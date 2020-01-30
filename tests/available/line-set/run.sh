#!/bin/bash -e

set -e

run() {
    msg="$1"
    opt="$2"
    echo "$msg"
    ( cat set1.txt
      echo
      cat set2.txt
      echo
      cat set3.txt ) | $DEBUGGER $PREFIX/bin/line-set $opt
    echo '===='
}

run 'AND (default):'
run 'AND (and):' '--op and'
run 'AND (a):' '--op a'
run 'AND (i):' '--op i'
run 'AND (intersection):' '--op intersection'

run 'OR (or):' '--op or'
run 'OR (union):' '--op union'
run 'OR (union):' '--op u'
run 'OR (union):' '--op o'
run 'NOT (not):' '--op not'
run 'NOT (n):' '--op n'

echo 'EMPTY (double empty line with AND):'
( cat set1.txt
  echo
  cat set2.txt
  echo
  echo
  cat set3.txt ) | $DEBUGGER $PREFIX/bin/line-set --op a

echo '===='

echo 'UNCHANGED (double empty line with OR):'
( cat set1.txt
  echo
  cat set2.txt
  echo
  echo
  cat set3.txt ) | $DEBUGGER $PREFIX/bin/line-set --op o
