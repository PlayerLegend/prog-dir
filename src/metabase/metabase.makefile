test/metabase-base16 test/metabase-base2: src/metabase/metabase.o
test/metabase-base16 test/metabase-base2: src/log/log.o
test/metabase-base16 test/metabase-base2: src/base16/base16.o
test/metabase-base16 test/metabase-base2: src/base2/base2.o
test/metabase-base16 test/metabase-base2: src/array/buffer.o

test/metabase-base16: src/metabase/test/base16.test.o
test/metabase-base2: src/metabase/test/base2.test.o

TESTS_C += test/metabase-base16 test/metabase-base2
RUN_TESTS += test/metabase-base16 test/metabase-base2
