C_PROGRAMS += test/base16
RUN_TESTS += test/base16

base16-tests: test/base16

test/base16: src/base16/test/base16.test.o
test/base16: src/base16/base16.o
test/base16: src/log/log.o
test/base16: src/array/buffer.o

tests: base16-tests
