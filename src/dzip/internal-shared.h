#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../vluint/vluint.h"
#endif

typedef enum {
    DZIP_CMD_LITERAL,
    DZIP_CMD_LZ77,
    DZIP_CMD_REPEAT,
}
    dzip_command;

typedef struct {
    vluint_result distance;
    vluint_result length;
    char next_char;
}
    dzip_match;

inline static void read_command (unsigned char * cmd, unsigned char * arg, unsigned char input)
{
    *cmd = input & 3;
    *arg = (input & (~3)) >> 2;
}
