test/json: src/json/test/json.test.o
test/json: src/log/log.o
test/json: src/table2/table.o
test/json: src/array/buffer.o

TESTS_C += test/json
RUN_TESTS += test/json
