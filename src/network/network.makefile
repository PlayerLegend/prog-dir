test/network-server: src/network/test/tcp/server.test.o src/network/network.o src/buffer_io/buffer_io.o src/log/log.o
test/run-network-server: src/network/test/tcp/run-server.test.sh

TESTS_C += test/network-server
TESTS_SH += test/run-network-server

RUN_TESTS += test/run-network-server
