test/metahash bin/metahash bin/metahash-analyzer: LDLIBS += -lgcrypt
test/metahash bin/metahash bin/metahash-analyzer: src/metahash/metahash.o src/buffer_io/buffer_io.o src/log/log.o src/metabase/metabase.o src/vluint/vluint.o src/base16/base16.o src/base2/base2.o

test/metahash: src/metahash/test/metahash.test.o
bin/metahash: src/metahash/metahash.util.o
bin/metahash-analyzer: src/metahash/metahash-analyzer.util.o

test/run-metahash: src/metahash/test/metahash.test.sh

UTILS_C += bin/metahash bin/metahash-analyzer
TESTS_C += test/metahash
TESTS_SH += test/run-metahash
RUN_TESTS += test/run-metahash
