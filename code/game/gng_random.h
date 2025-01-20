#ifndef GNG_RANDOM_H
#define GNG_RANDOM_H

#include "gng_types.h"

#define MAX_RAND_NUMBER (((1U << 31)-1) >> 16)
void setRNGSeed (u32 seed);
u32 randomU32 (void);
f32 randomF32 (void);

#endif
