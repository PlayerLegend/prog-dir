test/buffer_io: src/buffer_io/buffer_io.o
test/buffer_io: src/buffer_io/test/buffer_io.test.o
test/buffer_io: src/log/log.o
test/buffer_io: src/array/buffer.o
test/getline: src/buffer_io/buffer_io.o
test/getline: src/buffer_io/test/getline.test.o
test/getline: src/log/log.o
test/getline: src/array/buffer.o
test/run-buffer_io: src/buffer_io/test/run-buffer_io.test.sh
test/run-getline-sep: src/buffer_io/test/getline-sep.test.sh
test/run-getline: src/buffer_io/test/getline.test.sh

TESTS_C += test/buffer_io test/getline
TESTS_SH += test/run-buffer_io test/run-getline test/run-getline-sep
RUN_TESTS += test/run-buffer_io test/run-getline test/run-getline-sep
