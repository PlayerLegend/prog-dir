LD = $(CC)
PREFIX ?= /usr/local
DESTDIR ?= $(PREFIX)
CFLAGS += -std=c99
PATH := $(CURDIR)/bin/:$(CURDIR)/sbin/:$(PATH)

export PATH

all: utils

.PHONY: test bin debug utils install

#include src/*/*.makefile
SUB_MAKEFILES != find src -type f -name '*.makefile'
include $(SUB_MAKEFILES)

debug tests test: CFLAGS += -ggdb -Wall -Werror
utils benchmarks: CFLAGS += -DNDEBUG

utils debug: $(UTILS_C) $(UTILS_SH)
debug: $(TESTS_C) $(TESTS_SH)
benchmarks: $(BENCHMARKS_C)

$(UTILS_C) $(TESTS_C) $(BENCHMARKS_C):
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $> $(LDLIBS)

$(UTILS_SH) $(TESTS_SH):
	@mkdir -p $(@D)
	cp $< $@
	chmod +x $@

test:
	make depend
	make clean
	make debug
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

# DO NOT DELETE

src/network/network.o: src/array/range.h src/array/buffer.h
src/network/network.o: src/keyargs/keyargs.h
src/network/test/tcp/server.test.o: src/keyargs/keyargs.h src/test/debug.h
src/network/test/tcp/server.test.o: src/array/range.h src/array/buffer.h
src/network/test/tcp/server.test.o: src/network/network.h
src/network/test/tcp/server.test.o: src/buffer_io/buffer_io.h src/log/log.h
src/network/network.o: src/keyargs/keyargs.h src/array/range.h
src/network/network.o: src/array/buffer.h src/log/log.h src/network/network.h
src/immutable/immutable.o: src/immutable/immutable.h src/array/range.h
src/immutable/immutable.o: src/table2/table.h src/table2/table-string.h
src/immutable/test/immutable.test.o: src/immutable/immutable.h
src/immutable/test/immutable.test.o: src/test/debug.h
src/metahash/metahash.o: src/array/range.h src/array/buffer.h
src/metahash/metahash.o: src/keyargs/keyargs.h src/vluint/vluint.h
src/metahash/metahash.util.o: src/array/range.h src/array/buffer.h
src/metahash/metahash.util.o: src/keyargs/keyargs.h src/vluint/vluint.h
src/metahash/metahash.util.o: src/metahash/metahash.h src/metabase/metabase.h
src/metahash/metahash.util.o: src/buffer_io/buffer_io.h src/log/log.h
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
src/metahash/metahash-analyzer.util.o: src/log/log.h
src/metahash/metahash.o: src/keyargs/keyargs.h src/array/range.h
src/metahash/metahash.o: src/array/buffer.h src/buffer_io/buffer_io.h
src/metahash/metahash.o: src/vluint/vluint.h src/metahash/metahash.h
src/metahash/metahash.o: src/log/log.h
src/dzip/dzip.o: src/array/range.h src/array/buffer.h src/keyargs/keyargs.h
src/dzip/inflate.old.o: src/array/range.h src/array/buffer.h
src/dzip/inflate.old.o: src/keyargs/keyargs.h src/dzip/dzip.h
src/dzip/inflate.old.o: src/vluint/vluint.h src/dzip/internal-shared.h
src/dzip/inflate.old.o: src/log/log.h
src/dzip/test/dzip-deflate.test.o: src/array/range.h src/array/buffer.h
src/dzip/test/dzip-deflate.test.o: src/keyargs/keyargs.h
src/dzip/test/dzip-deflate.test.o: src/buffer_io/buffer_io.h src/dzip/dzip.h
src/dzip/dzip.util.o: src/array/range.h src/array/buffer.h
src/dzip/dzip.util.o: src/keyargs/keyargs.h src/dzip/dzip.h
src/dzip/dzip.util.o: src/buffer_io/buffer_io.h src/log/log.h
src/dzip/deflate.o: src/array/range.h src/array/buffer.h
src/dzip/deflate.o: src/keyargs/keyargs.h src/dzip/dzip.h src/vluint/vluint.h
src/dzip/deflate.o: src/dzip/internal-shared.h src/log/log.h
src/dzip/inflate.o: src/array/range.h src/array/buffer.h
src/dzip/inflate.o: src/keyargs/keyargs.h src/dzip/dzip.h src/vluint/vluint.h
src/dzip/inflate.o: src/dzip/internal-shared.h src/log/log.h
src/dzip/dzip.o: src/array/range.h src/array/buffer.h src/keyargs/keyargs.h
src/dzip/dzip.o: src/dzip/dzip.h src/log/log.h src/vluint/vluint.h
src/dzip/dzip.old.o: src/array/range.h src/array/buffer.h
src/dzip/dzip.old.o: src/keyargs/keyargs.h src/dzip/dzip.h src/log/log.h
src/dzip/dzip.old.o: src/vluint/vluint.h src/sliding-window/sliding-window.h
src/dzip/internal-shared.o: src/array/range.h src/array/buffer.h
src/dzip/internal-shared.o: src/keyargs/keyargs.h src/vluint/vluint.h
src/sliding-window/test/sliding-window.test.o: src/array/range.h
src/sliding-window/test/sliding-window.test.o: src/array/buffer.h
src/sliding-window/test/sliding-window.test.o: src/sliding-window/sliding-window.h
src/sliding-window/sliding-window.o: src/array/range.h
src/array/buffer.o: src/array/range.h
src/array/test/buffer.test.o: src/array/range.h src/array/buffer.h
src/array/test/buffer.test.o: src/test/debug.h
src/array/test/range.test.o: src/array/range.h src/test/debug.h
src/array/buffer.o: src/array/range.h src/array/buffer.h
src/tutorial/ffmpeg-video-player/player.o: src/log/log.h
src/pkg/pkg.o: src/pkg/pkg.h src/array/range.h src/array/buffer.h
src/pkg/pkg.o: src/list/list.h src/keyargs/keyargs.h
src/pkg/pkg.o: src/buffer_io/buffer_io.h src/log/log.h src/tar/tar.h
src/pkg/pkg.o: src/path/path.h src/immutable/immutable.h
src/pkg/pkg.o: src/paren-parser/paren-parser.h
src/pkg/pkg.o: src/paren-parser/paren-preprocessor.h src/table2/table.h
src/pkg/pkg.o: src/table2/table-string.h
src/pkg/pkg.util.o: src/pkg/pkg.h src/log/log.h
src/table2/table.o: src/array/range.h src/array/buffer.h src/table2/table.h
src/table2/table.o: src/table2/table-string.h
src/table2/table.o: src/array/range.h
src/table2/table-string.o: src/array/range.h src/array/buffer.h
src/table2/table-string.o: src/table2/table.h
src/table2/table-int.o: src/array/range.h src/array/buffer.h
src/table2/table-int.o: src/table2/table.h
src/table2/test/table-string-map.test.o: src/array/range.h src/array/buffer.h
src/table2/test/table-string-map.test.o: src/table2/table.h
src/table2/test/table-string-map.test.o: src/table2/table-string.h
src/table2/test/table-string-map.test.o: src/log/log.h src/keyargs/keyargs.h
src/table2/test/table-string-map.test.o: src/buffer_io/buffer_io.h
src/table2/test/table-string-benchmark.test.o: src/array/range.h
src/table2/test/table-string-benchmark.test.o: src/array/buffer.h
src/table2/test/table-string-benchmark.test.o: src/table2/table.h
src/table2/test/table-string-benchmark.test.o: src/table2/table-string.h
src/table2/test/table-string-benchmark.test.o: src/keyargs/keyargs.h
src/table2/test/table-string-benchmark.test.o: src/buffer_io/buffer_io.h
src/table2/test/table-string-benchmark.test.o: src/log/log.h
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
src/table/test/table.test.o: src/array/range.h src/list/list.h
src/table/test/table.test.o: src/table2/table.h src/test/debug.h
src/table/table.o: src/array/range.h src/list/list.h
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
src/tar/tar.o: src/array/range.h src/array/buffer.h src/keyargs/keyargs.h
src/tar/tar.o: src/buffer_io/buffer_io.h src/tar/tar.h src/log/log.h
src/tar/tar.o: src/tar/spec.h
src/tar/test/list-tar.test.o: src/array/range.h src/array/buffer.h
src/tar/test/list-tar.test.o: src/keyargs/keyargs.h src/buffer_io/buffer_io.h
src/tar/test/list-tar.test.o: src/tar/spec.h src/log/log.h src/tar/tar.h
src/tar/test/tar-dump-posix-header.test.o: src/keyargs/keyargs.h
src/tar/test/tar-dump-posix-header.test.o: src/array/range.h
src/tar/test/tar-dump-posix-header.test.o: src/array/buffer.h
src/tar/test/tar-dump-posix-header.test.o: src/buffer_io/buffer_io.h
src/tar/test/tar-dump-posix-header.test.o: src/tar/spec.h src/log/log.h
src/tar/tar.o: src/array/range.h src/array/buffer.h src/keyargs/keyargs.h
src/vluint/vluint.o: src/keyargs/keyargs.h src/array/range.h
src/vluint/vluint.o: src/array/buffer.h src/vluint/vluint.h src/log/log.h
src/vluint/test/vluint.test.o: src/keyargs/keyargs.h src/array/range.h
src/vluint/test/vluint.test.o: src/array/buffer.h src/vluint/vluint.h
src/vluint/test/vluint.test.o: src/metabase/metabase.h
src/vluint/test/vluint.test.o: src/buffer_io/buffer_io.h src/log/log.h
src/vluint/vluint.o: src/keyargs/keyargs.h src/array/range.h
src/vluint/vluint.o: src/array/buffer.h
src/list/test/list.test.o: src/list/list.h src/test/debug.h
src/json/json.o: src/array/range.h src/json/json.h src/array/buffer.h
src/json/json.o: src/log/log.h src/list/list.h src/table2/table.h
src/json/json.o: src/table2/table-string.h
src/json/json.o: src/array/range.h
src/json/test/json.test.o: src/json/json.c src/array/range.h src/json/json.h
src/json/test/json.test.o: src/array/buffer.h src/log/log.h src/list/list.h
src/json/test/json.test.o: src/table2/table.h src/table2/table-string.h
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
src/path/path.o: src/path/path.h src/array/range.h src/array/buffer.h
src/path/path.o: src/log/log.h
src/log/log.o: src/log/log.h
src/log/test/log.test.o: src/log/log.h src/test/debug.h
src/vec/vec3.o: src/vec/vec.h
src/vec/vec2.o: src/vec/vec.h
src/vec/mat4.o: src/vec/vec.h src/vec/vec3.h src/vec/vec4.h src/vec/mat4.h
src/vec/test/vec.test.o: src/vec/vec.h src/vec/vec2.h src/vec/vec3.h
src/vec/test/vec.test.o: src/test/debug.h
src/vec/mat4.o: src/vec/vec.h src/vec/vec3.h
src/vec/vec4.o: src/vec/vec.h
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
src/buffer_io/test/getline.test.o: src/keyargs/keyargs.h src/array/range.h
src/buffer_io/test/getline.test.o: src/array/buffer.h
src/buffer_io/test/getline.test.o: src/buffer_io/buffer_io.h src/test/debug.h
src/buffer_io/test/buffer_io.test.o: src/keyargs/keyargs.h src/array/range.h
src/buffer_io/test/buffer_io.test.o: src/array/buffer.h
src/buffer_io/test/buffer_io.test.o: src/buffer_io/buffer_io.h
src/buffer_io/test/buffer_io.test.o: src/test/debug.h
src/base2/test/base2.test.o: src/array/range.h src/array/buffer.h
src/base2/test/base2.test.o: src/base2/base2.h src/log/log.h
src/base2/base2.o: src/array/range.h src/array/buffer.h
src/base2/base2.o: src/array/range.h src/array/buffer.h src/base2/base2.h
src/base2/base2.o: src/log/log.h
src/http/http.o: src/keyargs/keyargs.h src/array/range.h src/array/buffer.h
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
