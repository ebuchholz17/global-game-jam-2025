#include "sponge_game.h"
#include "../gng_bool.h"
#include "../gng_math.h"
#include "../gng_sprites.h"
#include "../gng_virtual_input.h"
#include "../hitbox/hitbox.h"

SpongeGame *spongeGame;

void loadSpongeHitboxData (char *key, mem_arena *memory) {
    data_asset *hitboxData = getDataAsset(key);

    char_anim_data *animData = (char_anim_data *)allocMemory(memory, sizeof(char_anim_data));
    animData->key = key;
    loadHitboxData(assetMan, "sponge_atlas", hitboxData->data, animData);

    char_anim_data_ptr_hash_mapStore(&spongeGame->animations, animData, key);
}

SpongeGameInput parseGameInput (game_input *input, virtual_input *vInput) {
    SpongeGameInput result = {0};

    if (input->leftArrow.down || 
        input->rightArrow.down || 
        input->upArrow.down || 
        input->downArrow.down ||
        input->aKey.down) 
    {
        spongeGame->inputSource = INPUT_SOURCE_KEYBOARD;
    }
    else if (vInput->dPadUp.button.down || 
             vInput->dPadDown.button.down || 
             vInput->dPadLeft.button.down || 
             vInput->dPadRight.button.down)
    {
        spongeGame->inputSource = INPUT_SOURCE_VIRTUAL;
    }
    else {
        for (u32 controllerIndex = 0; controllerIndex < MAX_NUM_CONTROLLERS; controllerIndex++) {
            game_controller_input *cont = &input->controllers[controllerIndex];

            if (cont->connected) {
                b32 useController;
                if (cont->dPadUp.down || cont->dPadLeft.down || cont->dPadDown.down || cont->dPadRight.down) {
                    spongeGame->inputSource = INPUT_SOURCE_GAMEPAD;
                    break;
                }
            }
        }
    }

    switch (spongeGame->inputSource) {
        case INPUT_SOURCE_KEYBOARD: {
            result.left = input->leftArrow;
            result.right = input->rightArrow;
            result.up = input->upArrow;
            result.down = input->downArrow;
            result.jump = input->aKey;
        } break;
        case INPUT_SOURCE_VIRTUAL: {
            result.up = vInput->dPadUp.button;;
            result.down = vInput->dPadDown.button;
            result.left = vInput->dPadLeft.button;
            result.right = vInput->dPadRight.button;
            result.jump = vInput->bottomButton.button;
        } break;
        case INPUT_SOURCE_GAMEPAD: {
            for (u32 controllerIndex = 0; controllerIndex < MAX_NUM_CONTROLLERS; controllerIndex++) {
                game_controller_input *cont = &input->controllers[controllerIndex];

                if (cont->connected) {
                    result.left = cont->dPadLeft;
                    result.right = cont->dPadRight;
                    result.up = cont->dPadUp;
                    result.down = cont->dPadDown;
                    result.jump = cont->aButton;
                    break;
                }
            }
        } break;
    }

    return result;
}

void startAnimState (AnimationState *animState) {
    animState->currentFrameStep = 0;
    animState->currentFrame = 0;
    animState->totalFrames = 0;
    animState->prevKey = animState->key;
}

b32 updateAnimState (AnimationState *animState, f32 dt) {
    char_anim_data *animData = char_anim_data_ptr_hash_mapGetVal(&spongeGame->animations, animState->key);
    char_frame_data *currentFrame = &animData->frames[animState->currentFrame];

    b32 animationComplete = false;

    f32 timePerFrame = 1.0f / 60.0f;

    u32 startFrame = animState->currentFrame;

    animState->t += dt;
    //animState->t += dt * animState->speedMultiplier;
    while (animState->t >= timePerFrame) {
        animState->t -= timePerFrame;
        ++animState->currentFrameStep;
        ++animState->totalFrames;

        if (animState->currentFrameStep >= currentFrame->duration) {
            animState->currentFrameStep = 0;
            ++animState->currentFrame;
            if (animState->currentFrame == animData->numFrames) {
                animState->currentFrame = 0;
                animationComplete = true;
            }
            // avoid skipping over hitbox frames
            if (animState->currentFrame - startFrame > 1) {
                break;
            }
        }
    }

    return animationComplete;
}


void updateRunningInput (SpongeMan *sm, SpongeGameInput *input, f32 dt) {
    if (input->left.justPressed && sm->facing == DIRECTION_RIGHT) {
        sm->facing = DIRECTION_LEFT;
        sm->animState = (AnimationState){ .key = "sponge_run" };
        startAnimState(&sm->animState);
    }
    if (input->right.justPressed && sm->facing == DIRECTION_LEFT) {
        sm->facing = DIRECTION_RIGHT;
        sm->animState = (AnimationState){ .key = "sponge_run" };
        startAnimState(&sm->animState);
    }

    if (input->left.down) {
        sm->vel.x -= SPONGE_ACCEL_SPD * dt;
    }
    if (input->right.down) {
        sm->vel.x += SPONGE_ACCEL_SPD * dt;
    }
    if (!input->left.down && !input->right.down) {
        if (sm->vel.x > 0.0f) {
            sm->vel.x -= SPONGE_DECEL_SPD * dt;
            if (sm->vel.x < 0.0f) {
                sm->vel.x = 0.0f;
            }
        }
        else if (sm->vel.x < 0) {
            sm->vel.x += SPONGE_DECEL_SPD * dt;
            if (sm->vel.x > 0.0f) {
                sm->vel.x = 0.0f;
            }
        }
    }
    if (sm->vel.x > SPONGE_MAX_RUN_SPD) {
        sm->vel.x = SPONGE_MAX_RUN_SPD;
    }
    if (sm->vel.x < -SPONGE_MAX_RUN_SPD) {
        sm->vel.x = -SPONGE_MAX_RUN_SPD;
    }
}


void updateJumpingState (SpongeMan *sm, SpongeGameInput *input, f32 dt) {
    sm->jumpTime += dt;
    if (!input->jump.down) {
        sm->releasedJump = true;
    }

    sm->fullJumpAntiGrav = 0.0f;
    if (sm->jumpTime < 0.3f && !sm->releasedJump) {
        sm->fullJumpAntiGrav = SPONGE_GRAVITY * 0.7f;
    }

    if (input->left.down) {
        sm->vel.x -= SPONGE_AIR_CONTROL * dt;
    }
    if (input->right.down) {
        sm->vel.x += SPONGE_AIR_CONTROL * dt;
    }
    
    // run max speed = aerial max speed
    if (sm->vel.x > SPONGE_MAX_RUN_SPD) {
        sm->vel.x = SPONGE_MAX_RUN_SPD;
    }
    if (sm->vel.x < -SPONGE_MAX_RUN_SPD) {
        sm->vel.x = -SPONGE_MAX_RUN_SPD;
    }

    if (sm->vel.y > 0) {
        sm->animState = (AnimationState){ .key = "sponge_jump_falling" };
        startAnimState(&sm->animState);
    }

}

void enterJumpsquatState (SpongeMan *sm, SpongeGameInput *input, f32 dt) {
    sm->stateFrames = 0;
    sm->releasedJump = false;
    sm->state = SM_STATE_JUMPSQUAT;
    sm->animState = (AnimationState){ .key = "sponge_jumpsquat" };
    startAnimState(&sm->animState);
}

void enterJumpingState (SpongeMan *sm, SpongeGameInput *input, f32 dt) {
    sm->grounded = false;
    sm->jumpTime = 0.0f;
    sm->releasedJump = false;
    sm->vel.y = -SPONGE_JUMP_VEL;
    sm->state = SM_STATE_JUMP;
    sm->animState = (AnimationState){ .key = "sponge_jump_rising" };
    startAnimState(&sm->animState);
}

void updateJumpsquatState (SpongeMan *sm, SpongeGameInput *input, f32 dt) {
    if (!input->jump.down) {
        sm->releasedJump = true;
    }
    sm->stateFrames++;
    if (sm->stateFrames >= 3) {
        enterJumpingState(sm, input, dt);
    }
}

vec2 spongeManFeet (void) {
    SpongeMan *sm = &spongeGame->spongeMan;
    f32 feetDist = 16.0f;
    vec2 feet = vec2Add(sm->pos, (vec2){ .x = 0.0f, .y = feetDist });
    return feet;
}

b32 isTouchingPlatform (SpongeMan *sm, f32 *outPlatY) {
    f32 feetDist = 16.0f;
    vec2 feet = vec2Add(sm->pos, (vec2){ .x = 0.0f, .y = feetDist });
    for (i32 i = 0; i < NUM_SPONGE_PLATFORMS; i++) {
        SpongePlatform *p = &spongeGame->platforms[i];
        if (feet.x >= p->pos.x && feet.x < p->pos.x + p->length &&
            feet.y >= p->pos.y && feet.y < p->pos.y + 8)
        {
            *outPlatY = p->pos.y;
            return true;
        }
    }
    return false;
}

void updateSpongeManState (SpongeMan *sm, SpongeGameInput *input, f32 dt) {

    switch (sm->state) {
        case SM_STATE_STAND: {
            if (input->jump.justPressed) {
                enterJumpsquatState(sm, input, dt);
            }
            else if (input->left.justPressed || input->right.justPressed) {
                if (input->left.justPressed) {
                    sm->facing = DIRECTION_LEFT;
                }
                else if (input->right.justPressed) {
                    sm->facing = DIRECTION_RIGHT;
                }
                sm->state = SM_STATE_DASH;
                sm->animState = (AnimationState){ .key = "sponge_run" };
                startAnimState(&sm->animState);
                updateRunningInput(sm, input, dt);
            }
        } break;
        case SM_STATE_JUMPSQUAT: {
             updateJumpsquatState(sm, input, dt);
        } break;
        case SM_STATE_DASH: {
            if (input->jump.justPressed) {
                enterJumpsquatState(sm, input, dt);
            }
            else {
                updateRunningInput(sm, input, dt);
            }

            if (sm->vel.x == 0.0f) {
                sm->state = SM_STATE_STAND;
                sm->animState = (AnimationState){ .key = "sponge_idle" };
                startAnimState(&sm->animState);
            }
        } break;
        case SM_STATE_JUMP: {
            updateJumpingState(sm, input, dt);
            if (sm->grounded) {
                sm->state = SM_STATE_DASH;
                sm->animState = (AnimationState){ .key = "sponge_run" };
                startAnimState(&sm->animState);
            }
        } break;
    }

    f32 iterations = 10;
    f32 dtFrac = dt / (f32)iterations;
    for (i32 i = 0; i < iterations; i++) {
        sm->pos.x += sm->vel.x * dtFrac;
        if (sm->grounded) {
            f32 platY;
            if (sm->onPlatform && !isTouchingPlatform(sm, &platY)) {
                sm->onPlatform = false;
                sm->grounded = false;
                // falling
                sm->animState = (AnimationState){ .key = "sponge_jump_falling" };
                startAnimState(&sm->animState);
            }
        }
        if (!sm->grounded) {
            sm->vel.y += SPONGE_GRAVITY * dtFrac - sm->fullJumpAntiGrav * dtFrac;
            sm->pos.y += sm->vel.y * dtFrac;
        }

        f32 feetDist = 16.0f;
        vec2 feet = vec2Add(sm->pos, (vec2){ .x = 0.0f, .y = feetDist });
        if (!sm->grounded) {
            if (feet.y > GROUND_Y) {
                sm->pos.y = GROUND_Y - feetDist;
                sm->grounded = true;
            }
            else if (sm->vel.y > 0) {
                f32 platY;
                if (isTouchingPlatform(sm, &platY)) {
                    sm->pos.y = platY - feetDist;
                    sm->vel.y = 0.0f;
                    sm->grounded = true;
                    sm->onPlatform = true;
                }
            }
        }
    }

    ASSERT(sm->animState.key != 0);
    if (!stringEquals(sm->animState.key, sm->animState.prevKey)) {
        startAnimState(&sm->animState);
    }
    b32 animDone = updateAnimState(&sm->animState, dt);

    // state transitions on finished animations
    //if (animDone) {
    //    if (cm->state == CARDMAN_STATE_ATTACKING) {
    //        cm->state = CARDMAN_STATE_IDLE;
    //        setAnimState(cm, getCardmanIdleAnim(cm));
    //        startAnimState(&cm->animState);
    //    }
    //}
}

void initSpongeGame(SpongeGame *sg, mem_arena *memory) {
    spongeGame = sg;
    zeroMemory((u8 *)spongeGame, sizeof(SpongeGame));

    SpongeMan *spongeMan = &spongeGame->spongeMan;
    spongeMan->pos = (vec2){ .x = 160.0f, .y = GROUND_Y - 16.0f };
    spongeMan->grounded = true;
    spongeMan->facing = DIRECTION_LEFT;

    spongeGame->animations = char_anim_data_ptr_hash_mapInit(memory, 200);
    loadSpongeHitboxData("sponge_idle", memory);
    loadSpongeHitboxData("sponge_jump_falling", memory);
    loadSpongeHitboxData("sponge_jump_rising", memory);
    loadSpongeHitboxData("sponge_jumpsquat", memory);
    loadSpongeHitboxData("sponge_run", memory);

    spongeMan->animState = (AnimationState){ .key = "sponge_idle" };
    startAnimState(&spongeMan->animState);

    spongeGame->platforms[0] = (SpongePlatform){
        .pos = (vec2){
            .x = 64.0f,
            .y = 96.0f,
        },
        .length = 48.0f
    };
    spongeGame->platforms[1] = (SpongePlatform){
        .pos = (vec2){
            .x = 112.0f,
            .y = 112.0f,
        },
        .length = 40.0f
    };
    spongeGame->platforms[2] = (SpongePlatform){
        .pos = (vec2){
            .x = 192.0f,
            .y = 64.0f
        },
        .length = 136.0f
    };
    spongeGame->platforms[3] = (SpongePlatform){
        .pos = (vec2){
            .x = 272.0f,
            .y = 112.0f,
        },
        .length = 68.0f
    };

    for (i32 i = 0; i < NUM_TILE_ROWS; i++) {
        for (i32 j = 0; j < NUM_TILE_COLS; j++) {
            spongeGame->levelTileCoatings[i * NUM_TILE_COLS + j] = (TileCoating){0};
        }
    }


    for (i32 i = 0; i < NUM_SPONGE_PLATFORMS; i++) {
        SpongePlatform *p = &spongeGame->platforms[i];

        for (f32 tileX = p->pos.x; tileX < p->pos.x + p->length; tileX += 8.0f) {
            i32 x = tileX / 8.0f;
            i32 y = p->pos.y / 8.0f;
            TileCoating *c = &spongeGame->levelTileCoatings[y * NUM_TILE_COLS + x];
            c->isGround = true;
        }
    }

    for (i32 j = 0; j < NUM_TILE_COLS; j++) {
        i32 y = GROUND_Y / 8.0f;
        TileCoating *c = &spongeGame->levelTileCoatings[y * NUM_TILE_COLS + j];
        c->isGround = true;
    }

    spongeGame->isInitialized = true;
}

void updateSpongeGame (SpongeGame *spongeGame, game_input *input, virtual_input *vInput, f32 dt, plat_api platAPI, mem_arena *memory) {
    SpongeGameInput sgInput = parseGameInput(input, vInput);
    updateSpongeManState(&spongeGame->spongeMan, &sgInput, dt);

    vec2 feet = spongeManFeet();
    i32 feetX = feet.x / 8.0f;
    i32 feetY = feet.y / 8.0f;
    if (feetX >= 0 && feetX < NUM_TILE_COLS && feet.y >= 0 && feetY < NUM_TILE_ROWS) {
        TileCoating *c = &spongeGame->levelTileCoatings[feetY * NUM_TILE_COLS + feetX];
        if (c->isGround) {
            c->type = TILE_COATING_TYPE_SUDS;
            c->amount = 1;
        }
    }
}

void drawSpongeGame (SpongeGame *spongeGame, plat_api platAPI) { 
    // BACKGROUND
    {
        sprite s = defaultSprite();
        s.textureKey = "background";
        spriteManAddSprite(s);
    }

    // SUDS + GRIME
    for (i32 i = 0; i < NUM_TILE_ROWS; i++) {
        for (i32 j = 0; j < NUM_TILE_COLS; j++) {
            TileCoating *c = &spongeGame->levelTileCoatings[i * NUM_TILE_COLS + j];
            vec2 coatingPos = (vec2){ .x = 8.0f * j, .y = 8.0f * i - 8.0f };
            switch (c->type) {
                case TILE_COATING_TYPE_NONE: {
                    // nothing
                } break;
                case TILE_COATING_TYPE_SUDS: {
                    {
                        sprite s = defaultSprite();
                        s.pos = coatingPos;
                        s.atlasKey = "game_atlas";
                        s.frameKey = "suds_0";
                        spriteManAddSprite(s);
                    }
                } break;
                case TILE_COATING_TYPE_GRIME: {
                    // TODO grime
                } break;
            }
        }
    }

    // SPONGEMAN
    SpongeMan *sm = &spongeGame->spongeMan;
    char_anim_data *animData = char_anim_data_ptr_hash_mapGetVal(&spongeGame->animations, sm->animState.key);
    char_frame_data *currentFrame = &animData->frames[sm->animState.currentFrame];

    sprite smSprite = defaultSprite();
    smSprite.atlasKey = "sponge_atlas";
    smSprite.frameKey = currentFrame->frameKey;
    ASSERT(smSprite.frameKey != 0);

    if (sm->facing == DIRECTION_RIGHT) {
        mat3x3 posMatrix = mat3x3Translate(sm->pos.x - currentFrame->xOffset, sm->pos.y + currentFrame->yOffset);
        spriteManPushMatrix(posMatrix);
        mat3x3 scaleTransform = mat3x3ScaleXY(-1.0f, 1.0f);
        spriteManPushMatrix(scaleTransform);

        smSprite.pos = (vec2){ .x = 0.0f, .y = 16.0f };
        smSprite.anchor = (vec2){ .x = 0.0f, .y = 1.0f };
        spriteManAddSprite(smSprite);
        //drawHitBoxes(currentFrame, spriteList, assets, (float)-currentFrame->xOffset, (float)-currentFrame->yOffset);
        spriteManPopMatrix();
        spriteManPopMatrix();

    }
    else {
    vec2 smSpriteOrigin = vec2Add(sm->pos, 
                                  (vec2){ .x = currentFrame->xOffset, .y = currentFrame->yOffset + 16.0f });
    smSprite.pos = smSpriteOrigin;
        smSprite.anchor = (vec2){ .x = 0.0f, .y = 1.0f };
        spriteManAddSprite(smSprite);
    }
    //{
    //    sprite s = defaultSprite();
    //    s.pos = spongeGame->spongeMan.pos;
    //    s.atlasKey = "game_atlas";
    //    s.frameKey = "sponge_man";
    //    s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
    //}

}
