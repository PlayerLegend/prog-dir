C_PROGRAMS += bin/dzip bin/dzip-benchmark
SH_PROGRAMS += test/dzip-verify bin/benchmark-compression-utils

dzip-tests: test/dzip-verify
dzip-utils: bin/benchmark-compression-utils bin/dzip-benchmark bin/dzip

bin/benchmark-compression-utils: src/dzip/test/compression-benchmarks.sh
bin/dzip-benchmark: src/array/buffer.o
bin/dzip-benchmark: src/buffer_io/buffer_io.o
bin/dzip-benchmark: src/dzip/deflate.o
bin/dzip-benchmark: src/dzip/test/dzip-benchmark.test.o
bin/dzip-benchmark: src/io_wrapper/read.o
bin/dzip-benchmark: src/log/log.o
bin/dzip-benchmark: src/vluint/vluint.o
bin/dzip: src/array/buffer.o
bin/dzip: src/buffer_io/buffer_io.o
bin/dzip: src/dzip/deflate.o
bin/dzip: src/dzip/dzip.util.o
bin/dzip: src/dzip/extensions.o
bin/dzip: src/dzip/inflate.o
bin/dzip: src/io_wrapper/read.o
bin/dzip: src/log/log.o
bin/dzip: src/vluint/vluint.o
test/dzip-verify: src/dzip/test/dzip-verify.test.sh

tests: dzip-tests
utils: dzip-utils
