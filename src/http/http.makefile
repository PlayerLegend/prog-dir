test/http_get_open: src/http/test/http_get_open.test.o
test/http_parse_url: src/http/test/http_parse_url.test.o
test/http-cat: src/http/test/http-cat.test.o

test/http-cat test/http_parse_url test/http_get_open: src/network/network.o
test/http-cat test/http_parse_url test/http_get_open: src/http/http.o
test/http-cat test/http_parse_url test/http_get_open: src/log/log.o
test/http-cat test/http_parse_url test/http_get_open: src/buffer_io/buffer_io.o

test/run-chunked-http-cat: src/http/test/chunked-http-cat.test.sh
test/run-identity-http-cat: src/http/test/identity-http-cat.test.sh

TESTS_C += test/http_parse_url test/http_get_open test/http-cat
TESTS_SH += test/run-chunked-http-cat test/run-identity-http-cat

RUN_TESTS += test/run-chunked-http-cat test/run-identity-http-cat
