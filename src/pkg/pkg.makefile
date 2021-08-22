bin/pkg: src/pkg/pkg.util.o
bin/pkg: src/pkg/pkg.o
bin/pkg: src/buffer_io/buffer_io.o
bin/pkg: src/table2/table.o
bin/pkg: src/log/log.o
bin/pkg: src/path/path.o
bin/pkg: src/tar/tar.o
bin/pkg: src/paren-parser/paren-parser.o
bin/pkg: src/paren-parser/paren-preprocessor.o
bin/pkg: src/immutable/immutable.o
bin/pkg: src/array/buffer.o

UTILS_C += bin/pkg
