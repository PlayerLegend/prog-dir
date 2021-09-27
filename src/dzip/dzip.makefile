C_PROGRAMS += bin/dzip bin/dzip-benchmark
SH_PROGRAMS += test/dzip-verify bin/benchmark-compression-utils
RUN_TESTS += test/dzip-verify

dzip-tests: test/dzip-verify bin/dzip-benchmark
dzip-utils: bin/benchmark-compression-utils bin/dzip

bin/benchmark-compression-utils: src/dzip/test/compression-benchmarks.sh
bin/dzip-benchmark: src/array/buffer.o
bin/dzip-benchmark: src/buffer_io/buffer_io.o
bin/dzip-benchmark: src/chain-io/read.o
bin/dzip-benchmark: src/dzip/deflate/deflate.o
bin/dzip-benchmark: src/dzip/test/dzip-benchmark.test.o
bin/dzip-benchmark: src/log/log.o
bin/dzip-benchmark: src/vluint/vluint.o
bin/dzip: src/array/buffer.o
bin/dzip: src/buffer_io/buffer_io.o
bin/dzip: src/chain-io/fd/read.o
bin/dzip: src/chain-io/fd/write.o
bin/dzip: src/chain-io/read.o
bin/dzip: src/chain-io/write.o
bin/dzip: src/dzip/deflate/chain.o
bin/dzip: src/dzip/deflate/deflate.o
bin/dzip: src/dzip/inflate/chain.o
bin/dzip: src/dzip/inflate/extensions.o
bin/dzip: src/dzip/inflate/inflate.o
bin/dzip: src/dzip/util/dzip.util.o
bin/dzip: src/log/log.o
bin/dzip: src/vluint/vluint.o
test/dzip-verify: src/dzip/test/dzip-verify.test.sh

tests: dzip-tests
utils: dzip-utils
