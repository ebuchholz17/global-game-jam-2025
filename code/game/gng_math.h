#ifndef GNG_MATH_H
#define GNG_MATH_H

#include "gng_types.h"

// TODO Better constants
#define PI 3.14159265359f
#define EPSILON 0.0000001f

typedef struct mat3x3 {
    f32 m[9];
} mat3x3;

typedef struct vec2 {
    f32 x;
    f32 y;
} vec2;

typedef struct vec3 {
    f32 x;
    f32 y;
    f32 z;
} vec3;

typedef struct rect {
    vec2 min;
    vec2 max;
} rect;

vec2 vec2Add (vec2 a, vec2 b);
vec2 vec2Subtract (vec2 a, vec2 b);

f32 fastSin2PI (f32 x);

f32 fastCos2PI (f32 x);

f32 sin2PI (f32 x);
f32 cos2PI (f32 x);

mat3x3 mat3x3MatrixMul (mat3x3 a, mat3x3 b);

mat3x3 mat3x3ScalarMul (f32 a, mat3x3 b);

mat3x3 mat3x3Transpose (mat3x3 a);

mat3x3 mat3x3Inv (mat3x3 a);

mat3x3 mat3x3Identity ();

mat3x3 mat3x3Translate (f32 x, f32 y);

mat3x3 mat3x3Scale (f32 s);

mat3x3 mat3x3ScaleXY (f32 x, f32 y);

mat3x3 mat3x3Rotate2PI (f32 angle);

f32 squaref32 (f32 a);

vec2 vec2Add (vec2 a, vec2 b);

vec2 vec2Subtract (vec2 a, vec2 b);

vec2 vec2Negate (vec2 a);

vec2 vec2ScalarMul (f32 a, vec2 b);

vec2 vec2HadamardMul (vec2 a, vec2 b);

f32 vec2Dot (vec2 a, vec2 b);

vec2 vec2Mat3x3Mul (mat3x3 m, vec2 v);

vec2 vec2TransformPoint (mat3x3 m, vec2 v, f32* z);

vec2 vec2TransformDir (mat3x3 m, vec2 v);

vec3 vec3Add (vec3 a, vec3 b);

vec3 vec3Subtract (vec3 a, vec3 b);

vec3 vec3Negate (vec3 a);

vec3 vec3ScalarMul (f32 a, vec3 b);

vec3 vec3HadamardMul (vec3 a, vec3 b);

f32 vec3Dot (vec3 a, vec3 b);

vec3 crossProduct (vec3 a, vec3 b);

vec3 vec3MatrixMul (mat3x3 m, vec3 v);

f32 vec3LengthSqr (vec3 a);

b32 rectContainsPoint (rect r, f32 x, f32 y);

b32 rectsIntersect (rect a, rect b);
//f32 vec3Length (vec3 a);
//vec3 vec3Normalize (vec3 a);
vec2 vec2Normalize (vec2 a);
f32 vec2LengthSqr (vec2 a);
f32 vec2Length (vec2 a); 


#endif
