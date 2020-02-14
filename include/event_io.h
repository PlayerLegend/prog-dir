#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "range.h"
#include "stack.h"
#include "array.h"

#endif

typedef enum { IO_IN_TCP, IO_OUT_TCP, IO_IN_FILE, IO_OUT_FILE } io_domain;
typedef struct io_loop io_loop;
typedef struct
{
    io_domain domain;
    void * handle;
    bool success;
    struct {
	size_t have, payload;
    }
	size;
    char payload[];
}
    io_job;

typedef struct tcp_connection tcp_connection;

io_job * io_job_new(size_t payload, io_domain domain, void * handle);
io_job * io_job_reuse(io_job * job, io_domain domain, void * handle);
io_job * io_job_restart(io_job * job);

