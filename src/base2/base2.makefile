C_PROGRAMS += test/base2
RUN_TESTS += test/base2
TESTS_C += test/base2

base2-tests: test/base2

test/base2: src/array/buffer.o
test/base2: src/base2/base2.o
test/base2: src/base2/test/base2.test.o
test/base2: src/log/log.o

tests: base2-tests
