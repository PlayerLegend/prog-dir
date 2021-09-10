#ifndef FLAT_INCLUDES
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "pipe-program.h"
#include "../log/log.h"
#endif

struct pipe_program_state {
    pid_t pid;
};

static void setup_pipe (int * child_side, int * parent_side, bool read_from_parent)
{
    int pipe_tmp[2];
    
    if (*parent_side < 0)
    {
	pipe (pipe_tmp);
	*parent_side = pipe_tmp[!read_from_parent];
	*child_side = pipe_tmp[read_from_parent];
    }
    else
    {
	*child_side = *parent_side;
	*parent_side = -1;
    }
}

inline static void close_maybe(int fd)
{
    if (fd >= 0)
    {
	close (fd);
    }
}

inline static int dup2_maybe(int from, int to)
{
    if (from != to)
    {
	return dup2(from,to);
    }
    else
    {
	return 0;
    }
}

pipe_program_state * pipe_program (pipe_io * io, const char * program, char * const argv[])
{
    pipe_io child_side = {-1, -1, -1};

    setup_pipe (&child_side.err_fd, &io->err_fd, true);
    setup_pipe (&child_side.out_fd, &io->out_fd, true);
    setup_pipe (&child_side.in_fd, &io->in_fd, false);
    
    pid_t pid = fork();

    if (!pid) // child
    {
	close_maybe(io->in_fd);
	close_maybe(io->out_fd);
	close_maybe(io->err_fd);

	if (0 != dup2_maybe(child_side.err_fd, STDERR_FILENO) ||
	    0 != dup2_maybe(child_side.out_fd, STDOUT_FILENO) ||
	    0 != dup2_maybe(child_side.in_fd, STDIN_FILENO))
	{
	    log_fatal ("Could not duplicate a file descriptor");
	}
	
	execvp (program, argv);
    }
    else
    {
	close_maybe(child_side.in_fd);
	close_maybe(child_side.out_fd);
	close_maybe(child_side.err_fd);
    }

fail:
    exit(1);
}
