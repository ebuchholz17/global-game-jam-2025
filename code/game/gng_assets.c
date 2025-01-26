#include "gng_bool.h"
#include "gng_assets.h"

asset_man *assetMan;

atlas_asset *getAtlas (asset_man *assets, char *name) {
    return atlas_asset_hash_mapGetPtr(&assets->atlases, name);
}

texture_asset *getTexture (asset_man *assetMan, char *key) {
    return texture_asset_hash_mapGetPtr(&assetMan->textures, key);
}

sound_asset *getSound (asset_man *assetMan, char *key) {
    return sound_asset_hash_mapGetPtr(&assetMan->sounds, key);
}

data_asset *getDataAsset (char *key) {
    return data_asset_hash_mapGetPtr(&assetMan->dataAssets, key);
}

u32 leastSignificantBit (u32 value, b32 *found) {
    *found = false;
    for (u32 digit = 0; digit < 32; ++digit) {
        if (value & (1 << digit)) {
            *found = true;
            return digit;
        }
    }
    return 0;
}

// TODO: maybe discard pixel data once uploaded to gpu
void parseBitmap (asset_man *assetMan, char *key, void *assetData) {

    bitmap_header *header = (bitmap_header *)assetData;    
    u32 *pixels = (u32 *)((u8 *)assetData + header->bitmapOffset);

    texture_asset textureAsset = {
        .id = assetMan->currentTextureID++,
        .width = header->width,
        .height = header->height,
        .pixels = (u8 *)allocMemory(&assetMan->memory, sizeof(u8) * 4 * header->width * header->height)
    };
    texture_asset_hash_mapStore(&assetMan->textures, textureAsset, key);

    ASSERT(header->bitsPerPixel == 32 && header->compression == 3);

    u32 alphaMask = ~(header->redMask | header->greenMask | header->blueMask);

    b32 foundLeastSignificantDigit = false;
    u32 redMaskDigit = leastSignificantBit(header->redMask, &foundLeastSignificantDigit);
    ASSERT(foundLeastSignificantDigit);
    u32 greenMaskDigit = leastSignificantBit(header->greenMask, &foundLeastSignificantDigit);
    ASSERT(foundLeastSignificantDigit);
    u32 blueMaskDigit = leastSignificantBit(header->blueMask, &foundLeastSignificantDigit);
    ASSERT(foundLeastSignificantDigit);
    u32 alphaMaskDigit = leastSignificantBit(alphaMask, &foundLeastSignificantDigit);
    ASSERT(foundLeastSignificantDigit);

    u32 numPixelValues = header->width * header->height;
    u32 valuesPerRow = 4 * header->width;
    u32 numRows = header->height;
    for (u32 row = numRows - 1; row != (u32)(0-1); --row) {
        for (u32 i = 0; i < header->width; i++) {
            u32 color = pixels[row * header->width + i];

            u8 red = (u8)((color & header->redMask) >> redMaskDigit);
            u8 green = (u8)((color & header->greenMask) >> greenMaskDigit);
            u8 blue = (u8)((color & header->blueMask) >> blueMaskDigit);
            u8 alpha = (u8)((color & alphaMask) >> alphaMaskDigit);

            textureAsset.pixels[(numRows - 1 - row) * header->width * 4 + (i*4)] = red;
            textureAsset.pixels[(numRows - 1 - row) * header->width * 4 + (i*4)+1] = green;
            textureAsset.pixels[(numRows - 1 - row) * header->width * 4 + (i*4)+2] = blue;
            textureAsset.pixels[(numRows - 1 - row) * header->width * 4 + (i*4)+3] = alpha;
        }
    }
}

void parseWav (asset_man *assetMan, char *key, void *fileData, plat_api platAPI) {
    sound_asset soundAsset = {
        .key = key
    };

    wav_header *header = (wav_header *)fileData;    
    // NOTE(ebuchholz): supporting only a very specific format: 1 channel, 16-bit samples, 
    ASSERT(header->numChannels == 1);
    ASSERT(header->formatType == 1);
    ASSERT(header->bitsPerSample == 16);
    ASSERT(header->blockAlign == 2);

    i32 numFileSamples = header->dataSize / header->blockAlign;
    if (platAPI.audioSampleRate != (i32)header->sampleRate) {
        // resample
        // TODO: do this better
        f64 sampleRatio = (f64)header->sampleRate / (f64)platAPI.audioSampleRate;

        i32 numSamples;
        if (sampleRatio < 1.0) {
            numSamples = (i32)(numFileSamples * sampleRatio);
        }
        else {
            numSamples = (i32)(numFileSamples / sampleRatio);
        }
        i32 numSamplesToAllocate = ((numFileSamples / 4) + 1) * 4;
        soundAsset.samples = (i16 *)(allocMemory(&assetMan->memory, sizeof(i16) * numSamplesToAllocate));
        soundAsset.numSamples = numSamples;

        i16 *sampleData = (i16 *)((u8 *)fileData + sizeof(wav_header));
        for (i32 i = 0; i < numSamples; ++i) {
            f64 sampleIndex = (f64)i * sampleRatio;
            f64 firstSample = (f64)((i64)sampleIndex);
            f64 secondSample = (f64)((i64)(sampleIndex+1.0));
            if ((i32)firstSample >= numSamples || (i32)secondSample >= numSamples) {
                soundAsset.samples[i] = sampleData[numFileSamples - 1];
            }
            else {
                f64 t = sampleIndex - firstSample;
                f64 firstSampleContribution = (1.0 - t) * (f64)sampleData[(i32)firstSample];
                f64 secondSampleContribution = t * (f64)sampleData[(i32)secondSample];
                soundAsset.samples[i] = (i16)(firstSampleContribution + secondSampleContribution);
            }
        }
    }
    else {
        // align data??? think this will allocate a little extra so the data is aligned
        i32 numSamplesToAllocate = ((numFileSamples / 4) + 1) * 4;
        soundAsset.samples = (i16 *)(allocMemory(&assetMan->memory, sizeof(i16) * numSamplesToAllocate));
        soundAsset.numSamples = numFileSamples;

        i16 *sampleData = (i16 *)((u8 *)fileData + sizeof(wav_header));
        for (i32 i = 0; i < numFileSamples; ++i) {
            soundAsset.samples[i] = sampleData[i];
        }
    }

    sound_asset_hash_mapStore(&assetMan->sounds, soundAsset, key);
}

void parseAtlasData (asset_man *assetMan, atlas_asset *atlas, u8 *data) {
    // texture packer libgdx format
    char *start, *end, *nextLineStart;
    start = (char *)data;
    readToNextLine(start, &start, &end, &nextLineStart); // skip file name
    start = nextLineStart;
    readToNextLine(start, &start, &end, &nextLineStart);

    char *wordStart = start;
    char *wordEnd = wordStart;
    while (*wordEnd != ' ' && wordEnd != end) { ++wordEnd; }
    ++wordEnd;
    wordStart = wordEnd;
    while (*wordEnd != ',' && wordEnd != end) { ++wordEnd; }
    atlas->width = stringToU32(wordStart, wordEnd-1);

    wordEnd+=2;
    wordStart = wordEnd;
    while (*wordEnd != '\n' && *wordEnd != '\r' && wordEnd != end) { ++wordEnd; }
    atlas->height = stringToU32(wordStart, wordEnd-1);

    start = nextLineStart;
    // skip format, filter, repeat
    readToNextLine(start, &start, &end, &nextLineStart);
    start = nextLineStart;
    readToNextLine(start, &start, &end, &nextLineStart);
    start = nextLineStart;
    readToNextLine(start, &start, &end, &nextLineStart);
    start = nextLineStart;

    char *frameInfoStart = start;
    u32 numFrames = 0;
    while (true) {
        for (u32 i = 0; i < 7; ++i) {
            readToNextLine(start, &start, &end, &nextLineStart);
            start = nextLineStart;
        }
        numFrames++;
        if (*(nextLineStart) == 0) { break; }
    }

    // allocate enough atlas_frames
    ASSERT(numFrames < ATLAS_MAP_MAX_NUM_FRAMES);
    for (u32 i = 0; i < ATLAS_MAP_MAX_NUM_FRAMES; ++i) {
        zeroMemory((u8 *)&atlas->map.entries[i], sizeof(atlas_frame));
    }
    // TODO(ebuchholz): get rid of this first scan or change structure of map

    // TODO(ebuchholz): more convenient file reading/string copying functions
    start = frameInfoStart;
    for (u32 i = 0; i < numFrames; ++i) {
        atlas_frame frame;
        readToNextLine(start, &start, &end, &nextLineStart);
        u32 length = (u32)(end - start);
        char *frameName = (char *)allocMemory(&assetMan->memory, sizeof(char) * length+1);
        for (u32 j = 0; j < length; ++j) {
            frameName[j] = start[j];
        }
        frameName[length] = 0;
        frame.key = frameName;

        start = nextLineStart;
        readToNextLine(start, &start, &end, &nextLineStart);
        start = nextLineStart;
        readToNextLine(start, &start, &end, &nextLineStart);

        start += 6;
        wordStart = start;
        wordEnd = wordStart;
        while (*wordEnd != ',' && wordEnd != end) { ++wordEnd; }
        u32 frameX = stringToU32(wordStart, wordEnd-1);

        wordEnd += 2;
        wordStart = wordEnd;
        while (*wordEnd != '\n' && *wordEnd != '\r' && wordEnd != end) { ++wordEnd; }
        u32 frameY = stringToU32(wordStart, wordEnd-1);

        start = nextLineStart;
        readToNextLine(start, &start, &end, &nextLineStart);

        start += 8;
        wordStart = start;
        wordEnd = wordStart;
        while (*wordEnd != ',' && wordEnd != end) { ++wordEnd; }
        u32 frameWidth = stringToU32(wordStart, wordEnd-1);

        wordEnd += 2;
        wordStart = wordEnd;
        while (*wordEnd != '\n' && *wordEnd != '\r' && wordEnd != end) { ++wordEnd; }
        u32 frameHeight = stringToU32(wordStart, wordEnd-1);

        // skip the rest of the frame info
        start = nextLineStart;
        readToNextLine(start, &start, &end, &nextLineStart);
        start = nextLineStart;
        readToNextLine(start, &start, &end, &nextLineStart);
        start = nextLineStart;
        readToNextLine(start, &start, &end, &nextLineStart);
        start = nextLineStart;

        frame.frameWidth = frameWidth;
        frame.frameHeight = frameHeight;

        f32 uvWidth = (f32)frameWidth / (f32)atlas->width;
        f32 uvHeight = (f32)frameHeight / (f32)atlas->height;
        frame.frameCorners[0] = (vec2){
          .x = (f32)frameX / (f32)atlas->width,
          .y = (f32)frameY / (f32)atlas->height
        };
        frame.frameCorners[1] = (vec2){
          .x = frame.frameCorners[0].x + uvWidth,
          .y = frame.frameCorners[0].y
        };
        frame.frameCorners[2] = (vec2){
            .x = frame.frameCorners[0].x, 
            .y = frame.frameCorners[0].y + uvHeight
        };
        frame.frameCorners[3] = (vec2){
            .x = frame.frameCorners[0].x + uvWidth, 
            .y = frame.frameCorners[0].y + uvHeight
        };

        u32 hash = hashMapHashString(frameName);
        u32 mapIndex = hash % ATLAS_MAP_MAX_NUM_FRAMES;
        atlas_frame *existingFrame = &atlas->map.entries[mapIndex];
        while (existingFrame->key != 0) {
            mapIndex = (mapIndex + 1) % ATLAS_MAP_MAX_NUM_FRAMES;
            existingFrame = &atlas->map.entries[mapIndex];
        }
        atlas->map.entries[mapIndex] = frame;
    }
}

// TODO: just use generic hash map, this seems to have been implemented before that existed
atlas_frame *getAtlasFrame (asset_man *assetMan, char *atlasKey, char *frameName) {
    atlas_frame *result;
    atlas_asset *textureAtlas = getAtlas(assetMan, atlasKey);
    ASSERT(textureAtlas != 0);

    u32 hash = hashMapHashString(frameName);
    u32 mapIndex = hash % ATLAS_MAP_MAX_NUM_FRAMES;
    u32 originalMapIndex = mapIndex; // check that we actually find it after looping all the way around
    while (true) {
        result = &textureAtlas->map.entries[mapIndex];
        if (stringEquals(frameName, result->key)) {
            break;
        }
        else {
            mapIndex = (mapIndex + 1) % ATLAS_MAP_MAX_NUM_FRAMES;
            ASSERT(mapIndex != originalMapIndex);
        }
    }

    return result;
}

void loadDataFile (asset_man *assetMan, char *key, void *assetData, u32 size) {
    void *copiedData = allocMemory(&assetMan->memory, size); 
    copyMemory(copiedData, assetData, size);

    data_asset dataAsset = {
        .data = copiedData,
        .size = size
    };

    data_asset_hash_mapStore(&assetMan->dataAssets, dataAsset, key);
}

void initGameAssets (asset_man *am, plat_api *platAPI) {
    assetMan = am;

    assetMan->memory = (mem_arena){
        .base = platAPI->assetMemory,
        .current = (u8 *)platAPI->assetMemory,
        .capacity = platAPI->assetMemorySize
    };
    assetMan->assetToLoadList = asset_to_load_listInit(&assetMan->memory, MAX_NUM_ASSETS_TO_LOAD);
    assetMan->textures = texture_asset_hash_mapInit(&assetMan->memory, MAX_NUM_TEXTURE_ASSETS);
    assetMan->atlases = atlas_asset_hash_mapInit(&assetMan->memory, MAX_NUM_ATLAS_ASSETS);
    assetMan->sounds = sound_asset_hash_mapInit(&assetMan->memory, MAX_NUM_SOUND_ASSETS);
    assetMan->dataAssets = data_asset_hash_mapInit(&assetMan->memory, MAX_NUM_DATA_ASSETS);
}
