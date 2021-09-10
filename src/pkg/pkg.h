#ifndef FLAT_INCLUDES
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#endif

typedef struct pkg_root pkg_root;

pkg_root * pkg_root_open(const char * path);
bool pkg_install_name(pkg_root * root, const char * name);
bool pkg_install_fd(pkg_root * root, int fd);
void pkg_root_close (pkg_root * root);

typedef struct pkg_pack_state pkg_pack_state;
typedef enum {
    PKG_PACK_COMPRESSION_NONE,
    PKG_PACK_COMPRESSION_DZIP,
}
    pkg_pack_compression_type;

keyargs_declare(pkg_pack_state*, pkg_pack_new,
		int output_fd;
		int script_fd;
		pkg_pack_compression_type compression_type;);

#define pkg_pack_new(...) keyargs_call(pkg_pack_new,__VA_ARGS__)

bool pkg_pack_path(pkg_pack_state * state, const char * path);

bool pkg_pack_finish (pkg_pack_state * state);
