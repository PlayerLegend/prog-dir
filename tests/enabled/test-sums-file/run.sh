#!/bin/sh

set -e

$DEBUGGER $PREFIX/tests/test-sums-db sums.txt
