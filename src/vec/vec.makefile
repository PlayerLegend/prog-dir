C_PROGRAMS += test/vec
RUN_TESTS += test/vec

vec-tests: test/vec

test/vec: src/vec/test/vec.test.o

tests: vec-tests

