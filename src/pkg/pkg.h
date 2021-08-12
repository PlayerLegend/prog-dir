#ifndef FLAT_INCLUDES
#include <stdbool.h>
#define FLAT_INCLUDES
#endif

typedef struct pkg_root pkg_root;

pkg_root * pkg_root_open(const char * path);
bool pkg_install_name(const char * name);
bool pkg_install_fd(int fd);
void pkg_root_close (pkg_root * root);
