#ifndef SPONGE_GAME_H
#define SPONGE_GAME_H

#include "../gng_types.h"
#include "../gng_platform.h"
#include "../gng_math.h"

#define SPONGE_ACCEL_SPD 1200.0f
#define SPONGE_DECEL_SPD 700.0f
#define SPONGE_MAX_RUN_SPD 150.0f
#define SPONGE_JUMP_VEL 300.0f
#define SPONGE_GRAVITY 1500.0f
#define SPONGE_AIR_CONTROL 600.0f

#define NUM_SPONGE_PLATFORMS 4
#define GROUND_Y 160.0f

#define NUM_TILE_ROWS 25
#define NUM_TILE_COLS 45

//typedef char_anim_data *char_anim_data_ptr;
//#define HASH_MAP_TYPE char_anim_data_ptr
//#include "../hash_map.h"

typedef enum {
    INPUT_SOURCE_KEYBOARD,
    INPUT_SOURCE_GAMEPAD,
    INPUT_SOURCE_VIRTUAL
} InputSource;

typedef struct SpongeGameInput {
    input_key up;
    input_key down;
    input_key left;
    input_key right;
    input_key jump;
} SpongeGameInput;

typedef enum {
    SM_STATE_STAND,
    SM_STATE_RUN,
    SM_STATE_JUMP
} SpongeManState;

typedef struct SpongeMan {
    vec2 pos;
    vec2 vel;
    f32 fullJumpAntiGrav;
    SpongeManState state;
    f32 jumpTime;
    b32 releasedJump;
    b32 grounded;
    b32 onPlatform;
} SpongeMan;

typedef struct SpongePlatform {
    vec2 pos;
    f32 length;
} SpongePlatform;

typedef enum {
    TILE_COATING_TYPE_NONE,
    TILE_COATING_TYPE_SUDS,
    TILE_COATING_TYPE_GRIME
} TileCoatingType;

typedef struct TileCoating {
    TileCoatingType type;
    i32 amount;
    b32 isGround;
} TileCoating;

typedef struct SpongeGame {
    b32 isInitialized;
    SpongeMan spongeMan;

    InputSource inputSource;
    //char_anim_data_ptr_hash_map animations;
    //
    SpongePlatform platforms[NUM_SPONGE_PLATFORMS];
    TileCoating levelTileCoatings[NUM_TILE_ROWS * NUM_TILE_COLS];
} SpongeGame;


#endif

