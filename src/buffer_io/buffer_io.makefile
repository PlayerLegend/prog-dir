C_PROGRAMS += test/buffer_io
C_PROGRAMS += test/getline
RUN_TESTS += test/run-buffer_io test/run-getline test/run-getline-sep
SH_PROGRAMS += test/run-buffer_io
SH_PROGRAMS += test/run-getline
SH_PROGRAMS += test/run-getline-sep

buffer-io-tests: test/buffer_io
buffer-io-tests: test/getline
buffer-io-tests: test/run-buffer_io
buffer-io-tests: test/run-getline
buffer-io-tests: test/run-getline-sep

test/buffer_io: src/array/buffer.o
test/buffer_io: src/buffer_io/buffer_io.o
test/buffer_io: src/buffer_io/test/buffer_io.test.o
test/buffer_io: src/log/log.o
test/getline: src/array/buffer.o
test/getline: src/buffer_io/buffer_io.o
test/getline: src/buffer_io/test/getline.test.o
test/getline: src/log/log.o
test/run-buffer_io: src/buffer_io/test/run-buffer_io.test.sh
test/run-getline-sep: src/buffer_io/test/getline-sep.test.sh
test/run-getline: src/buffer_io/test/getline.test.sh

tests: buffer-io-tests
