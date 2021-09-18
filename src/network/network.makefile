C_PROGRAMS += test/network-server
SH_PROGRAMS += test/run-network-server
RUN_TESTS += test/run-network-server

network-tests: test/network-server test/run-network-server

test/network-server: src/network/test/tcp/server.test.o src/network/network.o src/buffer_io/buffer_io.o src/log/log.o src/array/buffer.o
test/run-network-server: src/network/test/tcp/run-server.test.sh

tests: network-tests
