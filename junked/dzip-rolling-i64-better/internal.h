#ifndef FLAT_INCLUDES
#include <stdint.h>
#define FLAT_INCLUDES
#endif

typedef enum {
    DZIP_CMD_LITERAL,
    DZIP_CMD_MATCH,
}
    dzip_command;

//typedef uint8_t dzip_window_point;
typedef uint16_t dzip_window_point;
//typedef uint32_t dzip_window_point;

typedef struct {
    //char begin[256];
    char begin[65536];
    //char begin[65535 * 256];
    dzip_window_point point;
}
    dzip_window;

#define DZIP_ARG1_BITS 13
#define DZIP_ARG1_MAX (1 << DZIP_ARG1_BITS)
#define DZIP_ARG1_EXTEND_BIT (1 << 2)
#define unwrap_cmd(byte) (byte & 3)
#define unwrap_arg1_compact(byte) ( (byte >> 3) & 31 )
#define arg1_is_extended(byte) (byte & 4)
#define unwrap_arg1_extended(extended) ( (extended >> 3) & (DZIP_ARG1_MAX - 1) )

