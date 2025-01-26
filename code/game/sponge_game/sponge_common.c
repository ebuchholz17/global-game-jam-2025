#include "sponge_common.h"
#include "../gng_types.h"
#include "../gng_bool.h"


extern SpongeGame *spongeGame;

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

    f32 timePerFrame = 1.0f / 40.0f;

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
                if (!animState->dontLoop) {
                    animState->currentFrame = 0;
                }
                else {
                    --animState->currentFrame;
                }
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
