#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdint.h>
#endif

enum {
    REG_RET = 0, // return values
    REG_ACC, // accumulator
    REG_GEN, // begin general purpose registers

    REG_MAX = 10
};

enum {
    VI_CHOOSE,
    VI_SAVE,
    VI_RECALL,
    VI_IFJUMP,
    VI_ADD,
    
}
