#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../chain-io/common.h"
#include "../chain-io/read.h"
#endif

typedef struct http_client http_client;

http_client * http_client_connect (const char * host, const char * port);
void http_client_disconnect (http_client * client);
chain_read * http_client_get (http_client * client, const char * path);
