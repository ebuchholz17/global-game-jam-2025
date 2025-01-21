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

#endif
