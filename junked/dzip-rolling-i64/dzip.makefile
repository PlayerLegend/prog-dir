bin/dzip-benchmark: src/dzip/test/dzip-benchmark.test.o
bin/dzip-benchmark: src/dzip/deflate.o
bin/dzip-benchmark: src/array/buffer.o
bin/dzip-benchmark: src/buffer_io/buffer_io.o
bin/dzip-benchmark: src/log/log.o
bin/benchmark-compression-utils: src/dzip/test/compression-benchmarks.sh

#UTILS_C += bin/dzip
UTILS_SH += bin/benchmark-compression-utils
BENCHMARKS_C += bin/dzip-benchmark
#TESTS_SH += 
#TESTS_C += bin/dzip-benchmark
