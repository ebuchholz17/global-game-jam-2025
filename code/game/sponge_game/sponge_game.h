#ifndef SPONGE_GAME_H
#define SPONGE_GAME_H

#include "../gng_types.h"
#include "../gng_platform.h"
#include "../gng_math.h"
#include "../hitbox/hitbox.h"

#define SPONGE_ACCEL_SPD 1200.0f
#define SPONGE_DECEL_SPD 200.0f
#define SPONGE_DASH_START_SPD 130.0f
#define SPONGE_MAX_RUN_SPD 115.0f
#define SPONGE_JUMP_VEL 250.0f
#define SPONGE_GRAVITY 1000.0f
#define SPONGE_AIR_CONTROL 500.0f
#define FRAMETIME (1.0f / 60.0f)

#define MAX_NUM_COCKROACHES 10

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
    input_key attack;
    input_key special;
    input_key shield;
} SpongeGameInput;

typedef enum {
    SG_INPUT_BUTT_UP,
    SG_INPUT_BUTT_DOWN,
    SG_INPUT_BUTT_LEFT,
    SG_INPUT_BUTT_RIGHT,
    SG_INPUT_BUTT_JUMP,
    SG_INPUT_BUTT_ATTACK,
    SG_INPUT_BUTT_SPECIAL
} SpongeGameInputButtons;

typedef enum {
    F_ANIM_STATE_IDLE,
    F_ANIM_STATE_JUMPSQUAT,
    F_ANIM_STATE_RUN,
    F_ANIM_STATE_JUMP_RISING,
    F_ANIM_STATE_JUMP_FALLING,
    F_ANIM_STATE_GETUP,
    F_ANIM_STATE_HIT,
    F_ANIM_STATE_DEAD,
} FighterAnimState;

typedef enum {
    F_STATE_STAND,
    F_STATE_DEAD,
    F_STATE_DASH_START,
    F_STATE_DASH,
    F_STATE_JUMPSQUAT,
    F_STATE_JUMP,
    F_STATE_LANDING,
    F_STATE_FALL,
    F_STATE_ATTACKING_GROUNDED,
    F_STATE_ATTACKING_AIRBORNE,
    F_STATE_HIT,
    F_STATE_SUICIDE_DAIR,
    F_STATE_SHIELD,
    F_STATE_SHIELD_STUN,
} FighterState;

typedef enum {
    F_ATTACK_NONE,
    F_ATTACK_JAB,
    F_ATTACK_LOW_KICK,
    F_ATTACK_SUICIDE_DAIR,
    F_ATTACK_BUBBLE_ATTACK,
    F_ATTACK_UPPERCUT,
    F_ATTACK_FP,
    F_ATTACK_BUBBLE_LAUNCH,
    F_ATTACK_SLIDE,
    F_ATTACK_DP,

    F_ATTACK_C_JAB,
    F_ATTACK_C_KICK
} FighterAttackType;

typedef struct Attack {
    FighterAttackType type;
    b32 isChargeable;
} Attack;

typedef struct FighterAttack {
    b32 active;
    f32 damage;
    u32 id;
    f32 knockbackMultiplier;
} FighterAttack;

typedef struct FighterHitByInfo {
    b32 wasHit;
    u32 attackID;
    FighterAttackType attackType;
    vec2 attackOrigin;
    f32 damage;
    f32 knockbackSpeed;
} FighterHitByInfo;

typedef enum {
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} Direction;

typedef struct AnimationState {
    char *key;
    char *prevKey;
    f32 t;
    f32 speedMultiplier;
    u32 currentFrame;
    u32 currentFrameStep;
    u32 totalFrames;
    b32 dontLoop;
} AnimationState;

typedef enum {
    FIGHTER_TYPE_SPONGE,
    FIGHTER_TYPE_COCKROACH
} FighterType;

typedef struct Fighter {
    FighterType type;

    vec2 pos;
    vec2 vel;
    f32 fullJumpAntiGrav;
    FighterState state;
    f32 jumpTime;
    b32 releasedJump;
    b32 grounded;
    b32 onPlatform;

    FighterAttack attack;

    AnimationState animState;
    Direction facing;
    f32 stateTimer;
    b32 isAttacking;
    FighterAttackType attackType;
    b32 isJumping;
    f32 landingLag;

    f32 knockbackTimer;
    f32 hitstunTimer;
    f32 iframesTimer;

    f32 defeatedTimer;
    f32 fadingTimer;

    f32 hitPoints;
    b32 dodging;
    u32 lastAttackHitByID;
    FighterHitByInfo hitByInfo;
    f32 passedThroughPlatformTimer;
    b32 justHitEnemy;
    b32 isShielding;
} Fighter;

typedef struct SpongeMan {
    Fighter fighter;
} SpongeMan;

typedef enum {
    COCKROACH_AI_STATE_THINKING,
    COCKROACH_AI_STATE_APPROACHING,
    COCKROACH_AI_STATE_MED_RANGE,
    COCKROACH_AI_STATE_SHORT_RANGE,
    COCKROACH_AI_STATE_WAS_HIT,
    COCKROACH_AI_STATE_HIT_PLAYER
} CockroachAIState;

typedef enum {
    COCKROACH_AI_DECISION_NONE,
    COCKROACH_AI_DECISION_WALK,
    COCKROACH_AI_DECISION_JUMP,
    COCKROACH_AI_DECISION_DASH_DANCE,
    COCKROACH_AI_DECISION_RUN_AWAY,
    COCKROACH_AI_DECISION_ATTACK,
    COCKROACH_AI_DECISION_DODGE,
    COCKROACH_AI_DECISION_WAIT,
    COCKROACH_AI_DECISION_JUMP_AWAY
} CockroachAIDecision;

typedef struct Cockroach {
    Fighter fighter;
    SpongeGameInput input;
    CockroachAIState aiState;
    CockroachAIDecision aiDecision;
    FighterAttackType aiAttack;
    f32 decisionTimer;
    f32 globalTimer;

    b32 didAttack;
    b32 didJump;
    b32 shortHop;
    b32 attackHitPlayer;
    f32 walkRange;
} Cockroach;

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
    f32 grimeTime;
    f32 sudTime;
} TileCoating;

typedef struct SudEmitter {
    vec2 pos;
    f32 minVel;
    f32 maxVel;
    f32 t;
    f32 timeAlive;
    f32 rate;
    f32 duration;
    f32 minAngle;
    f32 maxAngle;
} SudEmitter;

typedef struct Sud {
    b32 active;
    b32 flyToSponge;
    vec2 pos;
    vec2 vel;
} Sud;

typedef char_anim_data *char_anim_data_ptr;
#define HASH_MAP_TYPE char_anim_data_ptr
#include "../hash_map.h"

#define LIST_TYPE Cockroach
#include "../list.h"

#define LIST_TYPE SudEmitter
#include "../list.h"

#define MAX_NUM_SUDS 200

typedef struct SpongeGame {
    b32 isInitialized;
    SpongeMan spongeMan;

    Cockroach_list cockroaches;
    f32 cockroachTimer;
    SudEmitter_list sudEmitters;
    Sud suds[MAX_NUM_SUDS];
    i32 numSuds;

    InputSource inputSource;
    SpongePlatform platforms[NUM_SPONGE_PLATFORMS];
    TileCoating levelTileCoatings[NUM_TILE_ROWS * NUM_TILE_COLS];

    char_anim_data_ptr_hash_map animations;
    Fighter *lastHitEnemy;
} SpongeGame;

#endif

