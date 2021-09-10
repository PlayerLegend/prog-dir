#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

typedef struct {
    int in_fd;
    int out_fd;
    int err_fd;
}
    pipe_io;

typedef struct pipe_program_state pipe_program_state;

pipe_program_state * pipe_program (pipe_io * io, const char * program, char * const argv[]);
