test/system-strcmp-test: src/libc/strcmp/test/strcmp.test.o
test/system-strcmp-test: src/log/log.o

test/libc-strcmp-test: src/libc/strcmp/test/strcmp.test.o
test/libc-strcmp-test: src/libc/strcmp/strcmp.o
test/libc-strcmp-test: src/log/log.o

C_PROGRAMS += test/system-strcmp-test
C_PROGRAMS += test/libc-strcmp-test

RUN_TESTS += test/system-strcmp-test
RUN_TESTS += test/libc-strcmp-test

libc-strcmp-tests: test/system-strcmp-test
libc-strcmp-tests: test/libc-strcmp-test
libc-tests: libc-strcmp-tests
tests: libc-tests
