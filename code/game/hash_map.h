#ifndef HASH_MAP_H 
#define HASH_MAP_H

#include "gng_types.h"

#define HASH_MAP_TYPE_STRUCT(x) HASH_MAP_TYPE_STRUCT_(x)
#define HASH_MAP_TYPE_STRUCT_(x) x##_hash_map
#define HASH_MAP_ENTRY_STRUCT(x) HASH_MAP_ENTRY_STRUCT_(x)
#define HASH_MAP_ENTRY_STRUCT_(x) x##_hash_map_value
#define HASH_MAP_INIT(x) HASH_MAP_INIT_(x)
#define HASH_MAP_INIT_(x) x##_hash_mapInit
#define HASH_MAP_STORE(x) HASH_MAP_STORE_(x)
#define HASH_MAP_STORE_(x) x##_hash_mapStore
#define HASH_MAP_GET_PTR(x) HASH_MAP_GET_PTR_(x)
#define HASH_MAP_GET_PTR_(x) x##_hash_mapGetPtr
#define HASH_MAP_GET_VAL(x) HASH_MAP_GET_VAL_(x)
#define HASH_MAP_GET_VAL_(x) x##_hash_mapGetVal

u32 hashMapHashString (char *string) {
    u32 hash = 5381;
    i32 c;
    ASSERT(string);
    do {
        c = *string++;
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    } while (c);

    return hash;
}

#include "gng_string.h"
#include "gng_util.h"

#endif

typedef struct HASH_MAP_ENTRY_STRUCT(HASH_MAP_TYPE) {
    // NOTE(ebuchholz): hash map not responsible for managing string
    char *key;
    HASH_MAP_TYPE value;
} HASH_MAP_ENTRY_STRUCT(HASH_MAP_TYPE);

typedef struct HASH_MAP_TYPE_STRUCT(HASH_MAP_TYPE) {
    HASH_MAP_ENTRY_STRUCT(HASH_MAP_TYPE) *entries;
    u32 capacity;
} HASH_MAP_TYPE_STRUCT(HASH_MAP_TYPE);

char * tempStringFromI32(i32);
void HASH_MAP_STORE(HASH_MAP_TYPE) (HASH_MAP_TYPE_STRUCT(HASH_MAP_TYPE) *hashMap, HASH_MAP_TYPE value, char *key) {
    u32 hash = hashMapHashString(key);
    u32 mapIndex = hash % hashMap->capacity;
    u32 originalMapIndex = mapIndex; // check that we actually find it after looping all the way around
    HASH_MAP_ENTRY_STRUCT(HASH_MAP_TYPE) *existingEntry = &hashMap->entries[mapIndex];
    while (existingEntry->key != 0) {
        mapIndex = (mapIndex + 1) % hashMap->capacity;
        existingEntry = &hashMap->entries[mapIndex];
        ASSERT(mapIndex != originalMapIndex);
    }
    hashMap->entries[mapIndex] = 
    (HASH_MAP_ENTRY_STRUCT(HASH_MAP_TYPE))  {
        .key = key,
        .value = value
    };
;
}

HASH_MAP_TYPE *HASH_MAP_GET_PTR(HASH_MAP_TYPE) (HASH_MAP_TYPE_STRUCT(HASH_MAP_TYPE) *hashMap, char *key) {
    HASH_MAP_TYPE *result;

    u32 hash = hashMapHashString(key);
    u32 mapIndex = hash % hashMap->capacity;
    u32 originalMapIndex = mapIndex; // check that we actually find it after looping all the way around

    while (true) {
        HASH_MAP_ENTRY_STRUCT(HASH_MAP_TYPE) *entry = &hashMap->entries[mapIndex];
        if (stringEquals(key, entry->key)) {
            result = &entry->value;
            break;
        }
        else {
            mapIndex = (mapIndex + 1) % hashMap->capacity;
            if (mapIndex == originalMapIndex) {
                return 0;
            }
        }
    }

    return result;
}

HASH_MAP_TYPE HASH_MAP_GET_VAL(HASH_MAP_TYPE) (HASH_MAP_TYPE_STRUCT(HASH_MAP_TYPE) *hashMap, char *key) {
    HASH_MAP_TYPE *result = HASH_MAP_GET_PTR(HASH_MAP_TYPE)(hashMap, key);
    ASSERT(result != 0);
    return *result;
}

HASH_MAP_TYPE_STRUCT(HASH_MAP_TYPE) HASH_MAP_INIT(HASH_MAP_TYPE) (mem_arena *memory, u32 capacity) {
    HASH_MAP_TYPE_STRUCT(HASH_MAP_TYPE) result = {
        .capacity = capacity,
        .entries = (HASH_MAP_ENTRY_STRUCT(HASH_MAP_TYPE) *)allocMemory(memory, capacity * sizeof(HASH_MAP_ENTRY_STRUCT(HASH_MAP_TYPE)))
    };
    for (u32 i = 0; i < capacity; ++i) {
        zeroMemory((u8 *)&result.entries[i], sizeof(HASH_MAP_ENTRY_STRUCT(HASH_MAP_TYPE)));
    }
    return result;
}

#undef HASH_MAP_TYPE
