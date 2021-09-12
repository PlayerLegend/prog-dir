#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#endif

/**
   @file tar/read.h
   Describes the public interface for the reading portion of the tar library.
   In order to read through a tar with this library, first allocate and zero a tar_state structure. Then feed it tar sectors using tar_update_fd or tar_update_mem according to your needs. The metadata pertaining to the current file/directory/link/etc can be directly read from the tar_state after it has been updated. If the tar is being read from a stream, the contents of a file must be read before the tar state is updated again. To do this, use either tar_read_region or tar_skip_file.
*/

typedef struct tar_state tar_state;
struct tar_state {
    bool ready; ///< True if the state is ready for use
    
    tar_type type; ///< The current item type
    buffer_char path; ///< The path of the current item
    buffer_char tmp; ///< A temporary buffer used by tar_update_fd

    size_t mode; ///< The mode of the current item

    struct tar_state_link ///< tar_state information that is specific to links
    {
	buffer_char path; ///< If the current item is a hardlink or symlink, this is its target path
    }
	link; ///< Contains information specific to hardlinks and symlinks

    struct tar_state_file ///< tar_state information that is specific to files
    {
	size_t size; ///< If the current item is a file, this is its size
    }
	file; ///< Contains information specific to files
};
/**< @struct tar_state
   Gives the header information for the current file in a tar
*/

void tar_restart(tar_state * state);
/**<
   @brief Restarts the given state so that it may be used to read a different tar file.
   @param state The state to be restarted
*/

bool tar_update_fd (tar_state * state, int fd);
/**<
   @brief Reads the first or next item from the file descriptor into the given state
   @return True if successful, false otherwise
   @param state The state to be updated
   @param fd The file descriptor to read from
*/

bool tar_update_mem (tar_state * state, range_const_char * rest, const range_const_char * mem);
/**<
   @brief Adds one chunk from the beginning of mem to the given tar state.
   @return True if successful, false otherwise
   @param state The state to be updated
   @param rest A range of bytes that will be pointed to the portion of mem following the chunk read by this function. Pass NULL here if you do not need that.
   @param mem A range of input bytes
*/

void tar_cleanup (tar_state * state);
/**<
   @brief Frees all memory allocated to the given state, but not the state itself.
   @param state The state to be cleaned
*/

keyargs_declare(bool,tar_read_region,
		buffer_char * buffer;
		size_t size;
		int fd;);
#define tar_read_region(...) keyargs_call(tar_read_region, __VA_ARGS__) 
/**<
   @brief Reads a stream of tar chunks, discarding any padding at the end. Use this for reading the contents of a tar file after the tar state indicates that there is one.
   @return True if successful, false otherwise
   @param buffer The buffer in which to store the output
   @param size The size of the output to be read
   @param fd The file descriptor to read from
*/

bool tar_skip_file (tar_state * state, int fd);
/**<
   @brief Skips the file in a tar stream currently described by 'state'. If 'state' is not currently indicating a file, then the behavior of this function is undefined.
   @return True if successful, false otherwise
   @param state The state describing the file to skip
   @param fd The file descriptor to read from
*/
