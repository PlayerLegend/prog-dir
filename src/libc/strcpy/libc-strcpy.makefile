C_PROGRAMS += test/libc-strcpy-test
C_PROGRAMS += test/system-strcpy-test
RUN_TESTS += test/libc-strcpy-test
RUN_TESTS += test/system-strcpy-test

libc-strcpy-tests: test/libc-strcpy-test
libc-strcpy-tests: test/system-strcpy-test
libc-tests: libc-strcpy-tests

test/libc-strcpy-test: src/libc/strcpy/strcpy.o
test/libc-strcpy-test: src/libc/strcpy/test/strcpy.test.o
test/libc-strcpy-test: src/log/log.o
test/system-strcpy-test: src/libc/strcpy/test/strcpy.test.o
test/system-strcpy-test: src/log/log.o

tests: libc-tests
