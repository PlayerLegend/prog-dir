#include <string.h>
#include <math.h>
#define FLAT_INCLUDES
#include "vec.h"
#include "vec3.h"
#include "vec4.h"
#include "mat4.h"

void quaternion_rotation_matrix (mat4 mat, fvec4 * q)
{
    fvec
	xx = q->x * q->x,
	xy = q->x * q->y,
	xz = q->x * q->z,
	xw = q->x * q->w,
	yy = q->y * q->y,
	yz = q->y * q->z,
	yw = q->y * q->w,
	zz = q->z * q->z,
	zw = q->z * q->w;

    // https://stackoverflow.com/questions/4360918/correct-opengl-matrix-format
    // OpenGL specifies matrices as a one-dimensional array listed in column-major order, ie with elements ordered like this:
    /*
      m0 m4 m8  m12
      m1 m5 m9  m13
      m2 m6 m10 m14
      m3 m7 m11 m15
     */
    
    mat[0]  = 1 - 2 * (yy + zz);
    mat[1]  =     2 * (xy - zw);
    mat[2]  =     2 * (xz + yw);
    mat[3]  = 0;

    mat[4]  =     2 * (xy + zw);
    mat[5]  = 1 - 2 * (xx + zz);
    mat[6]  =     2 * (yz - xw);
    mat[7]  = 0;

    mat[8]  =     2 * (xz - yw);
    mat[9]  =     2 * (yz + xw);
    mat[10] = 1 - 2 * (xx + yy);
    mat[11] = 0;

    mat[12] = 0;
    mat[13] = 0;
    mat[14] = 0;
    mat[15] = 1;
}

void apply_translation (mat4 input, fvec3 * tl)
{
#define tl_matrix_unwrap(row) input[12 + row] = input[0 + row]*tl->x + input[4 + row]*tl->y + input[8 + row]*tl->z + input[12 + row];
    tl_matrix_unwrap(0);
    tl_matrix_unwrap(1);
    tl_matrix_unwrap(2);
    tl_matrix_unwrap(3);
}

void mat4_multiply (mat4 out, mat4 a, mat4 b)
{
    int row, col;

    for (row = 0; row < 4; row += 1)
    {
	for (col = 0; col < 16; col += 4)
	{
	    out[col + row]
		= a[row] * b[col]
		+ a[row + 4] * b[col + 1]
		+ a[row + 8] * b[col + 2]
		+ a[row + 12] * b[col + 3];
	}
    }
}

typedef struct perspective_args perspective_args;
struct perspective_args {
    fvec fovy;
    fvec aspect;
    fvec near;
    fvec far;
};

typedef struct object_args object_args;
struct object_args {
    fvec3 origin;
    fvec3 rotation;
};

void mat4_perspective (mat4 out, perspective_args * args)
{
    /* https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml */

    float f = args->fovy / 2;
    f = cos (f) / sin (f);

    out[0] = f / args->aspect;
    out[1] = 0;
    out[2] = 0;
    out[3] = 0;

    out[4] = 0;
    out[5] = f;
    out[6] = 0;
    out[7] = 0;

    out[8] = 0;
    out[9] = 0;
    out[10] = (args->far + args->near) / (args->near - args->far);
    out[11] = -1;

    out[12] = 0;
    out[13] = 0;
    out[14] = (2 * args->far * args->near) / (args->near - args->far);
    out[15] = 0;
}

void mat4_position (mat4 out, object_args * args)
{
    fvec4 quaternion = { args->rotation.x, args->rotation.y, args->rotation.z, 0 }; 
    quaternion_rotation_matrix (out, &quaternion);
    apply_translation (out, &args->origin);
}

void mat4_init_identity (mat4 mat)
{
    mat[0] = 1;
    mat[1] = 0;
    mat[2] = 0;
    mat[3] = 0;
    
    mat[4] = 0;
    mat[5] = 1;
    mat[6] = 0;
    mat[7] = 0;
    
    mat[8] = 0;
    mat[9] = 0;
    mat[10] = 1;
    mat[11] = 0;
    
    mat[12] = 0;
    mat[13] = 0;
    mat[14] = 0;
    mat[15] = 1;
    
}
