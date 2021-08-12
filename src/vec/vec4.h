#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include "vec.h"
#endif

#define vec4(type) struct { type x; type y; type z; type w; }
#define vec4_add(a,b) { (a).x += (b).x; (a).y += (b).y; (a).z += (b).z; (a).w += (b).w; }
#define vec4_subtract(a,b) { (a).x -= (b).x; (a).y -= (b).y; (a).z -= (b).z; (a).w -= (b).w; }
#define vec4_scale(a,s) { (a).x *= s; (a).y *= s; (a).z *= s; (a).w *= s; }

typedef vec4(ivec) ivec4;
typedef vec4(fvec) fvec4;

//#define TEST_VEC4_H
#ifdef TEST_VEC4_H
#ifdef NDEBUG
#error "Tests must be built in debug mode."
#endif
#include <assert.h>
int main (int argc, char * argv[])
{
    
}
#endif
