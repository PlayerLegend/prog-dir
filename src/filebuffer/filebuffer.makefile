C_PROGRAMS += bin/filebuffer
SH_PROGRAMS += test/run-filebuffer
RUN_TESTS += test/run-filebuffer

filebuffer-tests: test/run-filebuffer
filebuffer-utils: bin/filebuffer

bin/filebuffer: src/array/buffer.o
bin/filebuffer: src/buffer_io/buffer_io.o
bin/filebuffer: src/filebuffer/filebuffer.o
bin/filebuffer: src/filebuffer/filebuffer.util.o
bin/filebuffer: src/log/log.o
test/run-filebuffer: src/filebuffer/test/filebuffer.test.sh

tests: filebuffer-tests
utils: filebuffer-utils
