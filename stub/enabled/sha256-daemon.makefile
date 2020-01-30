PROGRAM_NAME = bin/sha256-daemon
PROGRAM_OBJ = programs/sha256-daemon print print_array hash_table hash_table_string index_map stack network tcp_event sha256

PKG_LDLIBS != pkg-config --libs openssl
PKG_CFLAGS != pkg-config --cflags openssl

CFLAGS += $(PKG_CFLAGS)
LDLIBS += $(PKG_LDLIBS)

LDLIBS += -lpthread -lev
