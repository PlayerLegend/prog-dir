C_PROGRAMS += test/list-tar
C_PROGRAMS += test/tar-dump-posix-header
RUN_TESTS += test/run-list-tar
RUN_TESTS += test/run-tar-dump-posix-header
SH_PROGRAMS += test/run-list-tar
SH_PROGRAMS += test/run-tar-dump-posix-header

tar-tests: test/list-tar
tar-tests: test/run-list-tar
tar-tests: test/run-tar-dump-posix-header
tar-tests: test/tar-dump-posix-header

test/list-tar: src/array/buffer.o
test/list-tar: src/buffer_io/buffer_io.o
test/list-tar: src/log/log.o
test/list-tar: src/tar/read.o
test/list-tar: src/tar/test/list-tar.test.o
test/run-list-tar: src/tar/test/list-tar.test.sh
test/run-tar-dump-posix-header: src/tar/test/tar-dump-posix-header.test.sh
test/tar-dump-posix-header: src/array/buffer.o
test/tar-dump-posix-header: src/buffer_io/buffer_io.o
test/tar-dump-posix-header: src/log/log.o
test/tar-dump-posix-header: src/tar/test/tar-dump-posix-header.test.o

tests: tar-tests
