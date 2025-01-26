#ifndef GNG_ASSETS_H
#define GNG_ASSETS_H

#include "gng_types.h"
#include "gng_math.h"
#include "gng_util.h"
#include "gng_memory.h"
#include "gng_string.h"

typedef enum {
    ASSET_TO_LOAD_TYPE_BITMAP,
    ASSET_TO_LOAD_TYPE_ATLAS_TEXTURE,
    ASSET_TO_LOAD_TYPE_ATLAS_DATA,
    ASSET_TO_LOAD_TYPE_WAV,
    ASSET_TO_LOAD_TYPE_DATA
} asset_to_load_type;

typedef struct asset_to_load {
    char *name;
    char *path;
    asset_to_load_type type;
    b32 loaded;

    // optional metadata for processing
    u32 id;
    char *key;
} asset_to_load;

#define LIST_TYPE asset_to_load
#include "list.h"

typedef struct texture_asset {
    char *key;
    u32 id;
    u32 width;
    u32 height;
    u8 *pixels;
} texture_asset;
#define HASH_MAP_TYPE texture_asset
#include "hash_map.h"
#define MAX_NUM_TEXTURE_ASSETS 50
// TODO: hash map to get texture by name

// assuming a particular bmp format (the one that gets exported from gimp: 32 bit, rgba, compression mode 3)
#pragma pack(push, 1)
typedef struct bitmap_header {
    u16 type;
    u32 fileSize;
    u16 reserved1;
    u16 reserved2;
    u32 bitmapOffset;
    u32 structSize;
    i32 width;
    i32 height;
    u16 planes;
    u16 bitsPerPixel;

    u32 compression;
    u32 imageSize;
    i32 xResolution;
    i32 yResolution;
    u32 numColors;
    u32 numImportantColors;

    u32 redMask;
    u32 greenMask;
    u32 blueMask;
} bitmap_header;
#pragma pack(pop)

typedef struct atlas_frame {
    // assumes unrotated
    vec2 frameCorners[4]; // 0.0-1.0 texture coords, not pixel width/height
    char *key;
    u32 frameWidth;
    u32 frameHeight;
} atlas_frame;

#define ATLAS_MAP_MAX_NUM_FRAMES 50
typedef struct atlas_map {
    atlas_frame entries[ATLAS_MAP_MAX_NUM_FRAMES];
} atlas_map;

typedef enum {
    ATLAS_ID_GAME
} atlas_id;

typedef struct atlas_asset {
    atlas_map map;
    atlas_id id;
    u32 textureID;
    u32 width;
    u32 height;
} atlas_asset;
#define MAX_NUM_ATLAS_ASSETS 3

#define HASH_MAP_TYPE atlas_asset
#include "hash_map.h"

typedef struct sound_asset {
    char *key;
    // info about file size, duration, etc?
    short *samples;
    int numSamples;
} sound_asset;

#define HASH_MAP_TYPE sound_asset
#include "hash_map.h"
#define MAX_NUM_SOUND_ASSETS 50

#pragma pack(push, 1)
typedef struct wav_header {
    u8 riff[4];
    i32 fileSize;
    u8 fileType[4];
    u8 formatChunkMarker[4];
    i32 formatLength;
    u16 formatType;
    u16 numChannels;
    i32 sampleRate;
    i32 byteRate;
    u16 blockAlign;
    u16 bitsPerSample;
    u8 dataMarker[4];
    i32 dataSize;
} wav_header;
#pragma pack(pop)

typedef struct data_asset {
    void *data;
    u32 size;
} data_asset;
#define MAX_NUM_DATA_ASSETS 200

#define HASH_MAP_TYPE data_asset
#include "hash_map.h"

#define MAX_NUM_ASSETS_TO_LOAD 50
typedef struct asset_man {
    asset_to_load_list assetToLoadList;
    b32 allFilesLoaded;

    mem_arena memory;

    texture_asset_hash_map textures;
    u32 currentTextureID; // TODO: use preprocessed string ids for textures?

    atlas_asset_hash_map atlases;
    sound_asset_hash_map sounds;
    data_asset_hash_map dataAssets;
} asset_man;

data_asset *getDataAsset (char *key);
extern asset_man *assetMan;

#endif

