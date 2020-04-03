#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <ev.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include "nc-db/server-state.h"
#endif

typedef struct {
    pthread_mutex_t mutex;
    int fd;
}
    connection_reference;

void io_connection(struct ev_loop * loop, int fd, server_state * state);
