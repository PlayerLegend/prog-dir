PROGRAM_NAME = bin/sha256sum-accel
PROGRAM_OBJ = programs/sha256sum-accel hash_table hash_table_string sha256 stack print index_map


PKG_LDLIBS != pkg-config --libs openssl
PKG_CFLAGS != pkg-config --cflags openssl

CFLAGS += $(PKG_CFLAGS)
LDLIBS += $(PKG_LDLIBS)
