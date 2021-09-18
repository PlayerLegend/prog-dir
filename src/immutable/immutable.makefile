C_PROGRAMS += test/immutable
RUN_TESTS += test/immutable

immutable-tests: test/immutable

test/immutable: LDLIBS += -lpthread
test/immutable: src/immutable/immutable.o
test/immutable: src/immutable/test/immutable.test.o
test/immutable: src/table/table.o

tests: immutable-tests
