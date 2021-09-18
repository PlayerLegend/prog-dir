C_PROGRAMS += test/libc-memcpy-benchmark
C_PROGRAMS += test/libc-memcpy-test
C_PROGRAMS += test/system-memcpy-benchmark
C_PROGRAMS += test/system-memcpy-test
RUN_TESTS += test/libc-memcpy-test
RUN_TESTS += test/system-memcpy-test

libc-memcpy-benchmarks: test/libc-memcpy-benchmark
libc-memcpy-benchmarks: test/system-memcpy-benchmark
libc-memcpy-tests: test/libc-memcpy-test
libc-memcpy-tests: test/system-memcpy-test
libc-tests: libc-memcpy-tests
libc-benchmarks: libc-memcpy-benchmarks

test/libc-memcpy-benchmark: src/libc/memcpy/memcpy.o
test/libc-memcpy-benchmark: src/libc/memcpy/memcpy.o
test/libc-memcpy-benchmark: src/libc/memcpy/test/memcpy.benchmark.o
test/libc-memcpy-benchmark: src/log/log.o
test/libc-memcpy-test: src/libc/memcpy/test/memcpy.test.o
test/libc-memcpy-test: src/log/log.o
test/system-memcpy-benchmark: src/libc/memcpy/test/memcpy.benchmark.o
test/system-memcpy-benchmark: src/log/log.o
test/system-memcpy-test: src/libc/memcpy/test/memcpy.test.o
test/system-memcpy-test: src/log/log.o

benchmarks: libc-benchmarks
tests: libc-tests
