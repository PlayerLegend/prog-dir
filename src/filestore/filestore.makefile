bin/filestore-path bin/filestore-add: LDLIBS += -lgcrypt
bin/filestore-path bin/filestore-add:src/filestore/filestore.o src/buffer_io/buffer_io.o src/metabase/metabase.o src/metahash/metahash.o src/vluint/vluint.o src/log/log.o src/base16/base16.o src/base2/base2.o src/array/buffer.o

bin/filestore-path: src/filestore/filestore-path.util.o
bin/filestore-add: src/filestore/filestore-add.util.o

UTILS_C += bin/filestore-path bin/filestore-add
