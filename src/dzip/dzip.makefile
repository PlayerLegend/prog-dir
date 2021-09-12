bin/dzip-benchmark: src/dzip/test/dzip-benchmark.test.o
bin/dzip: src/dzip/dzip.util.o
bin/dzip bin/dzip-benchmark: src/dzip/deflate.o
bin/dzip: src/dzip/inflate.o
bin/dzip bin/dzip-benchmark: src/array/buffer.o
bin/dzip bin/dzip-benchmark: src/buffer_io/buffer_io.o
bin/dzip bin/dzip-benchmark: src/log/log.o
bin/dzip bin/dzip-benchmark: src/vluint/vluint.o
bin/dzip: src/dzip/extensions.o
bin/benchmark-compression-utils: src/dzip/test/compression-benchmarks.sh
test/dzip-verify: src/dzip/test/dzip-verify.test.sh

UTILS_C += bin/dzip
UTILS_SH += bin/benchmark-compression-utils
TESTS_SH += test/dzip-verify
RUN_TESTS += test/dzip-verify
UTILS_C += bin/dzip-benchmark
