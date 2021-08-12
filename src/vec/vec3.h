#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include "vec.h"
#endif

#define vec3(type) struct { type x; type y; type z; }
#define vec3_add(a,b) { (a).x += (b).x; (a).y += (b).y; (a).z += (b).z; }
#define vec3_subtract(a,b) { (a).x -= (b).x; (a).y -= (b).y; (a).z -= (b).z; }
#define vec3_scale(a,s) { (a).x *= s; (a).y *= s; (a).z *= s; }

typedef vec3(ivec) ivec3;
typedef vec3(fvec) fvec3;
