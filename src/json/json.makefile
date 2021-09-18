C_PROGRAMS += test/json
RUN_TESTS += test/json

json-tests: test/json

test/json: src/array/buffer.o
test/json: src/json/test/json.test.o
test/json: src/log/log.o
test/json: src/table/table.o

tests: json-tests
