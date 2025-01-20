#include "../game/gng_types.h"
#include "web_platform.h"

u32 memCapacity;

extern u8 __heap_base;
u8 *memStart = &__heap_base;
u8 *memCurrent = &__heap_base;

// grow temp memory from other side of the heap
u8 *tempMemStart = &__heap_base;
u8 *tempMemCurrent = &__heap_base;

WASM_EXPORT void *webAllocMemory (u32 size) {
    // align to 4 bytes
    u32 memSize = memCurrent - memStart;
    if (memSize + size > memCapacity) {
        onError("Memory capacity exceeded during webAllocMemory");
    }

    // TODO: double check this, seems wrong
    u32 alignedSpot = (memSize / 4) * 4;
    if (alignedSpot != memSize) {
        memCurrent = (memStart + alignedSpot) + 4;
    }

    u8 *result = memCurrent;
    memCurrent += size;
    return (void *)result;
}

// per-frame temporary memory
WASM_EXPORT void *webAllocTempMemory (u32 size) {
    // align to 4 bytes
    u32 tempMemSize = tempMemCurrent - tempMemStart;

    char numBuffer[20];
    // don't let temporary memory flood into 'permanent' memory
    if (tempMemSize + size > memStart) {
        onError("Temp memory capacity exceeded during webAllocTempMemory");
    }

    // TODO: double check this, seems wrong
    u32 alignedSpot = (tempMemSize / 4) * 4;
    if (alignedSpot != tempMemSize) {
        tempMemCurrent = (tempMemStart + alignedSpot) + 4;
    }

    u8 *result = tempMemCurrent;
    tempMemCurrent += size;
    return (void *)result;
}

void webFreeTempMemory () {
    tempMemCurrent = tempMemStart;
}

