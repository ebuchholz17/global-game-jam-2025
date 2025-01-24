#ifndef HITBOX_H
#define HITBOX_H

#include "../gng_assets.h"

#define MAX_NUM_HITBOXES 10
#define MAX_NUM_HURTBOXES 10

typedef struct char_frame_data {
    char *frameKey;
    i32 xOffset;
    i32 yOffset;
    u32 duration;
    u32 numHitboxes;
    rect hitboxes[MAX_NUM_HITBOXES];
    u32 numHurtboxes;
    rect hurtboxes[MAX_NUM_HURTBOXES];
} char_frame_data;

typedef struct char_anim_data {
    char *key;
    char_frame_data frames[20];
    u32 numFrames;
} char_anim_data;
#define LIST_TYPE char_anim_data
#include "../list.h"

void loadHitboxData (asset_man *assetMan, char *atlasKey, void *fileData, char_anim_data *animationData);

#endif
