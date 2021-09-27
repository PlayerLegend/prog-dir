C_PROGRAMS += test/chain-io-fd-cat
SH_PROGRAMS += test/run-chain-io-fd-cat
RUN_TESTS += test/run-chain-io-fd-cat

chain-io-fd-tests: test/run-chain-io-fd-cat
chain-io-fd-tests: test/chain-io-fd-cat
chain-io-tests: chain-io-fd-tests

test/chain-io-fd-cat: src/array/buffer.o
test/chain-io-fd-cat: src/buffer_io/buffer_io.o
test/chain-io-fd-cat: src/chain-io/fd/read.o
test/chain-io-fd-cat: src/chain-io/fd/write.o
test/chain-io-fd-cat: src/chain-io/read.o
test/chain-io-fd-cat: src/chain-io/fd/test/fd-cat.test.o
test/chain-io-fd-cat: src/chain-io/write.o
test/chain-io-fd-cat: src/log/log.o
test/run-chain-io-fd-cat: src/chain-io/fd/test/run-fd-cat.test.sh

tests: chain-io-tests
