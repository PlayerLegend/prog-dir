C_PROGRAMS += test/libc-strdup-test
C_PROGRAMS += test/system-strdup-test
RUN_TESTS += test/libc-strdup-test
RUN_TESTS += test/system-strdup-test

libc-strdup-tests: test/libc-strdup-test
libc-strdup-tests: test/system-strdup-test
libc-tests: libc-strdup-tests

test/libc-strdup-test: src/libc/strdup/strdup.o
test/libc-strdup-test: src/libc/strdup/test/strdup.test.o
test/libc-strdup-test: src/log/log.o
test/system-strdup-test: src/libc/strdup/test/strdup.test.o
test/system-strdup-test: src/log/log.o

tests: libc-tests
