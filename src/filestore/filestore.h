#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../vluint/vluint.h"
#include "../metabase/metabase.h"
#include "../metahash/metahash.h"
#endif

typedef enum filestore_add_type filestore_add_type;
enum filestore_add_type {
    FILESTORE_ADD_COPY,
    FILESTORE_ADD_HARDLINK,
    FILESTORE_ADD_SYMLINK,
};

bool filestore_path (buffer_char * output_path, const range_const_unsigned_char * input_digest);

keyargs_declare(bool, filestore_copy,
		buffer_unsigned_char * result_digest;
		const range_const_metahash_id * use_hashes;
		const char * store_path;
		int src_fd;
		bool replace;
		bool existing_only;);
#define filestore_copy(...) keyargs_call(filestore_copy, __VA_ARGS__)

keyargs_declare(bool, filestore_link,
		buffer_unsigned_char * result_digest;
		int (*link)(const char * oldpath, const char * newpath);
		const range_const_metahash_id * use_hashes;
		const char * store_path;
		const char * src_path;
		bool replace;
		bool existing_only;);
#define filestore_link(...) keyargs_call(filestore_link, __VA_ARGS__)
