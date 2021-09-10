#ifndef FLAT_INCLUDES
#include <stdint.h>
#define FLAT_INCLUDES
#endif

typedef enum {
    DZIP_CMD_LITERAL,
    DZIP_CMD_MATCH,
}
    dzip_command;


//typedef uint32_t dzip_window_point;
//typedef char dzip_window[1000000];
typedef uint16_t dzip_window_point;
typedef char dzip_window[65536];
//typedef char dzip_window[];

#define DZIP_COMMAND_BITS 2
#define DZIP_EXTEND_BITS 1
#define DZIP_META_BITS (DZIP_COMMAND_BITS + DZIP_EXTEND_BITS)
#define DZIP_ARG1_COMPACT_BITS (8 - DZIP_META_BITS)
#define DZIP_ARG1_EXTEND_BITS (8 + DZIP_ARG1_COMPACT_BITS)
#define DZIP_ARG1_EXTEND_MAX (1 << DZIP_ARG1_EXTEND_BITS)
#define DZIP_ARG1_EXTEND_MASK (DZIP_ARG1_EXTEND_MAX - 1)
#define DZIP_ARG1_COMPACT_MAX (1 << DZIP_ARG1_COMPACT_BITS)
#define DZIP_ARG1_COMPACT_MASK ( (DZIP_ARG1_COMPACT_MAX - 1) )
#define DZIP_COMMAND_MAX (1 << DZIP_COMMAND_BITS)
#define DZIP_COMMAND_MASK (DZIP_COMMAND_MAX - 1)
#define DZIP_ARG1_EXTEND_BIT (1 << 2)

#define unwrap_cmd(byte) (byte & DZIP_COMMAND_MASK)
#define unwrap_arg1_compact(byte) ( ((byte) & DZIP_ARG1_COMPACT_MASK) >> DZIP_ARG1_META_BITS )
#define arg1_is_extended(byte) (byte & DZIP_ARG1_EXTEND_BIT)

#define sizeof_member(type, member) sizeof( ((type*)0)->member )

#define count_array(array) (sizeof(array) / sizeof((array)[0]))

#define reference_window_byte(window, index)		\
    ( (window)[ (count_array(window) + (index)) % count_array(window) ] )
