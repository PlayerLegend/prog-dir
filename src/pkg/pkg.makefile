bin/pkg-install bin/pkg-pack: src/array/buffer.o
bin/pkg-install bin/pkg-pack: src/buffer_io/buffer_io.o
bin/pkg-install bin/pkg-pack: src/log/log.o
bin/pkg-install bin/pkg-pack: src/path/path.o
bin/pkg-install: src/immutable/immutable.o
bin/pkg-install: src/paren-parser/paren-parser.o
bin/pkg-install: src/paren-parser/paren-preprocessor.o
bin/pkg-install: src/pkg/pkg-install.o
bin/pkg-install: src/pkg/pkg-install.util.o
bin/pkg-install: src/pkg/pkg-root.o
bin/pkg-install: src/table2/table.o
bin/pkg-install: src/tar/tar.o
bin/pkg-pack: src/dzip/deflate.o
bin/pkg-pack: src/pkg/pkg-pack.o
bin/pkg-pack: src/pkg/pkg-pack.util.o
bin/pkg-pack: src/tar/write.o
bin/pkg-pack: src/vluint/vluint.o

UTILS_C += bin/pkg-install bin/pkg-pack
