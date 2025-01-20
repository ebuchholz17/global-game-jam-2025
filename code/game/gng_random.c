#include "gng_random.h"

static u32 rngSeed;

void setRNGSeed (u32 seed) {
    rngSeed = seed;
}

u32 randomU32 (void) {
    rngSeed = (rngSeed * 1103515245U + 12345U) % (1U << 31);
    return (rngSeed & ((1U << 31)-1)) >> 16;
}

f32 randomF32 (void) {
    u32 randomNum = randomU32();
    return (f32)randomNum / (f32)(MAX_RAND_NUMBER);
}
