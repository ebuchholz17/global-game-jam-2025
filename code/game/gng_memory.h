#ifndef GNG_MEMORY_H
#define GNG_MEMORY_H

#include "gng_types.h"

typedef struct mem_arena {
    u64 capacity;
    void *base;
    void *current;
} mem_arena;

void *allocMemory (mem_arena *memory, u32 size);

#endif
