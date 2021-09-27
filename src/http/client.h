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

/** @file http/client.h
    
    This file implements an http client capable of issuing GET requests. At the moment, only identity and chunked transfer encodings are supported.

    The intended workflow for this library is to first open a connection to a server using http_client_connect, then use the connection returned by that function to issue one or more get requests in series. Finally, after the last get request finishes or errors, use http_client_disconnect to destroy the connection and free all associated memory.

 */

typedef struct http_client http_client; ///< A connection handle used to access an http server

http_client * http_client_connect (const char * host, const char * port);
/**< @brief Given a textual host and port, this function connects to an http server.
   @returns A pointer to a connection handle if successful, NULL otherwise.
*/

void http_client_disconnect (http_client * client); ///< Destroys the given connection handle and frees any associated memory. Note - the handle should not have any active get requests associated with it, let those finish or result in error before using this function to destroy their connection.
chain_read * http_client_get (http_client * client, const char * path); ///< Given a connection and requested file path, this function returns a chain_read handle for reading the requested file path from the server.
