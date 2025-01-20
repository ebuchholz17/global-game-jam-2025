#include "gng_memory.h"

void *allocMemory (mem_arena *memory, u32 size) {
    ASSERT(((u8 *)memory->current - (u8 *)memory->base) + size <= memory->capacity);

    void *mem = memory->current;
    memory->current = (u8 *)memory->current + size;
    return mem;
}

