C_PROGRAMS += test/list
RUN_TESTS += test/list

list-tests: test/list

test/list: src/list/test/list.test.o

tests: list-tests
