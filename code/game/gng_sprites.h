#ifndef GNG_SPRITES_H
#define GNG_SPRITES_H

#include "gng_types.h"
#include "gng_math.h"
#include "gng_assets.h"

#define SPRITE_LIST_MAX_NUM_SPRITES 10000
#define SPRITE_LIST_SPRITE_TEX_UNSET ((u32)(0-1))

typedef struct sprite {
    mat3x3 parentTransform;

    vec2 pos;
    f32 scale;
    f32 rotation;

    vec2 anchor;

    f32 alpha;
    u32 tint;

    u32 textureID;
    char *textureKey;
    char *atlasKey;
    char *frameKey;

    vec2 frameCorners[4];
    f32 width;
    f32 height;
} sprite;

#define LIST_TYPE sprite
#include "list.h"

#define SPRITE_LIST_MAT_STACK_MAX 20

typedef struct bitmap_font_letter_coord {
    f32 x;
    f32 y;
    f32 advance;
} bitmap_font_letter_coord;

typedef struct sprite_man {
    mat3x3 matrixStack[SPRITE_LIST_MAT_STACK_MAX];
    unsigned int matrixStackIndex;

    sprite_list sprites;
    bitmap_font_letter_coord fontLetterCoords[255];

    asset_man *assetMan;
} sprite_man;

typedef struct sprite_transform {
    vec2 pos;
    f32 scale;
    f32 rotation;
} sprite_transform;

typedef struct sprite_text {
    f32 x;
    f32 y;
    char *text;
    char *fontKey;
} sprite_text;

sprite defaultSprite (void);
void spriteManAddSprite (sprite s);

void spriteManPushMatrix (mat3x3 transform);

mat3x3 spriteManPeekMatrix ();

mat3x3 spriteManPopMatrix ();

void spriteManPushTransform (sprite_transform transform) ;
void spriteManAddText (sprite_text textInfo) ;

#endif
