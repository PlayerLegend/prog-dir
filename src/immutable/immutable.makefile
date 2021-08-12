test/immutable: LDLIBS += -lpthread
test/immutable: src/immutable/test/immutable.test.o
test/immutable: src/immutable/immutable.o
test/immutable: src/table2/table.o

TESTS_C += test/immutable
RUN_TESTS += test/immutable
