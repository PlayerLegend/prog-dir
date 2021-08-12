#ifndef FLAT_INCLUDES
#include <ev.h>
#include <stdbool.h>
#include <pthread.h>
#define FLAT_INCLUDES
#include "../ipfs/ipfs.h"
#endif

typedef struct ui_image ui_image;

ui_image * ui_load_image (ipfs_connection
