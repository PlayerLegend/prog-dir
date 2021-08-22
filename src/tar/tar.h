#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#endif

#define TAR_BLOCK_SIZE 512

typedef enum tar_type tar_type;
enum tar_type { TAR_ERROR, TAR_DIR, TAR_FILE, TAR_SYMLINK, TAR_HARDLINK, TAR_END, TAR_LONGNAME, TAR_LONGLINK };

typedef struct tar_state tar_state;
struct tar_state {
    bool ready;
    
    tar_type type;
    buffer_char path;
    buffer_char tmp;

    size_t mode;

    struct {
	buffer_char path;
    } link;

    struct {
	size_t size;
    } file;
};

void tar_restart(tar_state * state);

bool tar_update_fd (tar_state * state, int fd);
bool tar_update_mem (tar_state * state, range_const_char * rest, const range_const_char * mem);

void tar_cleanup (tar_state * state);

keyargs_declare(bool,tar_read_region,
		buffer_char * buffer;
		size_t size;
		int fd;);

#define tar_read_region(...) keyargs_call(tar_read_region, __VA_ARGS__) 

bool tar_skip_file (tar_state * state, int fd);
