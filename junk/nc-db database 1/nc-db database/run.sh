#!/bin/sh

set -e

SINGLE=test-single.txt
MULTIPLE=test-multiple.txt

rm -f $SINGLE $MULTIPLE

$DEBUGGER $PREFIX/tests/test-nc-db-database-1 $SINGLE $MULTIPLE
