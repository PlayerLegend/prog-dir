#!/bin/sh

set -e

cp sums.txt.orig sums.txt

rm -f no-file.txt

$DEBUGGER $PREFIX/tests/test-nc-db-database-1 sums.txt no-file.txt

wc -l sums.txt.orig
wc -l sums.txt

echo No file:
cat no-file.txt
