LD = $(CC)
PREFIX ?= /usr/local
DESTDIR ?= $(PREFIX)
CFLAGS += -std=c99
PATH := $(CURDIR)/bin/:$(CURDIR)/sbin/:$(PATH)
BUILD_ENV ?= release

export PATH

all: utils

.PHONY: test bin install

include env/$(BUILD_ENV).makefile

SUB_MAKEFILES != find src -type f -name '*.makefile'
include $(SUB_MAKEFILES)

#debug tests test: CFLAGS += -ggdb -Wall -Werror
#utils benchmarks: CFLAGS += -DNDEBUG -O2
#benchmarks: CFLAGS += -O2

#utils debug: $(UTILS_C) $(UTILS_SH)
#debug: $(TESTS_C) $(TESTS_SH)
#benchmarks: $(BENCHMARKS_C)

#$(UTILS_C) $(TESTS_C): LDLIBS += $(CFLAGS)

$(C_PROGRAMS):
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $> $(LDLIBS)

$(SH_PROGRAMS):
	@mkdir -p $(@D)
	cp $< $@
	chmod +x $@

run-tests:
	make depend
	make clean
	make BUILD_ENV=debug tests utils
	sh run-tests.sh $(RUN_TESTS)
	make clean

clean-fast:
	rm -rf test bin sbin init `find src -name '*.o'`

clean: clean-fast
	rm -rf external boot

depend: clean
	makedepend -Y `find src -name '*.c*' -or -name '*.h*'`

install: utils
	printf '%s\n' $(UTILS_C) $(UTILS_SH) | cpio -pudm $(DESTDIR)/

install-user:
	make -j12 install DESTDIR=$(HOME)/.local

# DO NOT DELETE

src/network/network.o: src/array/range.h src/array/buffer.h
src/network/network.o: src/keyargs/keyargs.h
src/network/test/tcp/server.test.o: src/keyargs/keyargs.h src/test/debug.h
src/network/test/tcp/server.test.o: src/array/range.h src/array/buffer.h
src/network/test/tcp/server.test.o: src/network/network.h
src/network/test/tcp/server.test.o: src/buffer_io/buffer_io.h src/log/log.h
src/network/network.o: src/keyargs/keyargs.h src/array/range.h
src/network/network.o: src/array/buffer.h src/log/log.h src/network/network.h
src/io_wrapper/fd/read.o: src/array/range.h src/array/buffer.h
src/io_wrapper/fd/read.o: src/keyargs/keyargs.h src/io_wrapper/common.h
src/io_wrapper/fd/read.o: src/io_wrapper/read.h
src/io_wrapper/fd/write.o: src/array/range.h src/array/buffer.h
src/io_wrapper/fd/write.o: src/keyargs/keyargs.h src/io_wrapper/common.h
src/io_wrapper/fd/write.o: src/io_wrapper/write.h
src/io_wrapper/fd/read.o: src/array/range.h src/array/buffer.h
src/io_wrapper/fd/read.o: src/keyargs/keyargs.h src/io_wrapper/common.h
src/io_wrapper/fd/read.o: src/io_wrapper/read.h src/io_wrapper/fd/read.h
src/io_wrapper/fd/read.o: src/buffer_io/buffer_io.h
src/io_wrapper/fd/write.o: src/array/range.h src/array/buffer.h
src/io_wrapper/fd/write.o: src/keyargs/keyargs.h src/io_wrapper/common.h
src/io_wrapper/fd/write.o: src/io_wrapper/write.h src/io_wrapper/fd/write.h
src/io_wrapper/fd/write.o: src/buffer_io/buffer_io.h
src/io_wrapper/write.o: src/array/range.h src/array/buffer.h
src/io_wrapper/write.o: src/keyargs/keyargs.h src/io_wrapper/common.h
src/io_wrapper/read.o: src/array/range.h src/array/buffer.h
src/io_wrapper/read.o: src/keyargs/keyargs.h src/io_wrapper/common.h
src/io_wrapper/read.o: src/array/range.h src/array/buffer.h
src/io_wrapper/read.o: src/keyargs/keyargs.h src/io_wrapper/common.h
src/io_wrapper/read.o: src/io_wrapper/fd/read.h
src/io_wrapper/write.o: src/array/range.h src/array/buffer.h
src/io_wrapper/write.o: src/keyargs/keyargs.h src/io_wrapper/common.h
src/io_wrapper/write.o: src/io_wrapper/fd/write.h src/libc/string.h
src/io_wrapper/test/fd-cat.test.o: src/array/range.h src/array/buffer.h
src/io_wrapper/test/fd-cat.test.o: src/keyargs/keyargs.h
src/io_wrapper/test/fd-cat.test.o: src/io_wrapper/common.h
src/io_wrapper/test/fd-cat.test.o: src/io_wrapper/read.h
src/io_wrapper/test/fd-cat.test.o: src/io_wrapper/write.h
src/io_wrapper/test/fd-cat.test.o: src/io_wrapper/fd/read.h
src/io_wrapper/test/fd-cat.test.o: src/io_wrapper/fd/write.h src/log/log.h
src/immutable/immutable.o: src/immutable/immutable.h src/array/range.h
src/immutable/immutable.o: src/table/table.h src/table/table-string.h
src/immutable/test/immutable.test.o: src/immutable/immutable.h
src/immutable/test/immutable.test.o: src/test/debug.h
src/metahash/metahash.o: src/array/range.h src/array/buffer.h
src/metahash/metahash.o: src/keyargs/keyargs.h src/vluint/vluint.h
src/metahash/metahash.util.o: src/array/range.h src/array/buffer.h
src/metahash/metahash.util.o: src/keyargs/keyargs.h src/vluint/vluint.h
src/metahash/metahash.util.o: src/metahash/metahash.h src/metabase/metabase.h
src/metahash/metahash.util.o: src/buffer_io/buffer_io.h src/log/log.h
src/metahash/metahash.util.o: src/libc/string.h
src/metahash/test/metahash.test.o: src/array/range.h src/array/buffer.h
src/metahash/test/metahash.test.o: src/keyargs/keyargs.h src/vluint/vluint.h
src/metahash/test/metahash.test.o: src/metahash/metahash.h
src/metahash/test/metahash.test.o: src/metabase/metabase.h
src/metahash/test/metahash.test.o: src/buffer_io/buffer_io.h src/log/log.h
src/metahash/metahash-analyzer.util.o: src/array/range.h src/array/buffer.h
src/metahash/metahash-analyzer.util.o: src/keyargs/keyargs.h
src/metahash/metahash-analyzer.util.o: src/vluint/vluint.h
src/metahash/metahash-analyzer.util.o: src/metahash/metahash.h
src/metahash/metahash-analyzer.util.o: src/metabase/metabase.h
src/metahash/metahash-analyzer.util.o: src/buffer_io/buffer_io.h
src/metahash/metahash-analyzer.util.o: src/log/log.h src/libc/string.h
src/metahash/metahash.o: src/keyargs/keyargs.h src/array/range.h
src/metahash/metahash.o: src/array/buffer.h src/buffer_io/buffer_io.h
src/metahash/metahash.o: src/vluint/vluint.h src/metahash/metahash.h
src/metahash/metahash.o: src/log/log.h
src/dzip/deflate.o: src/keyargs/keyargs.h src/array/range.h
src/dzip/deflate.o: src/array/buffer.h src/io_wrapper/common.h
src/dzip/deflate.o: src/io_wrapper/read.h src/dzip/dzip.h src/dzip/internal.h
src/dzip/deflate.o: src/log/log.h src/vluint/vluint.h src/libc/string.h
src/dzip/inflate.o: src/keyargs/keyargs.h src/array/range.h
src/dzip/inflate.o: src/array/buffer.h src/io_wrapper/common.h
src/dzip/inflate.o: src/io_wrapper/read.h src/dzip/dzip.h src/dzip/internal.h
src/dzip/inflate.o: src/log/log.h src/vluint/vluint.h src/libc/string.h
src/dzip/dzip.o: src/keyargs/keyargs.h src/array/range.h src/array/buffer.h
src/dzip/dzip.o: src/io_wrapper/read.h
src/dzip/dzip.util.o: src/array/range.h src/array/buffer.h
src/dzip/dzip.util.o: src/keyargs/keyargs.h src/io_wrapper/common.h
src/dzip/dzip.util.o: src/io_wrapper/read.h src/dzip/dzip.h
src/dzip/dzip.util.o: src/buffer_io/buffer_io.h src/log/log.h
src/dzip/extensions.o: src/keyargs/keyargs.h src/array/range.h
src/dzip/extensions.o: src/array/buffer.h src/io_wrapper/common.h
src/dzip/extensions.o: src/io_wrapper/read.h src/dzip/dzip.h
src/dzip/extensions.o: src/buffer_io/buffer_io.h
src/dzip/test/dzip-benchmark.test.o: src/array/range.h src/array/buffer.h
src/dzip/test/dzip-benchmark.test.o: src/keyargs/keyargs.h
src/dzip/test/dzip-benchmark.test.o: src/buffer_io/buffer_io.h
src/dzip/test/dzip-benchmark.test.o: src/io_wrapper/common.h
src/dzip/test/dzip-benchmark.test.o: src/io_wrapper/read.h src/dzip/dzip.h
src/dzip/test/dzip-benchmark.test.o: src/log/log.h
src/url/url.o: src/array/range.h src/array/buffer.h
src/url/url.o: src/array/range.h src/array/buffer.h src/url/url.h
src/url/url.o: src/log/log.h
src/array/buffer.o: src/array/range.h
src/array/test/buffer.test.o: src/array/range.h src/array/buffer.h
src/array/test/buffer.test.o: src/test/debug.h
src/array/test/range.test.o: src/array/range.h src/test/debug.h
src/array/buffer.o: src/array/range.h src/array/buffer.h
src/tutorial/ffmpeg-video-player/player.o: src/log/log.h
src/pkg/pkg.o: src/keyargs/keyargs.h
src/pkg/pkg-install.o: src/keyargs/keyargs.h src/pkg/pkg.h src/array/range.h
src/pkg/pkg-install.o: src/array/buffer.h src/dzip/internal.h
src/pkg/pkg-install.o: src/tar/common.h src/tar/read.h src/log/log.h
src/pkg/pkg-install.o: src/path/path.h src/buffer_io/buffer_io.h
src/pkg/pkg-pack.util.o: src/keyargs/keyargs.h src/pkg/pkg.h src/log/log.h
src/pkg/pkg-update.o: src/pkg/pkg-update.h src/immutable/immutable.h
src/pkg/pkg-update.o: src/keyargs/keyargs.h src/paren-parser/paren-parser.h
src/pkg/pkg-update.o: src/paren-parser/paren-preprocessor.h
src/pkg/internal.o: src/keyargs/keyargs.h src/pkg/pkg.h src/array/range.h
src/pkg/internal.o: src/array/buffer.h
src/pkg/pkg-install.util.o: src/keyargs/keyargs.h src/pkg/pkg.h src/log/log.h
src/pkg/pkg-manifest.o: src/array/range.h src/array/buffer.h src/list/list.h
src/pkg/pkg-manifest.o: src/keyargs/keyargs.h src/pkg/pkg-manifest.h
src/pkg/pkg-pack.o: src/keyargs/keyargs.h src/pkg/pkg.h src/array/range.h
src/pkg/pkg-pack.o: src/array/buffer.h src/tar/common.h src/tar/write.h
src/pkg/pkg-pack.o: src/log/log.h src/buffer_io/buffer_io.h src/dzip/dzip.h
src/pkg/pkg-pack.o: src/path/path.h
src/pkg/pkg-manifest.o: src/array/range.h src/array/buffer.h src/list/list.h
src/pkg/pkg-manifest.o: src/keyargs/keyargs.h
src/pkg/pkg-root.o: src/keyargs/keyargs.h src/pkg/pkg.h src/array/range.h
src/pkg/pkg-root.o: src/array/buffer.h src/dzip/internal.h
src/pkg/pkg-root.o: src/buffer_io/buffer_io.h src/path/path.h src/log/log.h
src/pkg/pkg-root.o: src/immutable/immutable.h src/paren-parser/paren-parser.h
src/pkg/pkg-root.o: src/paren-parser/paren-preprocessor.h
src/kademlia/kademlia.o: src/list/list.h
src/paren-parser/paren-parser.o: src/immutable/immutable.h
src/paren-parser/paren-parser.o: src/keyargs/keyargs.h
src/paren-parser/test/paren-parser.test.o: src/keyargs/keyargs.h
src/paren-parser/test/paren-parser.test.o: src/immutable/immutable.h
src/paren-parser/test/paren-parser.test.o: src/paren-parser/paren-parser.h
src/paren-parser/test/paren-parser.test.o: src/array/range.h
src/paren-parser/test/paren-parser.test.o: src/array/buffer.h
src/paren-parser/test/paren-parser.test.o: src/buffer_io/buffer_io.h
src/paren-parser/test/paren-preprocessor.test.o: src/keyargs/keyargs.h
src/paren-parser/test/paren-preprocessor.test.o: src/immutable/immutable.h
src/paren-parser/test/paren-preprocessor.test.o: src/paren-parser/paren-parser.h
src/paren-parser/test/paren-preprocessor.test.o: src/paren-parser/paren-preprocessor.h
src/paren-parser/test/paren-preprocessor.test.o: src/array/range.h
src/paren-parser/test/paren-preprocessor.test.o: src/array/buffer.h
src/paren-parser/test/paren-preprocessor.test.o: src/buffer_io/buffer_io.h
src/paren-parser/paren-preprocessor.o: src/immutable/immutable.h
src/paren-parser/paren-preprocessor.o: src/keyargs/keyargs.h
src/paren-parser/paren-preprocessor.o: src/paren-parser/paren-parser.h
src/paren-parser/paren-parser.o: src/immutable/immutable.h
src/paren-parser/paren-parser.o: src/keyargs/keyargs.h
src/paren-parser/paren-parser.o: src/paren-parser/paren-parser.h
src/paren-parser/paren-parser.o: src/log/log.h src/array/range.h
src/paren-parser/paren-parser.o: src/array/buffer.h
src/paren-parser/paren-preprocessor.o: src/log/log.h src/array/range.h
src/paren-parser/paren-preprocessor.o: src/array/buffer.h
src/paren-parser/paren-preprocessor.o: src/immutable/immutable.h
src/paren-parser/paren-preprocessor.o: src/keyargs/keyargs.h
src/paren-parser/paren-preprocessor.o: src/paren-parser/paren-parser.h
src/paren-parser/paren-preprocessor.o: src/paren-parser/paren-preprocessor.h
src/hash-fs/internal.o: src/array/range.h src/array/buffer.h
src/hash-fs/internal.o: src/hash-fs/hash-fs.h src/keyargs/keyargs.h
src/hash-fs/internal.o: src/vluint/vluint.h src/metahash/metahash.h
src/hash-fs/hash-fs.o: src/array/range.h src/array/buffer.h
src/hash-fs/hash-fs.o: src/hash-fs/hash-fs.h src/keyargs/keyargs.h
src/hash-fs/hash-fs.o: src/vluint/vluint.h src/metahash/metahash.h
src/hash-fs/hash-fs.o: src/dzip/internal.h src/log/log.h
src/filestore/filestore-add.util.o: src/keyargs/keyargs.h src/array/range.h
src/filestore/filestore-add.util.o: src/array/buffer.h
src/filestore/filestore-add.util.o: src/buffer_io/buffer_io.h src/log/log.h
src/filestore/filestore-add.util.o: src/vluint/vluint.h
src/filestore/filestore-add.util.o: src/metabase/metabase.h
src/filestore/filestore-add.util.o: src/metahash/metahash.h
src/filestore/filestore-add.util.o: src/filestore/filestore.h
src/filestore/filestore-path.util.o: src/keyargs/keyargs.h src/array/range.h
src/filestore/filestore-path.util.o: src/array/buffer.h
src/filestore/filestore-path.util.o: src/buffer_io/buffer_io.h src/log/log.h
src/filestore/filestore-path.util.o: src/vluint/vluint.h
src/filestore/filestore-path.util.o: src/metabase/metabase.h
src/filestore/filestore-path.util.o: src/metahash/metahash.h
src/filestore/filestore-path.util.o: src/filestore/filestore.h
src/filestore/filestore.o: src/log/log.h src/array/range.h src/array/buffer.h
src/filestore/filestore.o: src/keyargs/keyargs.h src/buffer_io/buffer_io.h
src/filestore/filestore.o: src/base16/base16.h src/vluint/vluint.h
src/filestore/filestore.o: src/metabase/metabase.h src/metahash/metahash.h
src/filestore/filestore.o: src/filestore/filestore.h
src/filestore/filestore.o: src/array/range.h src/array/buffer.h
src/filestore/filestore.o: src/keyargs/keyargs.h src/vluint/vluint.h
src/filestore/filestore.o: src/metabase/metabase.h src/metahash/metahash.h
src/libc/memcpy/test/memcpy.test.o: src/test/debug.h src/log/log.h
src/libc/memcpy/test/memcpy.benchmark.o: src/benchmark/benchmark.h
src/libc/memcpy/test/memcpy.benchmark.o: src/log/log.h
src/libc/memcpy/memcpy.o: src/libc/string.h
src/libc/strcmp/test/strcmp.test.o: src/test/debug.h src/log/log.h
src/libc/strlen/test/strlen.test.o: src/test/debug.h src/log/log.h
src/table/table.o: src/array/range.h src/array/buffer.h src/table/table.h
src/table/table.o: src/table/table-string.h
src/table/table.o: src/array/range.h
src/table/table-string.o: src/array/range.h src/array/buffer.h
src/table/table-string.o: src/table/table.h
src/table/table-int.o: src/array/range.h src/array/buffer.h src/table/table.h
src/table/test/table-string-map.test.o: src/array/range.h src/array/buffer.h
src/table/test/table-string-map.test.o: src/table/table.h
src/table/test/table-string-map.test.o: src/table/table-string.h
src/table/test/table-string-map.test.o: src/log/log.h src/keyargs/keyargs.h
src/table/test/table-string-map.test.o: src/buffer_io/buffer_io.h
src/table/test/table-string-benchmark.test.o: src/array/range.h
src/table/test/table-string-benchmark.test.o: src/array/buffer.h
src/table/test/table-string-benchmark.test.o: src/table/table.h
src/table/test/table-string-benchmark.test.o: src/table/table-string.h
src/table/test/table-string-benchmark.test.o: src/keyargs/keyargs.h
src/table/test/table-string-benchmark.test.o: src/buffer_io/buffer_io.h
src/table/test/table-string-benchmark.test.o: src/log/log.h
src/blkd/blkd.o: src/blkd/blkd.h src/log/log.h
src/blkd/test/blkd.test.o: src/blkd/blkd.h
src/blkd/blkd-cache.o: src/blkd/blkd.h
src/blkd/blkd-cache-io.o: src/blkd/blkd.h
src/blkd/blkd-cache.o: src/array/range.h src/array/buffer.h src/list/list.h
src/blkd/blkd-cache.o: src/blkd/blkd.h src/blkd/blkd-cache-io.h
src/blkd/blkd-cache.o: src/table/table.h
src/blkd/blkd-cache-io.o: src/array/range.h src/list/list.h src/blkd/blkd.h
src/blkd/blkd-cache-io.o: src/blkd/blkd-cache-io.h src/table/table.h
src/blkd/blkd-fuse.o: src/blkd/blkd.h
src/blkd/blkd-direct-io.o: src/blkd/blkd.h
src/buffer_stream/buffer_stream.o: src/array/range.h src/array/buffer.h
src/file/file.o: src/array/range.h src/array/buffer.h src/file/file.h
src/file/test/file.test.o: src/array/range.h src/array/buffer.h
src/file/test/file.test.o: src/file/file.h src/test/debug.h
src/file/file.o: src/array/range.h src/array/buffer.h
src/tar/read.o: src/array/range.h src/array/buffer.h src/keyargs/keyargs.h
src/tar/read.o: src/buffer_io/buffer_io.h src/io_wrapper/common.h
src/tar/read.o: src/io_wrapper/fd/read.h src/log/log.h
src/tar/read.o: src/tar/internal/spec.h
src/tar/write.o: src/array/range.h src/array/buffer.h src/keyargs/keyargs.h
src/tar/write.o: src/buffer_io/buffer_io.h src/io_wrapper/common.h
src/tar/write.o: src/io_wrapper/fd/write.h src/tar/internal/spec.h
src/tar/write.o: src/log/log.h src/path/path.h
src/tar/test/list-tar.test.o: src/array/range.h src/array/buffer.h
src/tar/test/list-tar.test.o: src/keyargs/keyargs.h src/buffer_io/buffer_io.h
src/tar/test/list-tar.test.o: src/tar/internal/spec.h src/log/log.h
src/tar/test/list-tar.test.o: src/io_wrapper/common.h src/io_wrapper/read.h
src/tar/test/tar-dump-posix-header.test.o: src/keyargs/keyargs.h
src/tar/test/tar-dump-posix-header.test.o: src/array/range.h
src/tar/test/tar-dump-posix-header.test.o: src/array/buffer.h
src/tar/test/tar-dump-posix-header.test.o: src/buffer_io/buffer_io.h
src/tar/test/tar-dump-posix-header.test.o: src/tar/internal/spec.h
src/tar/test/tar-dump-posix-header.test.o: src/log/log.h
src/tar/read.o: src/array/range.h src/array/buffer.h src/keyargs/keyargs.h
src/tar/read.o: src/io_wrapper/common.h
src/tar/write.o: src/array/range.h src/array/buffer.h src/keyargs/keyargs.h
src/tar/write.o: src/io_wrapper/common.h
src/vluint/vluint.o: src/keyargs/keyargs.h src/array/range.h
src/vluint/vluint.o: src/array/buffer.h src/vluint/vluint.h src/log/log.h
src/vluint/test/vluint.test.o: src/keyargs/keyargs.h src/array/range.h
src/vluint/test/vluint.test.o: src/array/buffer.h src/vluint/vluint.h
src/vluint/test/vluint.test.o: src/metabase/metabase.h
src/vluint/test/vluint.test.o: src/buffer_io/buffer_io.h src/log/log.h
src/vluint/vluint.o: src/keyargs/keyargs.h src/array/range.h
src/vluint/vluint.o: src/array/buffer.h
src/pipe-program/pipe-program.o: src/pipe-program/pipe-program.h
src/pipe-program/pipe-program.o: src/log/log.h
src/list/test/list.test.o: src/list/list.h src/test/debug.h
src/json/json.o: src/array/range.h src/json/json.h src/array/buffer.h
src/json/json.o: src/log/log.h src/list/list.h src/table/table.h
src/json/json.o: src/table/table-string.h
src/json/json.o: src/array/range.h
src/json/test/json.test.o: src/json/json.c src/array/range.h src/json/json.h
src/json/test/json.test.o: src/array/buffer.h src/log/log.h src/list/list.h
src/json/test/json.test.o: src/table/table.h src/table/table-string.h
src/metabase/metabase.o: src/array/range.h src/array/buffer.h
src/metabase/metabase.o: src/metabase/metabase.h src/log/log.h
src/metabase/metabase.o: src/base16/base16.h src/base2/base2.h
src/metabase/metabase.o: src/array/range.h src/array/buffer.h
src/metabase/mbti.o: src/array/range.h src/array/buffer.h
src/metabase/mbti.o: src/metabase/metabase.h src/log/log.h
src/metabase/mbti.o: src/buffer_io/buffer_io.h
src/metabase/test/base16.test.o: src/test/debug.h src/array/range.h
src/metabase/test/base16.test.o: src/array/buffer.h src/metabase/metabase.h
src/metabase/test/base16.test.o: src/log/log.h
src/metabase/test/base2.test.o: src/test/debug.h src/array/range.h
src/metabase/test/base2.test.o: src/array/buffer.h src/metabase/metabase.h
src/metabase/test/base2.test.o: src/log/log.h
src/base16/base16.o: src/array/range.h src/array/buffer.h
src/base16/test/base16.test.o: src/array/range.h src/array/buffer.h
src/base16/test/base16.test.o: src/base16/base16.h src/log/log.h
src/base16/base16.o: src/array/range.h src/array/buffer.h src/base16/base16.h
src/base16/base16.o: src/log/log.h
src/path/path.o: src/array/range.h src/array/buffer.h src/path/path.h
src/path/path.o: src/log/log.h src/keyargs/keyargs.h
src/path/path.o: src/buffer_io/buffer_io.h
src/path/path.o: src/array/range.h src/array/buffer.h
src/log/log.o: src/log/log.h
src/log/test/log.test.o: src/log/log.h src/test/debug.h
src/vec/vec3.o: src/vec/vec.h
src/vec/vec2.o: src/vec/vec.h
src/vec/mat4.o: src/vec/vec.h src/vec/vec3.h src/vec/vec4.h src/vec/mat4.h
src/vec/test/vec.test.o: src/vec/vec.h src/vec/vec2.h src/vec/vec3.h
src/vec/test/vec.test.o: src/test/debug.h
src/vec/mat4.o: src/vec/vec.h src/vec/vec3.h
src/vec/vec4.o: src/vec/vec.h
src/manifest/manifest.o: src/array/range.h src/array/buffer.h
src/manifest/manifest.o: src/array/range.h src/array/buffer.h
src/manifest/manifest.o: src/manifest/manifest.h
src/filebuffer/filebuffer.o: src/array/range.h src/array/buffer.h
src/filebuffer/filebuffer.o: src/keyargs/keyargs.h src/buffer_io/buffer_io.h
src/filebuffer/filebuffer.o: src/filebuffer/filebuffer.h
src/filebuffer/filebuffer.o: src/array/range.h src/array/buffer.h
src/filebuffer/filebuffer.util.o: src/array/range.h src/array/buffer.h
src/filebuffer/filebuffer.util.o: src/keyargs/keyargs.h
src/filebuffer/filebuffer.util.o: src/buffer_io/buffer_io.h
src/filebuffer/filebuffer.util.o: src/filebuffer/filebuffer.h
src/buffer_io/buffer_io.o: src/keyargs/keyargs.h src/array/range.h
src/buffer_io/buffer_io.o: src/array/buffer.h src/buffer_io/buffer_io.h
src/buffer_io/buffer_io.o: src/log/log.h
src/buffer_io/buffer_io.o: src/array/range.h src/array/buffer.h
src/buffer_io/buffer_io.o: src/keyargs/keyargs.h
src/buffer_io/test/getline.test.o: src/test/debug.h src/keyargs/keyargs.h
src/buffer_io/test/getline.test.o: src/array/range.h src/array/buffer.h
src/buffer_io/test/getline.test.o: src/buffer_io/buffer_io.h
src/buffer_io/test/buffer_io.test.o: src/test/debug.h src/keyargs/keyargs.h
src/buffer_io/test/buffer_io.test.o: src/array/range.h src/array/buffer.h
src/buffer_io/test/buffer_io.test.o: src/buffer_io/buffer_io.h
src/base2/test/base2.test.o: src/array/range.h src/array/buffer.h
src/base2/test/base2.test.o: src/base2/base2.h src/log/log.h
src/base2/base2.o: src/array/range.h src/array/buffer.h
src/base2/base2.o: src/array/range.h src/array/buffer.h src/base2/base2.h
src/base2/base2.o: src/log/log.h
src/http/http.o: src/array/range.h src/array/buffer.h
src/http/test/http_parse_url.test.o: src/keyargs/keyargs.h src/array/range.h
src/http/test/http_parse_url.test.o: src/array/buffer.h src/http/http.h
src/http/test/http-cat.test.o: src/array/range.h src/array/buffer.h
src/http/test/http-cat.test.o: src/keyargs/keyargs.h src/http/http.h
src/http/test/http-cat.test.o: src/network/network.h src/log/log.h
src/http/test/http_get_open.test.o: src/array/range.h src/array/buffer.h
src/http/test/http_get_open.test.o: src/keyargs/keyargs.h src/http/http.h
src/http/test/http_get_open.test.o: src/network/network.h src/log/log.h
src/http/http.o: src/array/range.h src/array/buffer.h src/keyargs/keyargs.h
src/http/http.o: src/buffer_io/buffer_io.h src/http/http.h src/log/log.h
src/http/http.o: src/network/network.h
