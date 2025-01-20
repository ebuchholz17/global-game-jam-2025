#ifndef GNG_PLATFORM_H
#define GNG_PLATFORM_H

#include "gng_types.h"
#include "gng_memory.h"

#if defined(_GNG_WEB)
#define ASSERT(exp) if (!(exp)) { __builtin_trap(); }
#else
#define ASSERT(exp) if (!(exp)) { *((int *)0) = 0xDEAD; }
#endif
#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

typedef enum {
    RENDER_CMD_TYPE_SPRITE_BATCH_START,
    RENDER_CMD_TYPE_SPRITE_BATCH_DRAW,
    RENDER_CMD_TYPE_SPRITE_BATCH_END,
    RENDER_CMD_TYPE_LOAD_TEXTURE
} render_cmd_type;

typedef struct render_cmd_header {
    render_cmd_type type;
} render_cmd_header;

typedef struct render_cmd_sprite_data {
    f32 positions[12];
    f32 texCoords[8];
    f32 colors[16];
    u32 textureID;
} render_cmd_sprite_data;

typedef struct render_cmd_sprite_batch_draw {
    render_cmd_sprite_data *sprites;
    u32 numSprites;
} render_cmd_sprite_batch_draw;

typedef struct render_cmd_load_texture {
    u32 id;
    u32 width;
    u32 height;
    u8 *pixels; // R G B A
} render_cmd_load_texture;

typedef void (plat_console_log)(char *);
typedef void (plat_load_file)(char *, char *);
typedef u64 (plat_rng_seed_from_time)(void);

typedef struct plat_api {
    plat_console_log *consoleLog;
    plat_load_file *loadFile;
    plat_rng_seed_from_time *rngSeedFromTime;

    void *mainMemory;
    u64 mainMemorySize;

    void *assetMemory;
    u64 assetMemorySize;

    void *scratchMemory;
    u64 scratchMemorySize;

    u32 windowWidth;
    u32 windowHeight;
    u32 audioSampleRate;

    b32 hasTouchControls;
} plat_api;

typedef struct input_key {
    b32 down;
    b32 justPressed;
} input_key;

// based on xbox 360 controller/xinput
typedef struct game_controller_input {
    b32 connected;

    input_key dPadUp;
    input_key dPadDown;
    input_key dPadLeft;
    input_key dPadRight;

    input_key start;
    input_key back;

    input_key leftStick;
    input_key rightStick;

    input_key leftBumper;
    input_key rightBumper;

    input_key aButton;
    input_key bButton;
    input_key xButton;
    input_key yButton;

    f32 leftTrigger;
    input_key leftTriggerButton;
    f32 rightTrigger;
    input_key rightTriggerButton;

    f32 leftStickX;
    f32 leftStickY;
    f32 rightStickX;
    f32 rightStickY;

    input_key leftStickUp;
    input_key leftStickDown;
    input_key leftStickLeft;
    input_key leftStickRight;

    input_key rightStickUp;
    input_key rightStickDown;
    input_key rightStickLeft;
    input_key rightStickRight;
} game_controller_input;
#define MAX_NUM_CONTROLLERS 4

typedef struct touch_input {
    b32 active;
    input_key touchState;
    u32 x;
    u32 y;
    f32 radiusX;
    f32 radiusY;
} touch_input;
#define MAX_NUM_TOUCHES 4

typedef struct game_input {
    input_key upArrow;
    input_key downArrow;
    input_key leftArrow;
    input_key rightArrow;

    input_key aKey;
    input_key sKey;
    input_key zKey;
    input_key xKey;

    game_controller_input controllers[MAX_NUM_CONTROLLERS];

    b32 pointerDown;
    b32 pointerJustDown;
    u32 pointerX;
    u32 pointerY;

    touch_input touches[MAX_NUM_TOUCHES];
} game_input;

typedef struct plat_loaded_file {
    char *id;
    void *data;
    u32 dataSize;
} plat_loaded_file;

typedef struct sound_sample {
    f32 value;
} sound_sample;

typedef struct game_sound_output {
    i32 samplesPerSecond;
    i32 sampleCount;
    sound_sample *samples;
} game_sound_output;

#define UPDATE_GNG_GAME(name) void name(plat_api platAPI, mem_arena *renderMemory, game_input *input, f32 dt, u32 numLoadedFiles, plat_loaded_file *loadedFiles)
typedef UPDATE_GNG_GAME(update_gng_game);

#define GET_SOUND_SAMPLES_GNG_GAME(name) void name(plat_api platAPI, game_sound_output *gameSoundOutput)
typedef GET_SOUND_SAMPLES_GNG_GAME(get_sound_samples_gng_game);

typedef struct game_api {
    update_gng_game *updateGNGGame;
    get_sound_samples_gng_game *getSoundSamplesGNGGame;
} game_api;

#endif
