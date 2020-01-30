PROGRAM_NAME = bin/sha256sum
PROGRAM_OBJ = programs/sha256sum sha256

PKG_LDLIBS != pkg-config --libs openssl
PKG_CFLAGS != pkg-config --cflags openssl

CFLAGS += $(PKG_CFLAGS)
LDLIBS += $(PKG_LDLIBS)
