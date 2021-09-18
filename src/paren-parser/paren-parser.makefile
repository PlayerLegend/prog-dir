C_PROGRAMS += test/paren-parser
C_PROGRAMS += test/paren-preprocessor
RUN_TESTS += test/run-paren-parser test/run-paren-preprocessor
SH_PROGRAMS += test/run-paren-parser
SH_PROGRAMS += test/run-paren-preprocessor

paren-parser-tests: test/paren-parser
paren-parser-tests: test/paren-preprocessor
paren-parser-tests: test/run-paren-parser
paren-parser-tests: test/run-paren-preprocessor

test/paren-parser: src/array/buffer.o
test/paren-parser: src/buffer_io/buffer_io.o
test/paren-parser: src/immutable/immutable.o
test/paren-parser: src/log/log.o
test/paren-parser: src/paren-parser/paren-parser.o
test/paren-parser: src/paren-parser/test/paren-parser.test.o
test/paren-parser: src/table/table.o
test/paren-preprocessor: src/array/buffer.o
test/paren-preprocessor: src/buffer_io/buffer_io.o
test/paren-preprocessor: src/immutable/immutable.o
test/paren-preprocessor: src/log/log.o
test/paren-preprocessor: src/paren-parser/paren-parser.o
test/paren-preprocessor: src/paren-parser/paren-preprocessor.o
test/paren-preprocessor: src/paren-parser/test/paren-preprocessor.test.o
test/paren-preprocessor: src/table/table.o
test/run-paren-parser: src/paren-parser/test/paren-parser.test.sh
test/run-paren-preprocessor: src/paren-parser/test/paren-preprocessor.test.sh

tests: paren-parser-tests
