C_PROGRAMS += test/libc-strlen-test
C_PROGRAMS += test/system-strlen-test
RUN_TESTS += test/libc-strlen-test
RUN_TESTS += test/system-strlen-test

libc-strlen-tests: test/libc-strlen-test
libc-strlen-tests: test/system-strlen-test
libc-tests: libc-strlen-tests

test/libc-strlen-test: src/libc/strlen/strlen.o
test/libc-strlen-test: src/libc/strlen/test/strlen.test.o
test/libc-strlen-test: src/log/log.o
test/system-strlen-test: src/libc/strlen/test/strlen.test.o
test/system-strlen-test: src/log/log.o

tests: libc-tests
