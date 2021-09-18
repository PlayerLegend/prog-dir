C_PROGRAMS += test/file
SH_PROGRAMS += test/run-file
RUN_TESTS += test/run-file

file-tests: test/file
file-tests: test/run-file

test/file: src/array/buffer.o
test/file: src/file/file.o
test/file: src/file/test/file.test.o
test/run-file: src/file/test/file.test.sh

tests: file-tests
