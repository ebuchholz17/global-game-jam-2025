#ifndef BLOCK_GAME_H

#include "../gng_types.h"
#include "../gng_platform.h"

#define BLOCK_GAME_H

#define NUM_GRID_ROWS 10
#define NUM_GRID_COLS 10
#define GRID_BLOCK_WIDTH 16.0f
#define GRID_BLOCK_HEIGHT 16.0f

#define BLOCK_MOVE_SPEED 0.15f

typedef enum {
    DIRECTION_NONE,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} direction;

// TODO(ebuchholz): group together "movable block" stuff
typedef struct block_sheep {
    b32 moving;
    f32 moveTime;

    i32 row;
    i32 col;
    i32 prevCol;
    i32 prevRow;
    f32 x;
    f32 y;
} block_sheep;

typedef struct grid_block {
    char *color;
    i32 id;
    b32 active;

    b32 moving;
    f32 moveTime;

    i32 row;
    i32 col;
    i32 prevCol;
    i32 prevRow;
    f32 x;
    f32 y;
} grid_block;

typedef struct block_piece {
    i32 filledCells[25];
    char *color;
} block_piece;

typedef enum {
    BLOCK_GAME_STATE_MOVING_BLOCKS,
    BLOCK_GAME_STATE_CLEARING_LINES
} block_game_state;

typedef struct block_game_input {
    input_key up;
    input_key down;
    input_key left;
    input_key right;
    input_key a;
    input_key z;
} block_game_input;

typedef struct animation_state {
    char *key;
    u32 currentFrame;
    u32 currentFrameStep;
} animation_state;

typedef char_anim_data *char_anim_data_ptr;
#define HASH_MAP_TYPE char_anim_data_ptr
#include "../hash_map.h"

typedef enum {
    INPUT_SOURCE_KEYBOARD,
    INPUT_SOURCE_GAMEPAD,
    INPUT_SOURCE_VIRTUAL
} input_source;

typedef struct block_game {
    b32 isInitialized;

    grid_block blocks[NUM_GRID_ROWS * NUM_GRID_COLS];
    i32 numBlocks;

    i32 grid[NUM_GRID_ROWS * NUM_GRID_COLS];

    block_sheep sheep;
    direction nextMoveDirection;

    i32 score;

    f32 timeToSpawnNextPiece;
    f32 nextPieceTimer;

    block_piece nextPiece;
    i32 nextBlockPieceRow;
    i32 nextBlockPieceCol;

    f32 blockIndicatorTimer;
    b32 fadingInBlockIndicator;

    block_game_state gameState;

    // TODO(ebuchholz): bit flags?
    b32 rowsBeingCleared[NUM_GRID_ROWS];
    b32 colsBeingCleared[NUM_GRID_COLS];
    b32 clearedBlocksVisible;
    f32 clearingBlocksTimer;

    input_source inputSource;
    b32 animationsLoaded;
    b32 sheepAnimating;
    animation_state sheepAnimState;
    char_anim_data_ptr_hash_map animations;
} block_game;


#endif
