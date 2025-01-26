#include "sponge_game.h"
#include "../gng_bool.h"
#include "../gng_math.h"
#include "../gng_random.h"

#include "fighter_logic.h"
#include "sponge_common.h"

extern SpongeGame *spongeGame;

vec2 getKnockbackAngleForAttack(FighterAttackType attack) {
        switch (attack) {
            case F_ATTACK_JAB: {

        return (vec2){.x = 1.0f, .y =-0.0f };
                               } break;
            case F_ATTACK_LOW_KICK: {

        return (vec2){.x = 0.5f, .y =-0.5f };
                                    } break;
            case F_ATTACK_BUBBLE_ATTACK: {

        return (vec2){.x = 1.0f, .y =-0.0f };
                                         } break;
            case F_ATTACK_SUICIDE_DAIR: {

        return (vec2){.x = 0.5f, .y =-0.5f };
                                         } break;
            case F_ATTACK_UPPERCUT: {

        return (vec2){.x = 0.0f, .y =-1.0f };
                                    } break;
            case F_ATTACK_FP: {

        return (vec2){.x = 0.5f, .y =-0.5f };
                              } break;
            case F_ATTACK_BUBBLE_LAUNCH: {

        return (vec2){.x = 1.0f, .y =-0.0f };
                                         } break;
            case F_ATTACK_SLIDE: {

        return (vec2){.x = 0.5f, .y =-0.5f };
                                 } break;
            case F_ATTACK_DP: {

        return (vec2){.x = 0.0f, .y =-1.0f };
                              } break;

            case F_ATTACK_C_JAB: {

        return (vec2){.x = 1.0f, .y =-0.0f };
                                 } break;
            case F_ATTACK_C_KICK: {
        return (vec2){.x = 0.5f, .y =-0.5f };
                                  } break;
        }
        return (vec2){.x = 0.0f, .y =-0.0f };
}

f32 getKnockbackAmountForAttack(FighterAttackType attack) {
    switch (attack) {
        case F_ATTACK_JAB: {

                               return 100.0f;
                           } break;
        case F_ATTACK_LOW_KICK: {

                                    return 100.0f;
                                } break;
        case F_ATTACK_BUBBLE_ATTACK: {

                                         return 280.0f;
                                     } break;
        case F_ATTACK_SUICIDE_DAIR: {

                                         return 100.0f;
                                     } break;
        case F_ATTACK_UPPERCUT: {

                                    return 400.0f;
                                } break;
        case F_ATTACK_FP: {

                              return 1000.0f;
                          } break;
        case F_ATTACK_BUBBLE_LAUNCH: {

                                         return 500.0f;
                                     } break;
        case F_ATTACK_SLIDE: {

                                 return 300.0f;
                             } break;
        case F_ATTACK_DP: {

                              return 400.0f;
                          } break;

        case F_ATTACK_C_JAB: {

                                 return 200.0f;
                             } break;
        case F_ATTACK_C_KICK: {
                                  return 300.0f;
                              } break;
    }
    return 0.0f;
}

void startFighterAnimState(Fighter *f, FighterAnimState state) {
    char *anim = "sponge_idle";
    switch (f->type) {
        case FIGHTER_TYPE_SPONGE: {
            switch (state) {
                case F_ANIM_STATE_IDLE: {
                   anim = "sponge_idle";
               } break;
                case F_ANIM_STATE_JUMPSQUAT: {
                   anim = "sponge_jump_rising";
                } break;
                case F_ANIM_STATE_RUN: {
                   anim = "sponge_run";
              } break;
                case F_ANIM_STATE_JUMP_RISING: {
                   anim = "sponge_jump_rising";
              } break;
                case F_ANIM_STATE_JUMP_FALLING: {
                   anim = "sponge_jump_falling";
               } break;
                case F_ANIM_STATE_GETUP: {
                   anim = "sponge_getup";
                } break;
                case F_ANIM_STATE_HIT: {
                   anim = "sponge_hit";
                } break;
                case F_ANIM_STATE_DEAD: {
                   anim = "sponge_on_ground";
                } break;
            }
        } break;
        case FIGHTER_TYPE_COCKROACH: {
            switch (state) {
                case F_ANIM_STATE_IDLE: {
                   anim = "cockroach_idle";
               } break;
                case F_ANIM_STATE_JUMPSQUAT: {
                   anim = "cockroach_jumpsquat";
                } break;
                case F_ANIM_STATE_RUN: {
                   anim = "cockroach_walk";
              } break;
                case F_ANIM_STATE_JUMP_RISING: {
                   anim = "cockroach_jump";
              } break;
                case F_ANIM_STATE_JUMP_FALLING: {
                   anim = "cockroach_jump";
               } break;
                case F_ANIM_STATE_GETUP: {
                   anim = "cockroach_getup";
            } break;
                case F_ANIM_STATE_HIT: {
                   anim = "cockroach_hit";
                } break;
                case F_ANIM_STATE_DEAD: {
                   anim = "cockroach_on_ground";
                } break;
            }
        } break;
    }

    f->animState = (AnimationState){ .key = anim };
    startAnimState(&f->animState);
}

void enterStandingState (Fighter *f, SpongeGameInput *input, f32 dt) {
    f->state = F_STATE_STAND;
    startFighterAnimState(f, F_ANIM_STATE_IDLE);
}

void enterDeadState (Fighter *f, f32 dt) {
    f->state = F_STATE_DEAD;
    startFighterAnimState(f, F_ANIM_STATE_DEAD);
}


void enterJumpsquatState (Fighter *f, SpongeGameInput *input, f32 dt) {
    f->stateTimer = 0;
    f->releasedJump = false;
    f->state = F_STATE_JUMPSQUAT;
    startFighterAnimState(f, F_ANIM_STATE_JUMPSQUAT);
}

void enterDashStartState (Fighter *f, f32 dt) {
    f->stateTimer = 0;
    f->state = F_STATE_DASH_START;
    startFighterAnimState(f, F_ANIM_STATE_RUN);

    f32 velSign = f->facing == DIRECTION_LEFT ? -1.0f : 1.0f;
    f->vel.x = velSign * SPONGE_DASH_START_SPD;
}


void enterJumpingState (Fighter *f, SpongeGameInput *input, f32 dt) {
    if (!f->isJumping) {
        f->grounded = false;
        f->jumpTime = 0.0f;
        f->isJumping = true;
        f->releasedJump = false;
        f->vel.y = -SPONGE_JUMP_VEL;
    }
    f->state = F_STATE_JUMP;
    startFighterAnimState(f, F_ANIM_STATE_JUMP_RISING);
}


vec2 fighterFeet (Fighter *f) {
    f32 feetDist = 16.0f;
    vec2 feet = vec2Add(f->pos, (vec2){ .x = 0.0f, .y = feetDist });
    return feet;
}

b32 isTouchingPlatform (Fighter *f, f32 *outPlatY) {
    if (f->passedThroughPlatformTimer > 0.0f) { return false; }

    f32 feetDist = 16.0f;
    vec2 feet = vec2Add(f->pos, (vec2){ .x = 0.0f, .y = feetDist });
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

void enterFallingState (Fighter *f, f32 dt) {
    f->fullJumpAntiGrav = 0.0f;
    f->state = F_STATE_FALL;
    f->isJumping = false;
    f->isShielding = false;
    startFighterAnimState(f, F_ANIM_STATE_JUMP_FALLING);
}

void onFallOffPlatform (Fighter *f, f32 dt) {
    switch (f->state) {
        default: {
            enterFallingState(f, dt);
        } break;
        case F_STATE_HIT: {
            //enterFallingState(f, dt);
        } break;
        case F_STATE_DEAD: {
            //enterFallingState(f, dt);
        } break;
    }
}

void enterLandingState (Fighter *f, f32 dt) {
    f->landingLag = 0.0f;
    if (f->isAttacking) {
        switch (f->attackType) {
            case F_ATTACK_NONE: {
            } break;
            case F_ATTACK_JAB: {
                f->landingLag = 3 * FRAMETIME;
            } break;
            case F_ATTACK_LOW_KICK: {
                f->landingLag = 8 * FRAMETIME;
            } break;
            case F_ATTACK_BUBBLE_ATTACK: {
                f->landingLag = 8 * FRAMETIME;
            } break;
            case F_ATTACK_SUICIDE_DAIR: {
                f->landingLag = 10 * FRAMETIME;
            } break;
            case F_ATTACK_UPPERCUT: {
                f->landingLag = 5 * FRAMETIME;
            } break;
            case F_ATTACK_FP: {
                f->landingLag = 10 * FRAMETIME;
            } break;
            case F_ATTACK_BUBBLE_LAUNCH: {
                f->landingLag = 5 * FRAMETIME;
            } break;
            case F_ATTACK_SLIDE: {
                f->landingLag = 5 * FRAMETIME;
            } break;
            case F_ATTACK_DP: {
                f->landingLag = 5 * FRAMETIME;
            } break;

            case F_ATTACK_C_KICK: {
                f->landingLag = 12 * FRAMETIME;
            } break;
            case F_ATTACK_C_JAB: {
                f->landingLag = 12 * FRAMETIME;
            } break;
        }
    }
    f->isAttacking = false;

    f->stateTimer = 0;
    f->state = F_STATE_LANDING;
    startFighterAnimState(f, F_ANIM_STATE_GETUP);
}


void enterRunningState (Fighter *f, f32 dt) {
    f->stateTimer = 0.0f;
    f->state = F_STATE_DASH;
    startFighterAnimState(f, F_ANIM_STATE_RUN);
}

void enterSuicideDairState (Fighter *f, SpongeGameInput *input, f32 dt) {
    f->stateTimer = 0.0f;
    f->state = F_STATE_SUICIDE_DAIR;
    f->animState = (AnimationState){ .key = "sponge_suicide_dair" };
    f->animState.dontLoop = true;
    startAnimState(&f->animState);
}

void enterShieldState (Fighter *f, SpongeGameInput *input, f32 dt) {
    f->stateTimer = 0.0f;
    f->state = F_STATE_SHIELD;
    f->animState = (AnimationState){ .key = "sponge_shield" };
    startAnimState(&f->animState);
}

void enterShieldStunState (Fighter *f, f32 dt) {
    f->stateTimer = 0.0f;
    f->state = F_STATE_SHIELD_STUN;
    f->animState = (AnimationState){ .key = "sponge_shield" };
    startAnimState(&f->animState);
}

char *attackTypeToAnimStateKey (FighterAttackType type) {
    switch (type) {
        case F_ATTACK_NONE: {
             ASSERT(false);
             return 0;
        } break;
        case F_ATTACK_JAB: {
            return "sponge_jab";
        } break;
        case F_ATTACK_LOW_KICK: {
            return "sponge_slide";
        } break;
        case F_ATTACK_BUBBLE_ATTACK: {
            return "sponge_bubble_attack";
        } break;
        case F_ATTACK_SUICIDE_DAIR: {
            return "sponge_suicide_dair";
        } break;
        case F_ATTACK_UPPERCUT: {
            return "sponge_dp";
        } break;
        case F_ATTACK_FP: {
            return "sponge_jab";
        } break;
        case F_ATTACK_BUBBLE_LAUNCH: {
            return "sponge_bubble_attack";
        } break;
        case F_ATTACK_SLIDE: {
            return "sponge_slide";
        } break;
        case F_ATTACK_DP: {
            return "sponge_dp";
        } break;

        case F_ATTACK_C_JAB: {
            return "cockroach_jab";
        } break;
        case F_ATTACK_C_KICK: {
            return "cockroach_leg_attack";
        } break;
    }
    return 0;
}

void enterAttackAirborneState (Fighter *f, FighterAttackType attackType) {
    f->attackType = attackType;
    f->isAttacking = true;
    f->stateTimer = 0;
    f->state = F_STATE_ATTACKING_AIRBORNE;
    f->animState = (AnimationState){ .key = attackTypeToAnimStateKey(attackType) };
    f->attack = (FighterAttack){
        .active = true,
        .damage = 20,
        .id = randomU32(),
        .knockbackMultiplier = 500
    };
    startAnimState(&f->animState);
}

void enterAttackGroundedState (Fighter *f, FighterAttackType attackType) {
    f->attackType = attackType;
    f->isAttacking = true;
    f->stateTimer = 0;
    f->state = F_STATE_ATTACKING_GROUNDED;
    f->animState = (AnimationState){ .key = attackTypeToAnimStateKey(attackType) };
    f->attack = (FighterAttack){
        .active = true,
        .damage = 20,
        .id = randomU32(),
        .knockbackMultiplier = 500
    };
    startAnimState(&f->animState);
}

void enterHitState(Fighter *f, f32 dt) {
    f->isAttacking = false;
    f->isJumping = false;
    f->stateTimer = 0.0f;
    f->state = F_STATE_HIT;
    f->vel.y = 0.0f;
    startFighterAnimState(f, F_ANIM_STATE_HIT);
}

FighterAttackType tryDoAttack (Fighter *f, SpongeGameInput *input, f32 dt) {
    if (input->attack.justPressed) {
        switch (f->type) {
            case FIGHTER_TYPE_SPONGE: {
                if (input->down.down) {
                    if (!f->grounded) {
                        return F_ATTACK_SUICIDE_DAIR;
                    }
                    else {
                        return F_ATTACK_LOW_KICK;
                    }
                }
                else if ((f->facing == DIRECTION_LEFT && input->left.down) ||
                    (f->facing == DIRECTION_RIGHT && input->right.down)) 
                {
                    // TODO
                    return F_ATTACK_JAB;
                }
                else if (input->up.down) {
                    return F_ATTACK_UPPERCUT;
                }
                else if (!input->up.down && !input->down.down && !input->left.down && !input->right.down) {
                    return F_ATTACK_JAB;
                }
            } break;
            case FIGHTER_TYPE_COCKROACH: {
                if (input->down.down) {
                    return F_ATTACK_C_KICK;
                }
                else if (!input->up.down && !input->down.down && !input->left.down && !input->right.down) {
                    return F_ATTACK_C_JAB;
                }
            } break;
        }
    }
    else if (input->special.justPressed) {
        switch (f->type) {
            case FIGHTER_TYPE_SPONGE: {
                if ((f->facing == DIRECTION_LEFT && input->left.down) ||
                    (f->facing == DIRECTION_RIGHT && input->right.down)) 
                {
                    return F_ATTACK_BUBBLE_LAUNCH;
                }
                else if (input->down.down) {
                    return F_ATTACK_SLIDE;
                }
                else if (input->up.down) {
                    return F_ATTACK_DP;
                }
                else if (!input->up.down && !input->down.down && !input->left.down && !input->right.down) {
                    return F_ATTACK_FP;
                }
            } break;
            case FIGHTER_TYPE_COCKROACH: {
                if (input->down.down) {
                    return F_ATTACK_C_KICK;
                }
                else if (!input->up.down && !input->down.down && !input->left.down && !input->right.down) {
                    return F_ATTACK_C_JAB;
                }
            } break;
        }
    }
    return F_ATTACK_NONE;

}

void updateAttackingGroundedState (Fighter *f, SpongeGameInput *input, f32 dt) {
    if (!f->isAttacking) {
        enterStandingState(f, input, dt);
    }
}

void updateHitState (Fighter *f, SpongeGameInput * input, f32 dt) {
    f->hitstunTimer -= dt;
    if (f->hitstunTimer <= 0.0f) {
        if (f->grounded) {
            enterStandingState(f, input, dt);
        }
        else {
            enterFallingState(f, dt);
        }
    }
}

f32 bounceVel = -200.0f;

void updateSuicideDairState (Fighter *f, SpongeGameInput * input, f32 dt) {
    if (f->facing == DIRECTION_LEFT) {
        f->vel.x = -75.0f;
    }
    else {
        f->vel.x = 75.0f;
    }
    f->vel.y = 250.0f;
    if (f->justHitEnemy) {
        f->vel.y = bounceVel;
        f->isAttacking = false;
        enterFallingState(f, dt);
    }
}

void updateShieldState (Fighter *f, SpongeGameInput * input, f32 dt) {
    f->isShielding = true;
    if (!input->shield.down) {
        f->isShielding = false;
        enterStandingState(f, input, dt);
    }
}

void updateShieldStunState (Fighter *f, SpongeGameInput * input, f32 dt) {
    f->hitstunTimer -= dt;
    if (f->hitstunTimer <= 0.0f) {
        f->isShielding = false;
        enterStandingState(f, input, dt);
    }
}

void updateAttackingAirborneState (Fighter *f, SpongeGameInput *input, f32 dt) {
    if (f->isJumping) {
        f->jumpTime += dt;
        if (!input->jump.down) {
            f->releasedJump = true;
        }

        f->fullJumpAntiGrav = 0.0f;
        if (f->jumpTime < 0.3f && !f->releasedJump) {
            f->fullJumpAntiGrav = SPONGE_GRAVITY * 0.7f;
        }
    }

    if (input->left.down) {
        f->vel.x -= SPONGE_AIR_CONTROL * dt;
    }
    if (input->right.down) {
        f->vel.x += SPONGE_AIR_CONTROL * dt;
    }
    // run max speed = aerial max speed
    if (f->vel.x > SPONGE_MAX_RUN_SPD) {
        f->vel.x = SPONGE_MAX_RUN_SPD;
    }
    if (f->vel.x < -SPONGE_MAX_RUN_SPD) {
        f->vel.x = -SPONGE_MAX_RUN_SPD;
    }

    if (!f->isAttacking) {
        if (f->vel.y > 0) {
            enterFallingState(f, dt);
        }
        else {
            enterJumpingState(f, input, dt);
        }
    }
    if (f->justHitEnemy) {
        f->vel.y = -50.0f;
    }
    if (f->isAttacking && f->attackType == F_ATTACK_SUICIDE_DAIR && f->justHitEnemy) {
        f->vel.y = bounceVel;
        f->isAttacking = false;
        enterFallingState(f, dt);
    }
}

void updateJumpingState (Fighter *f, SpongeGameInput *input, f32 dt) {
    f->jumpTime += dt;
    if (!input->jump.down) {
        f->releasedJump = true;
    }

    f->fullJumpAntiGrav = 0.0f;
    if (f->jumpTime < 0.3f && !f->releasedJump) {
        f->fullJumpAntiGrav = SPONGE_GRAVITY * 0.7f;
    }

    if (input->left.down) {
        f->vel.x -= SPONGE_AIR_CONTROL * dt;
    }
    if (input->right.down) {
        f->vel.x += SPONGE_AIR_CONTROL * dt;
    }
    // run max speed = aerial max speed
    if (f->vel.x > SPONGE_MAX_RUN_SPD) {
        f->vel.x = SPONGE_MAX_RUN_SPD;
    }
    if (f->vel.x < -SPONGE_MAX_RUN_SPD) {
        f->vel.x = -SPONGE_MAX_RUN_SPD;
    }

    FighterAttackType attackType = tryDoAttack(f, input, dt);
    if (attackType != F_ATTACK_NONE) {
        enterAttackAirborneState(f, attackType);
    }
    else if (f->vel.y > 0) {
        enterFallingState(f, dt);
    }
}

void updateFallingState (Fighter *f, SpongeGameInput *input, f32 dt) {
    if (input->left.down) {
        f->vel.x -= SPONGE_AIR_CONTROL * dt;
    }
    if (input->right.down) {
        f->vel.x += SPONGE_AIR_CONTROL * dt;
    }
    // run max speed = aerial max speed
    if (f->vel.x > SPONGE_MAX_RUN_SPD) {
        f->vel.x = SPONGE_MAX_RUN_SPD;
    }
    if (f->vel.x < -SPONGE_MAX_RUN_SPD) {
        f->vel.x = -SPONGE_MAX_RUN_SPD;
    }
    FighterAttackType attackType = tryDoAttack(f, input, dt);
    if (attackType != F_ATTACK_NONE) {
        enterAttackAirborneState(f, attackType);
    }
}

void updateRunningInput (Fighter *f, SpongeGameInput *input, f32 dt) {
    f->stateTimer += dt;
    FighterAttackType attackType = tryDoAttack(f, input, dt);
    if (attackType != F_ATTACK_NONE) {
        enterAttackGroundedState(f, attackType);
    }
    else if (input->jump.justPressed) {
        enterJumpsquatState(f, input, dt);
    }
    else if (input->shield.down) {
        enterShieldState(f, input, dt);
    }
    else if (f->stateTimer < 15.0f * FRAMETIME && f->facing == DIRECTION_LEFT && input->right.down) {
        f->facing = DIRECTION_RIGHT;
        enterDashStartState(f, dt);
    }
    else if (f->stateTimer < 15.0f * FRAMETIME && f->facing == DIRECTION_RIGHT && input->left.down) {
        f->facing = DIRECTION_LEFT;
        enterDashStartState(f, dt);
    }
    else {

        if (input->left.justPressed && f->facing == DIRECTION_RIGHT) {
            f->facing = DIRECTION_LEFT;
            startFighterAnimState(f, F_ANIM_STATE_RUN);
        }
        if (input->right.justPressed && f->facing == DIRECTION_LEFT) {
            f->facing = DIRECTION_RIGHT;
            startFighterAnimState(f, F_ANIM_STATE_RUN);
        }
        if (input->left.down) {
            f->vel.x -= SPONGE_ACCEL_SPD * dt;
        }
        if (input->right.down) {
            f->vel.x += SPONGE_ACCEL_SPD * dt;
        }
        if (!input->left.down && !input->right.down) {
            if (f->vel.x > 0.0f) {
                f->vel.x -= SPONGE_DECEL_SPD * dt;
                if (f->vel.x < 0.0f) {
                    f->vel.x = 0.0f;
                }
            }
            else if (f->vel.x < 0) {
                f->vel.x += SPONGE_DECEL_SPD * dt;
                if (f->vel.x > 0.0f) {
                    f->vel.x = 0.0f;
                }
            }
        }
        if (f->vel.x > SPONGE_MAX_RUN_SPD) {
            f->vel.x = SPONGE_MAX_RUN_SPD;
        }
        if (f->vel.x < -SPONGE_MAX_RUN_SPD) {
            f->vel.x = -SPONGE_MAX_RUN_SPD;
        }
        if (f->vel.x == 0.0f) {
            f->state = F_STATE_STAND;
            startFighterAnimState(f, F_ANIM_STATE_IDLE);
        }
        //if (input->down.down) {
        //    enterStandingState(f, input, dt);
        //}
    }

}

void updateJumpsquatState (Fighter *f, SpongeGameInput *input, f32 dt) {
    if (!input->jump.down) {
        f->releasedJump = true;
    }
    f->stateTimer += dt;
    if (f->stateTimer >= 3.0f * FRAMETIME) {
        enterJumpingState(f, input, dt);
    }
}


void updateLandingState (Fighter *f, SpongeGameInput *input, f32 dt) {
    f->stateTimer += dt;
    if (f->stateTimer >= 3.0f * FRAMETIME + f->landingLag) {
        enterStandingState(f, input, dt);
    }
}


void updateDeadState (Fighter *f, SpongeGameInput *input, f32 dt) {
    f->stateTimer += dt;
}
void updateStandingState (Fighter *f, SpongeGameInput *input, f32 dt) {
    FighterAttackType attackType = tryDoAttack(f, input, dt);
    if (attackType != F_ATTACK_NONE) {
        enterAttackGroundedState(f, attackType);
    }
    else if (input->jump.justPressed) {
        enterJumpsquatState(f, input, dt);
    }
    else if (input->shield.down) {
        enterShieldState(f, input, dt);
    }
    else {
        if (f->onPlatform && input->down.justPressed) {
            f->onPlatform = false;
            f->grounded = false;
            f->passedThroughPlatformTimer = 0.3f;
            onFallOffPlatform(f, dt);
        }
        else if (input->left.down || input->right.down) {
            if (input->left.down) {
                f->facing = DIRECTION_LEFT;
            }
            else if (input->right.down) {
                f->facing = DIRECTION_RIGHT;
            }
            enterRunningState(f, dt);
        }
    }
}

void onBecomeGrounded (Fighter *f, f32 dt) {
    switch (f->state) {
        default: {
            enterLandingState(f, dt);
        } break;
        case F_STATE_HIT: {
            // enter on ground state
        } break;
        case F_STATE_DEAD: {
            //enterFallingState(f, dt);
        } break;
    }
}

void updateDashStartState (Fighter *f, SpongeGameInput *input, f32 dt) {
    f->stateTimer += dt;
    FighterAttackType attackType = tryDoAttack(f, input, dt);
    if (attackType != F_ATTACK_NONE) {
        enterAttackGroundedState(f, attackType);
    }
    else if (input->jump.justPressed) {
        enterJumpsquatState(f, input, dt);
    }
    else if (input->shield.down) {
        enterShieldState(f, input, dt);
    }
    else if (f->facing == DIRECTION_LEFT && input->right.down) {
        f->facing = DIRECTION_RIGHT;
        enterDashStartState(f, dt);
    }
    else if (f->facing == DIRECTION_RIGHT && input->left.down) {
        f->facing = DIRECTION_LEFT;
        enterDashStartState(f, dt);
    }
    else {
        if (f->stateTimer >= 12.0f * FRAMETIME) {
            enterRunningState(f, dt);
        }
    }
}

void updateSpongeManState (Fighter *f, SpongeGameInput *input, f32 dt) {
    zeroMemory((u8 *)&f->hitByInfo, sizeof(FighterHitByInfo));

    if (!f->isAttacking) {
        zeroMemory((u8 *)&f->attack, sizeof(FighterAttack));
    }

    switch (f->state) {
        case F_STATE_STAND: {
            updateStandingState(f, input, dt);
        } break;
        case F_STATE_JUMPSQUAT: {
            updateJumpsquatState(f, input, dt);
        } break;
        case F_STATE_LANDING: {
            updateLandingState(f, input, dt);
        } break;
        case F_STATE_DASH_START: {
            updateDashStartState(f, input, dt);
        } break;
        case F_STATE_DASH: {
            updateRunningInput(f, input, dt);
        } break;
        case F_STATE_JUMP: {
            updateJumpingState(f, input, dt);
            if (f->grounded) {
                f->state = F_STATE_DASH;
                startFighterAnimState(f, F_ANIM_STATE_RUN);
            }
        } break;
        case F_STATE_FALL: {
            updateFallingState(f, input, dt);
        } break;

        case F_STATE_ATTACKING_AIRBORNE: {
            updateAttackingAirborneState(f, input, dt);
        } break;
        case F_STATE_ATTACKING_GROUNDED: {
            updateAttackingGroundedState(f, input, dt);
        } break;
        case F_STATE_HIT: {
            updateHitState(f, input, dt);
        } break;
        case F_STATE_SUICIDE_DAIR: {
            updateSuicideDairState(f, input, dt);
        } break;
        case F_STATE_SHIELD: {
            updateShieldState(f, input, dt);
        } break;
        case F_STATE_SHIELD_STUN: {
            updateShieldStunState(f, input, dt);
        } break;
        case F_STATE_DEAD: {
            updateDeadState(f, input, dt);
        } break;
    }

    f32 iterations = 10;
    f32 dtFrac = dt / (f32)iterations;
    for (i32 i = 0; i < iterations; i++) {
        f32 friction = 500.0;
        if (!f->grounded || 
            (f->state == F_STATE_DASH || 
             f->state == F_STATE_DASH_START)) 
        {
            friction = 0.0;
        }

        f32 prevVelSign = f->vel.x > 0.0 ? 1.0 : -1.0;
        f->vel.x += -prevVelSign * friction * dtFrac;

        f32 velSign = f->vel.x > 0.0 ? 1.0 : -1.0;
        if (prevVelSign != velSign) {
            f->vel.x = 0.0f;
        }
        f->pos.x += f->vel.x * dtFrac;
        if (f->pos.x <= 0.0f) {
            f->pos.x = 0.0f;
        }
        if (f->pos.x > 355.0f) {
            f->pos.x = 355.0f;
        }

        if (f->grounded) {
            f32 platY;
            if (f->onPlatform && !isTouchingPlatform(f, &platY)) {
                f->onPlatform = false;
                f->grounded = false;
                onFallOffPlatform(f, dt);
            }
        }
        if (!f->grounded) {
            f->vel.y += SPONGE_GRAVITY * dtFrac - f->fullJumpAntiGrav * dtFrac;
            f->pos.y += f->vel.y * dtFrac;
        }

        f32 feetDist = 16.0f;
        vec2 feet = vec2Add(f->pos, (vec2){ .x = 0.0f, .y = feetDist });
        if (!f->grounded) {
            if (feet.y > GROUND_Y) {
                f->pos.y = GROUND_Y - feetDist;
                f->vel.y = 0.0f;
                f->grounded = true;
                f->isJumping = false;
                onBecomeGrounded(f, dt);
            }
            else if (f->vel.y > 0) {
                f32 platY;
                if (isTouchingPlatform(f, &platY)) {
                    f->pos.y = platY - feetDist;
                    f->vel.y = 0.0f;
                    f->grounded = true;
                    f->isJumping = false;
                    f->onPlatform = true;
                    onBecomeGrounded(f, dt);
                }
            }
        }
    }

    ASSERT(f->animState.key != 0);
    if (!stringEquals(f->animState.key, f->animState.prevKey)) {
        startAnimState(&f->animState);
    }
    b32 animDone = updateAnimState(&f->animState, dt);

    // state transitions on finished animations
    if (animDone) {
        if (f->isAttacking) {
            if (f->attackType == F_ATTACK_SUICIDE_DAIR && f->state != F_STATE_SUICIDE_DAIR) {
                enterSuicideDairState(f, input, dt);
            }
            else {
                f->isAttacking = false;
            }
        }
    }
    f->passedThroughPlatformTimer -= dt;
    if (f->passedThroughPlatformTimer < 0.0f) {
        f->passedThroughPlatformTimer = 0.0f;
    }

    f->justHitEnemy = false;
}
