bin/filebuffer: src/filebuffer/filebuffer.util.o
bin/filebuffer: src/filebuffer/filebuffer.o
bin/filebuffer: src/buffer_io/buffer_io.o
bin/filebuffer: src/log/log.o
bin/filebuffer: src/array/buffer.o
test/run-filebuffer: src/filebuffer/test/filebuffer.test.sh

UTILS_C += bin/filebuffer
TESTS_SH += test/run-filebuffer
RUN_TESTS += test/run-filebuffer
