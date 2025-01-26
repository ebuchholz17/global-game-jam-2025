#ifndef FIGHTER_LOGIC_H
#define FIGHTER_LOGIC_H

#include "sponge_game.h"

void enterStandingState (Fighter *f, SpongeGameInput *input, f32 dt);
void enterJumpsquatState (Fighter *f, SpongeGameInput *input, f32 dt);
void enterDashStartState (Fighter *f, f32 dt);
void enterJumpingState (Fighter *f, SpongeGameInput *input, f32 dt);
vec2 fighterFeet (Fighter *f);
b32 isTouchingPlatform (Fighter *f, f32 *outPlatY);
void enterFallingState (Fighter *f, f32 dt);
void onFallOffPlatform (Fighter *f, f32 dt);
void enterLandingState (Fighter *f, f32 dt);
void enterRunningState (Fighter *f, f32 dt);
char *attackTypeToAnimStateKey (FighterAttackType type);
void enterAttackAirborneState (Fighter *f, FighterAttackType attackType);
void enterAttackGroundedState (Fighter *f, FighterAttackType attackType);
void enterHitState(Fighter *f, f32 dt) ;
FighterAttackType tryDoAttack (Fighter *f, SpongeGameInput *input, f32 dt);
void updateAttackingGroundedState (Fighter *f, SpongeGameInput *input, f32 dt);
void updateAttackingAirborneState (Fighter *f, SpongeGameInput *input, f32 dt);
void updateJumpingState (Fighter *f, SpongeGameInput *input, f32 dt);
void updateFallingState (Fighter *f, SpongeGameInput *input, f32 dt);
void updateRunningInput (Fighter *f, SpongeGameInput *input, f32 dt);
void updateJumpsquatState (Fighter *f, SpongeGameInput *input, f32 dt);
void updateLandingState (Fighter *f, SpongeGameInput *input, f32 dt);
void updateStandingState (Fighter *f, SpongeGameInput *input, f32 dt);
void onBecomeGrounded (Fighter *f, f32 dt);
void updateDashStartState (Fighter *f, SpongeGameInput *input, f32 dt);
void updateSpongeManState (Fighter *f, SpongeGameInput *input, f32 dt);

f32 getKnockbackAmountForAttack (FighterAttackType attack);
vec2 getKnockbackAngleForAttack (FighterAttackType attack);

#endif
