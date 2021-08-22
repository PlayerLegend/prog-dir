test/file: src/file/test/file.test.o src/file/file.o src/array/buffer.o
test/run-file: src/file/test/file.test.sh

TESTS_C += test/file
TESTS_SH += test/run-file
RUN_TESTS += test/run-file

