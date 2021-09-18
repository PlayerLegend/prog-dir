C_PROGRAMS += test/vluint
RUN_TESTS += test/vluint

vluint-tests: test/vluint

test/vluint: src/array/buffer.o
test/vluint: src/base16/base16.o
test/vluint: src/base2/base2.o
test/vluint: src/buffer_io/buffer_io.o
test/vluint: src/log/log.o
test/vluint: src/log/log.o
test/vluint: src/metabase/metabase.o
test/vluint: src/vluint/test/vluint.test.o
test/vluint: src/vluint/vluint.o

tests: vluint-tests

