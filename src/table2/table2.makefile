test/table-string-map: src/table2/test/table-string-map.test.o
test/table-string-benchmark: src/table2/test/table-string-benchmark.test.o

test/table-string-map test/table-string-benchmark: src/table2/table.o
test/table-string-map test/table-string-benchmark: src/buffer_io/buffer_io.o
test/table-string-map test/table-string-benchmark: src/log/log.o
test/table-string-map test/table-string-benchmark: src/array/buffer.o

test/run-table-string-map: src/table2/test/table-string-map.test.sh

TESTS_C += test/table-string-map
TESTS_SH += test/run-table-string-map
RUN_TESTS += test/run-table-string-map
BENCHMARKS_C += test/table-string-benchmark
