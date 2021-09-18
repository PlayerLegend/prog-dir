C_PROGRAMS += bin/metahash
C_PROGRAMS += bin/metahash-analyzer
C_PROGRAMS += test/metahash
RUN_TESTS += test/run-metahash
SH_PROGRAMS += test/run-metahash

metahash-tests: test/metahash
metahash-tests: test/run-metahash
metahash-utils: bin/metahash
metahash-utils: bin/metahash-analyzer

test/metahash bin/metahash bin/metahash-analyzer: LDLIBS += -lgcrypt
bin/metahash-analyzer: src/array/buffer.o
bin/metahash-analyzer: src/base16/base16.o
bin/metahash-analyzer: src/base2/base2.o
bin/metahash-analyzer: src/buffer_io/buffer_io.o
bin/metahash-analyzer: src/log/log.o
bin/metahash-analyzer: src/metabase/metabase.o
bin/metahash-analyzer: src/metahash/metahash-analyzer.util.o
bin/metahash-analyzer: src/metahash/metahash.o
bin/metahash-analyzer: src/vluint/vluint.o
bin/metahash: src/array/buffer.o
bin/metahash: src/base16/base16.o
bin/metahash: src/base2/base2.o
bin/metahash: src/buffer_io/buffer_io.o
bin/metahash: src/log/log.o
bin/metahash: src/metabase/metabase.o
bin/metahash: src/metahash/metahash.o
bin/metahash: src/metahash/metahash.util.o
bin/metahash: src/vluint/vluint.o
test/metahash: src/array/buffer.o
test/metahash: src/base16/base16.o
test/metahash: src/base2/base2.o
test/metahash: src/buffer_io/buffer_io.o
test/metahash: src/log/log.o
test/metahash: src/metabase/metabase.o
test/metahash: src/metahash/metahash.o
test/metahash: src/metahash/test/metahash.test.o
test/metahash: src/vluint/vluint.o
test/run-metahash: src/metahash/test/metahash.test.sh

tests: metahash-tests
