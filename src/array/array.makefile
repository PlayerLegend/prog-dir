C_PROGRAMS += test/range test/buffer
RUN_TESTS += test/range test/buffer

array-tests: test/range test/buffer

test/buffer: src/array/test/buffer.test.o src/array/buffer.o
test/range: src/array/range.o
test/range: src/array/range_atozd.o
test/range: src/array/test/range.test.o

tests: array-tests
