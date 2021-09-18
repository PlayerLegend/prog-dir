C_PROGRAMS += test/io-wrapper-fd-cat

io-wrapper-tests: test/io-wrapper-fd-cat

test/io-wrapper-fd-cat: src/array/buffer.o
test/io-wrapper-fd-cat: src/buffer_io/buffer_io.o
test/io-wrapper-fd-cat: src/io_wrapper/fd/read.o
test/io-wrapper-fd-cat: src/io_wrapper/fd/write.o
test/io-wrapper-fd-cat: src/io_wrapper/read.o
test/io-wrapper-fd-cat: src/io_wrapper/test/fd-cat.test.o
test/io-wrapper-fd-cat: src/io_wrapper/write.o
test/io-wrapper-fd-cat: src/log/log.o

tests: io-wrapper-tests
