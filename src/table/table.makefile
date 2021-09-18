C_PROGRAMS += test/table-string-benchmark
C_PROGRAMS += test/table-string-map
RUN_TESTS += test/run-table-string-map
SH_PROGRAMS += test/run-table-string-map

table-benchmarks: test/table-string-benchmark
table-tests: test/run-table-string-map
table-tests: test/table-string-map

test/run-table-string-map: src/table/test/table-string-map.test.sh
test/table-string-benchmark: src/table/test/table-string-benchmark.test.o
test/table-string-map test/table-string-benchmark: src/array/buffer.o
test/table-string-map test/table-string-benchmark: src/buffer_io/buffer_io.o
test/table-string-map test/table-string-benchmark: src/log/log.o
test/table-string-map test/table-string-benchmark: src/table/table.o
test/table-string-map: src/table/test/table-string-map.test.o

tests: table-tests
benchmarks: table-benchmarks
