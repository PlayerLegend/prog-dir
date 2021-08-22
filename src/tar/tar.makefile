test/tar-dump-posix-header: src/tar/test/tar-dump-posix-header.test.o
test/list-tar: src/tar/test/list-tar.test.o

test/list-tar: src/tar/tar.o
test/list-tar test/tar-dump-posix-header: src/buffer_io/buffer_io.o
test/list-tar test/tar-dump-posix-header: src/log/log.o
test/list-tar test/tar-dump-posix-header: src/array/buffer.o


test/run-list-tar: src/tar/test/list-tar.test.sh
test/run-tar-dump-posix-header: src/tar/test/tar-dump-posix-header.test.sh

TESTS_C += test/list-tar test/tar-dump-posix-header
TESTS_SH += test/run-list-tar test/run-tar-dump-posix-header
RUN_TESTS += test/run-list-tar test/run-tar-dump-posix-header
