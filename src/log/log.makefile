C_PROGRAMS  += test/log
RUN_TESTS += test/log

log-tests: test/log

test/log: src/log/log.o
test/log: src/log/test/log.test.o

tests: log-tests
