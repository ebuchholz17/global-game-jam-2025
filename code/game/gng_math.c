#include "gng_math.h"
#include "gng_bool.h"

// https://web.archive.org/web/20130927121234/http://devmaster.net/posts/9648/fast-and-accurate-sine-cosine
f32 fastSin2PI (f32 x) {
    while (x > 0.5f) {
        x -= 1.0f;
    }
    while (x < -0.5f) {
        x += 1.0f;
    }

    f32 absX = x > 0.0f ? x : -x;
    f32 y = 8.0f * x - 16.0f * x * absX;
    f32 absY = y > 0.0f ? y : -y;
    return 0.225f * (y * absY - y) + y;
}

f32 fastCos2PI (f32 x) {
    return fastSin2PI(x + 0.25f);
}

// TODO: accurate sin/cos
f32 sin2PI (f32 x) {
    return fastSin2PI(x);
}
f32 cos2PI (f32 x) {
    return fastCos2PI(x);
}

float sqrtF32 (f32 num) {
    if (num < 0.0f) {
        return -1.0f; // error
    }
    f32 error = 0.00001f * num;

    f32 estimate = num;
    f32 test;
    u32 iterations = 0;
    do {
        ++iterations;
        estimate = (num / estimate + estimate) / 2.0f;
        test = estimate - (num / estimate);
        if (test < 0) { test = -test; }
    } while (test > error && iterations < 50);
    return estimate;
}

mat3x3 mat3x3MatrixMul (mat3x3 a, mat3x3 b) {
    mat3x3 result = {
        .m[0] = a.m[0]*b.m[0] + a.m[1]*b.m[3] + a.m[2]*b.m[6],
        .m[1] = a.m[0]*b.m[1] + a.m[1]*b.m[4] + a.m[2]*b.m[7],
        .m[2] = a.m[0]*b.m[2] + a.m[1]*b.m[5] + a.m[2]*b.m[8],

        .m[3] = a.m[3]*b.m[0] + a.m[4]*b.m[3] + a.m[5]*b.m[6], 
        .m[4] = a.m[3]*b.m[1] + a.m[4]*b.m[4] + a.m[5]*b.m[7], 
        .m[5] = a.m[3]*b.m[2] + a.m[4]*b.m[5] + a.m[5]*b.m[8],

        .m[6] = a.m[6]*b.m[0] + a.m[7]*b.m[3] + a.m[8]*b.m[6], 
        .m[7] = a.m[6]*b.m[1] + a.m[7]*b.m[4] + a.m[8]*b.m[7], 
        .m[8] = a.m[6]*b.m[2] + a.m[7]*b.m[5] + a.m[8]*b.m[8] 
    };
    return result;
}

mat3x3 mat3x3ScalarMul (f32 a, mat3x3 b) {
    mat3x3 result = {
        .m[0] = a*b.m[0],
        .m[1] = a*b.m[1],
        .m[2] = a*b.m[2],
        .m[3] = a*b.m[3],
        .m[4] = a*b.m[4],
        .m[5] = a*b.m[5],
        .m[6] = a*b.m[6],
        .m[7] = a*b.m[7],
        .m[8] = a*b.m[8]
    };
    return result;
}

mat3x3 mat3x3Transpose (mat3x3 a) {
    mat3x3 result = {
        .m[0] = a.m[0],
        .m[1] = a.m[3],
        .m[2] = a.m[6],
        .m[3] = a.m[1],
        .m[4] = a.m[4],
        .m[5] = a.m[7],
        .m[6] = a.m[2],
        .m[7] = a.m[5],
        .m[8] = a.m[8]
    };
    return result;
}

mat3x3 mat3x3Inv (mat3x3 a) {
    // 0 1 2
    // 3 4 5
    // 6 7 8

    mat3x3 minors = {
        .m[0] = a.m[4]*a.m[8] - a.m[5]*a.m[7],
        .m[1] = -(a.m[3]*a.m[8] - a.m[5]*a.m[6]),
        .m[2] = a.m[3]*a.m[7] - a.m[4]*a.m[6],

        .m[3] = -(a.m[1]*a.m[8] - a.m[2]*a.m[7]),
        .m[4] = a.m[0]*a.m[8] - a.m[2]*a.m[6],
        .m[5] = -(a.m[0]*a.m[7] - a.m[1]*a.m[6]),

        .m[6] = a.m[1]*a.m[5] - a.m[2]*a.m[4],
        .m[7] = -(a.m[0]*a.m[5] - a.m[2]*a.m[3]),
        .m[8] = a.m[0]*a.m[4] - a.m[1]*a.m[3]
    };


    f32 determinant = a.m[0]*minors.m[0] + a.m[1]*minors.m[1] + a.m[2]*minors.m[2]; // Check for 0?
    f32 ood = 1.0f / determinant;

    mat3x3 result = mat3x3ScalarMul(ood, mat3x3Transpose(minors));
    return result;
}

mat3x3 mat3x3Identity () {
    mat3x3 result = {
        .m[0] = 1.0f,
        .m[4] = 1.0f,
        .m[8] = 1.0f
    };
    return result;
}

mat3x3 mat3x3Translate (f32 x, f32 y) {
    mat3x3 result = mat3x3Identity();
    result.m[2] = x;
    result.m[5] = y;
    return result;
}

mat3x3 mat3x3Scale (f32 s) {
    mat3x3 result = {
        .m[0] = s,
        .m[4] = s,
        .m[8] = 1.0f
    };
    return result;
}

mat3x3 mat3x3ScaleXY (f32 x, f32 y) {
    mat3x3 result = {
        .m[0] = x,
        .m[4] = y,
        .m[8] = 1.0f
    };
    return result;
}

mat3x3 mat3x3Rotate2PI (f32 angle) {
    mat3x3 result = mat3x3Identity();

    result.m[0] = cos2PI(angle);
    result.m[1] = -sin2PI(angle);
    result.m[3] = sin2PI(angle);
    result.m[4] = cos2PI(angle);

    return result;
}

f32 squaref32 (f32 a) {
    return a * a;
}

vec2 vec2Add (vec2 a, vec2 b) {
    vec2 result = {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
    return result;
}

vec2 vec2Subtract (vec2 a, vec2 b) {
    vec2 result = {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
    return result;
}

vec2 vec2Negate (vec2 a) {
    vec2 result = {
        .x = -a.x,
        .y = -a.y
    };
    return result;
}

vec2 vec2ScalarMul (f32 a, vec2 b) {
    vec2 result = {
        .x = a * b.x,
        .y = a * b.y
    };
    return result;
}

vec2 vec2HadamardMul (vec2 a, vec2 b) {
    vec2 result = {
        .x = a.x * b.x,
        .y = a.y * b.y
    };
    return result;
}

f32 vec2Dot (vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

f32 vec2LengthSqr (vec2 a) {
    return vec2Dot(a, a);
}

f32 vec2Length (vec2 a) {
    return sqrtF32(vec2LengthSqr(a));
}

vec2 vec2Normalize (vec2 a) {
    return vec2ScalarMul((1.0f / vec2Length(a)), a);
}


// shortcuts for using mat3x3's with vec2's
vec2 vec2Mat3x3Mul (mat3x3 m, vec2 v){
    vec2 result = {
        .x = m.m[0]*v.x + m.m[1]*v.y + m.m[2]*1.0f,
        .y = m.m[3]*v.x + m.m[4]*v.y + m.m[5]*1.0f
    };

    // w component would be here
    // TODO(ebuchholz): check if this is a good idea

    return result;
}

vec2 vec2TransformPoint (mat3x3 m, vec2 v, f32* z){
    vec2 result = {
        .x = m.m[0]*v.x + m.m[1]*v.y + m.m[2]*(*z),
        .y = m.m[3]*v.x + m.m[4]*v.y + m.m[5]*(*z)
    };

    // z as an out parameter so we don't have to make a vector3
    *z = m.m[6]*v.x + m.m[7]*v.y + m.m[8]*1.0f;

    return result;
}

// Multiplies the vector like a direction
// TODO(ebuchholz): make a separate point data type?
vec2 vec2TransformDir (mat3x3 m, vec2 v) {
    vec2 result = {
        .x = m.m[0]*v.x + m.m[1]*v.y + m.m[2]*0.0f,
        .y = m.m[3]*v.x + m.m[4]*v.y + m.m[5]*0.0f
    };

    // w component would be here
    // TODO(ebuchholz): check if this is a good idea

    return result;
}

vec3 vec3Add (vec3 a, vec3 b) {
    vec3 result = {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
    return result;
}

vec3 vec3Subtract (vec3 a, vec3 b) {
    vec3 result = {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
    return result;
}

vec3 vec3Negate (vec3 a) {
    vec3 result = {
        .x = -a.x,
        .y = -a.y,
        .z = -a.z
    };
    return result;
}

vec3 vec3ScalarMul (f32 a, vec3 b) {
    vec3 result = {
        .x = a * b.x,
        .y = a * b.y,
        .z = a * b.z
    };
    return result;
}

vec3 vec3HadamardMul (vec3 a, vec3 b) {
    vec3 result = {
        .x = a.x * b.x,
        .y = a.y * b.y,
        .z = a.z * b.z 
    };
    return result;
}

f32 vec3Dot (vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 crossProduct (vec3 a, vec3 b) {
    vec3 result = {
        .x = -b.y * a.z + a.y * b.z,
        .y = -b.z * a.x + a.z * b.x,
        .z = -b.x * a.y + a.x * b.y
    };
    return result;
}

vec3 vec3MatrixMul (mat3x3 m, vec3 v){
    vec3 result = {
        .x = m.m[0]*v.x + m.m[1]*v.y + m.m[2]*v.z,
        .y = m.m[3]*v.x + m.m[4]*v.y + m.m[5]*v.z,
        .z = m.m[6]*v.x + m.m[7]*v.y + m.m[8]*v.z
    };
    return result;
}

f32 vec3LengthSqr (vec3 a) {
    return vec3Dot(a, a);
}

b32 rectContainsPoint (rect r, f32 x, f32 y) {
    return x >= r.min.x && x < r.max.x && y >= r.min.y && y < r.max.y;
}

b32 rectsIntersect (rect a, rect b) {
    if (a.max.x < b.min.x) { return false; }
    if (a.min.x > b.max.x) { return false; }
    if (a.max.y < b.min.y) { return false; }
    if (a.min.y > b.max.y) { return false; }
    return true;
}

// TODO: implement sqrt
//f32 vec3Length (vec3 a) {
//    return sqrtf(vec3LengthSqr(a));
//}

//vec3 vec3Normalize (vec3 a) {
//    return a * (1.0f / vec3Length(a));
//}
