#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include "vec.h"
#endif

#define vec2(type) struct { type x; type y; }
#define vec2_add(a,b) { (a).x += (b).x; (a).y += (b).y; }
#define vec2_subtract(a,b) { (a).x -= (b).x; (a).y -= (b).y; }
#define vec2_scale(v,s) { (v).x *= s; (v).y *= s; }

typedef vec2(ivec) ivec2;
typedef vec2(fvec) fvec2;
