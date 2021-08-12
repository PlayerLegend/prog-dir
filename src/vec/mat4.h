#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include "vec.h"
#include "vec3.h"
#endif

typedef fvec mat4[16];
typedef struct mat4_swap mat4_swap;
struct mat4_swap {
    int result_index;
    mat4 contents[3];
};

void mat4_swap_init_identity (mat4_swap * swap);
void add_translation_matrix (mat4 input, fvec3 tl);
