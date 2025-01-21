#include "sponge_game.h"
#include "../gng_bool.h"
#include "../gng_math.h"
#include "../gng_sprites.h"
#include "../gng_virtual_input.h"

SpongeGame *spongeGame;

void initSpongeGame(SpongeGame *sg, mem_arena *memory) {
    spongeGame = sg;
    SpongeMan *spongeMan = &spongeGame->spongeMan;
    spongeMan->pos = (vec2){ .x = 0.0f, .y = GROUND_Y };
    spongeMan->grounded = true;

    spongeGame->platforms[0] = (SpongePlatform){
        .pos = (vec2){
            .x = 100.0f,
            .y = GROUND_Y - 32.0f
        },
        .length = 48.0f
    };
    spongeGame->platforms[1] = (SpongePlatform){
        .pos = (vec2){
            .x = 50.0f,
            .y = GROUND_Y - 64.0f
        },
        .length = 48.0f
    };
    spongeGame->platforms[2] = (SpongePlatform){
        .pos = (vec2){
            .x = 150.0f,
            .y = GROUND_Y - 64.0f
        },
        .length = 48.0f
    };
    spongeGame->platforms[3] = (SpongePlatform){
        .pos = (vec2){
            .x = 100.0f,
            .y = GROUND_Y - 100.0f
        },
        .length = 48.0f
    };
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

void updateRunningInput (SpongeMan *sm, SpongeGameInput *input, f32 dt) {
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

}

void enterJumpingState (SpongeMan *sm, SpongeGameInput *input, f32 dt) {
    sm->grounded = false;
    sm->jumpTime = 0.0f;
    sm->releasedJump = false;
    sm->vel.y = -SPONGE_JUMP_VEL;
    updateJumpingState(sm, input, dt);
}

b32 isTouchingPlatform (SpongeMan *sm) {
    f32 feetDist = 16.0f;
    vec2 feet = vec2Add(sm->pos, (vec2){ .x = 0.0f, .y = feetDist });
    for (i32 i = 0; i < NUM_SPONGE_PLATFORMS; i++) {
        SpongePlatform *p = &spongeGame->platforms[i];
        if (feet.x >= p->pos.x && feet.x < p->pos.x + p->length &&
            feet.y >= p->pos.y && feet.y < p->pos.y + 8)
        {
            sm->pos.y = p->pos.y - feetDist;
            sm->vel.y = 0.0f;
            sm->grounded = true;
            break;
        }
    }
}

void updateSpongeManState (SpongeMan *sm, SpongeGameInput *input, f32 dt) {

    switch (sm->state) {
        case SM_STATE_STAND: {
            if (input->jump.justPressed) {
                enterJumpingState(sm, input, dt);
                sm->state = SM_STATE_JUMP;
            }
            else if (input->left.justPressed || input->right.justPressed) {
                sm->state = SM_STATE_RUN;
                updateRunningInput(sm, input, dt);
            }
        } break;
        case SM_STATE_RUN: {
            if (input->jump.justPressed) {
                enterJumpingState(sm, input, dt);
                sm->state = SM_STATE_JUMP;
            }
            else {
                updateRunningInput(sm, input, dt);
            }

            if (sm->vel.x == 0.0f) {
                sm->state = SM_STATE_STAND;
            }
        } break;
        case SM_STATE_JUMP: {
            updateJumpingState(sm, input, dt);
            if (sm->grounded) {
                sm->state = SM_STATE_RUN;
            }
        } break;
    }

    f32 iterations = 10;
    f32 dtFrac = dt / (f32)iterations;
    for (i32 i = 0; i < iterations; i++) {
        sm->pos.x += sm->vel.x * dtFrac;
        if (sm->grounded) {
            if (sm->onPlatform && isTouchingPlatform(sm)) {
                sm->onPlatform = false;
                sm->grounded = false;
                // falling
            }
        }
        if (!sm->grounded) {
            sm->vel.y += SPONGE_GRAVITY * dtFrac - sm->fullJumpAntiGrav * dtFrac;
            sm->pos.y += sm->vel.y * dtFrac;
        }

        if (!sm->grounded) {
            if (sm->pos.y > GROUND_Y) {
                sm->pos.y = GROUND_Y;
                sm->grounded = true;
            }
            else if (sm->vel.y > 0) {
                f32 feetDist = 16.0f;
                vec2 feet = vec2Add(sm->pos, (vec2){ .x = 0.0f, .y = feetDist });
                for (i32 i = 0; i < NUM_SPONGE_PLATFORMS; i++) {
                    SpongePlatform *p = &spongeGame->platforms[i];
                    if (feet.x >= p->pos.x && feet.x < p->pos.x + p->length &&
                        feet.y >= p->pos.y && feet.y < p->pos.y + 8)
                    {
                        sm->pos.y = p->pos.y - feetDist;
                        sm->vel.y = 0.0f;
                        sm->grounded = true;
                        sm->onPlatform = true;
                        break;
                    }
                }
            }
        }
    }
}

void updateSpongeGame (SpongeGame *spongeGame, game_input *input, virtual_input *vInput, f32 dt, plat_api platAPI, mem_arena *memory) {
    SpongeGameInput sgInput = parseGameInput(input, vInput);
    updateSpongeManState(&spongeGame->spongeMan, &sgInput, dt);
    spongeGame->isInitialized = true;
}

void drawSpongeGame (SpongeGame *spongeGame, plat_api platAPI) { 
    for (i32 i = 0; i < NUM_SPONGE_PLATFORMS; i++) {
        SpongePlatform *p = &spongeGame->platforms[i];
        {
            sprite s = defaultSprite();
            s.pos = p->pos;
            s.atlasKey = "game_atlas";
            s.frameKey = "platform";
            spriteManAddSprite(s);
        }

    }
    {
        sprite s = defaultSprite();
        s.pos = spongeGame->spongeMan.pos;
        s.atlasKey = "game_atlas";
        s.frameKey = "sponge_man";
        s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(s);
    }

}
