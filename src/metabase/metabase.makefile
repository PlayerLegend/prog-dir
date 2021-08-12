METABASE_OBJECTS = src/metabase/metabase.o src/log/log.o src/base16/base16.o src/base2/base2.o

test/metabase-base16: src/metabase/test/base16.test.o $(METABASE_OBJECTS)
test/metabase-base2: src/metabase/test/base2.test.o $(METABASE_OBJECTS)

TESTS_C += test/metabase-base16 test/metabase-base2
RUN_TESTS += test/metabase-base16 test/metabase-base2
