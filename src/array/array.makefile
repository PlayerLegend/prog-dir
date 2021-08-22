test/range: src/array/test/range.test.o
test/buffer: src/array/test/buffer.test.o src/array/buffer.o

TESTS_C += test/range test/buffer
RUN_TESTS += test/range test/buffer
