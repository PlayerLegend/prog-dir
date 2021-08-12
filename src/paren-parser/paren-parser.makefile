test/paren-parser: src/paren-parser/test/paren-parser.test.o
test/paren-parser: src/paren-parser/paren-parser.o
test/paren-parser: src/log/log.o
test/paren-parser: src/immutable/immutable.o
test/paren-parser: src/buffer_io/buffer_io.o
test/paren-parser: src/table2/table.o

test/run-paren-parser: src/paren-parser/test/paren-parser.test.sh
test/paren-preprocessor: src/paren-parser/paren-preprocessor.o
test/paren-preprocessor: src/paren-parser/paren-parser.o
test/paren-preprocessor: src/paren-parser/test/paren-preprocessor.test.o
test/paren-preprocessor: src/log/log.o
test/paren-preprocessor: src/immutable/immutable.o
test/paren-preprocessor: src/buffer_io/buffer_io.o
test/paren-preprocessor: src/table2/table.o

test/run-paren-preprocessor: src/paren-parser/test/paren-preprocessor.test.sh

TESTS_C += test/paren-parser test/paren-preprocessor
TESTS_SH += test/run-paren-parser test/run-paren-preprocessor
RUN_TESTS += test/run-paren-parser test/run-paren-preprocessor
