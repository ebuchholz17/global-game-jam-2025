#include "cockroach_ai.h"
#include "../gng_random.h"
#include "../gng_bool.h"

extern SpongeGame *spongeGame;

void pressCockroachButton (Cockroach *c, SpongeGameInputButtons butt) {
    switch (butt) {
        case SG_INPUT_BUTT_UP:{
            if (!c->input.up.down) {
                c->input.up.justPressed = true;
            }
            c->input.up.down = true;
        } break;
        case SG_INPUT_BUTT_DOWN:{
            if (!c->input.down.down) {
                c->input.down.justPressed = true;
            }
            c->input.down.down = true;
        } break;
        case SG_INPUT_BUTT_LEFT:{
            if (!c->input.left.down) {
                c->input.left.justPressed = true;
            }
            c->input.left.down = true;
        } break;
        case SG_INPUT_BUTT_RIGHT:{
            if (!c->input.right.down) {
                c->input.right.justPressed = true;
            }
            c->input.right.down = true;
        } break;
        case SG_INPUT_BUTT_JUMP:{
            if (!c->input.jump.down) {
                c->input.jump.justPressed = true;
            }
            c->input.jump.down = true;
        } break;
        case SG_INPUT_BUTT_ATTACK:{
            if (!c->input.attack.down) {
                c->input.attack.justPressed = true;
            }
            c->input.attack.down = true;
        } break;
        case SG_INPUT_BUTT_SPECIAL:{
            if (!c->input.special.down) {
                c->input.special.justPressed = true;
            }
            c->input.special.down = true;
        } break;
    }
}

void releaseCockroachButton (Cockroach *c, SpongeGameInputButtons butt) {
    switch (butt) {
        case SG_INPUT_BUTT_UP:{
            c->input.up.down = false;
        } break;
        case SG_INPUT_BUTT_DOWN:{
            c->input.down.down = false;
        } break;
        case SG_INPUT_BUTT_LEFT:{
            c->input.left.down = false;
        } break;
        case SG_INPUT_BUTT_RIGHT:{
            c->input.right.down = false;
        } break;
        case SG_INPUT_BUTT_JUMP:{
            c->input.jump.down = false;
        } break;
        case SG_INPUT_BUTT_ATTACK:{
            c->input.attack.down = false;
        } break;
        case SG_INPUT_BUTT_SPECIAL:{
            c->input.special.down = false;
        } break;
    }
}

void releaseAllButtons (Cockroach *c, SpongeGameInputButtons butt) {
    releaseCockroachButton(c, SG_INPUT_BUTT_UP);
    releaseCockroachButton(c, SG_INPUT_BUTT_DOWN);
    releaseCockroachButton(c, SG_INPUT_BUTT_LEFT);
    releaseCockroachButton(c, SG_INPUT_BUTT_RIGHT);
    releaseCockroachButton(c, SG_INPUT_BUTT_JUMP);
    releaseCockroachButton(c, SG_INPUT_BUTT_ATTACK);
    releaseCockroachButton(c, SG_INPUT_BUTT_SPECIAL);
}

void resetCockroachInput (Cockroach *c) {
    c->input.up.justPressed = false;
    c->input.down.justPressed = false;
    c->input.left.justPressed = false;
    c->input.right.justPressed = false;
    c->input.jump.justPressed = false;
    c->input.attack.justPressed = false;
    c->input.special.justPressed = false;
}

void enterShortRangeState (Cockroach *c, f32 dt) {
    c->aiDecision = COCKROACH_AI_DECISION_NONE;
    c->aiState = COCKROACH_AI_STATE_SHORT_RANGE;
}

void enterMedRangeState (Cockroach *c, f32 dt) {
    c->aiDecision = COCKROACH_AI_DECISION_NONE;
    c->aiState = COCKROACH_AI_STATE_MED_RANGE;
}
void enterApproachingState (Cockroach *c, f32 dt) {
    c->aiDecision = COCKROACH_AI_DECISION_NONE;
    c->aiState = COCKROACH_AI_STATE_APPROACHING;
}

void enterWasHitState (Cockroach *c, f32 dt) {
    c->aiDecision = COCKROACH_AI_DECISION_NONE;
    c->aiState = COCKROACH_AI_STATE_WAS_HIT;
}

void enterHitPlayerState (Cockroach *c, f32 dt) {
    c->aiDecision = COCKROACH_AI_DECISION_NONE;
    c->aiState = COCKROACH_AI_STATE_HIT_PLAYER;
}

void enterThinkingState (Cockroach *c, f32 dt) {
    c->aiDecision = COCKROACH_AI_DECISION_NONE;
    c->aiState = COCKROACH_AI_STATE_THINKING;
}

Direction directionToPlayer (Cockroach *c) {
    SpongeMan *sm = &spongeGame->spongeMan;
    if (c->fighter.pos.x < sm->fighter.pos.x) {
        return DIRECTION_RIGHT;
    }
    else {
        return DIRECTION_LEFT;
    }
}

f32 distToPlayer (Cockroach *c) {
    SpongeMan *sm = &spongeGame->spongeMan;
    vec2 distToPlayer = vec2Subtract(sm->fighter.pos, c->fighter.pos);
    f32 distLength = vec2Length(distToPlayer);
    return distLength;
}

void doWalkDecision (Cockroach *c, f32 dt) {
     Direction d = directionToPlayer(c);
     Direction runD = d == DIRECTION_LEFT ? DIRECTION_LEFT : DIRECTION_RIGHT;
     if (runD == DIRECTION_LEFT) {
         releaseCockroachButton(c, SG_INPUT_BUTT_RIGHT);
         pressCockroachButton(c, SG_INPUT_BUTT_LEFT);
     }
     else {
         releaseCockroachButton(c, SG_INPUT_BUTT_LEFT);
         pressCockroachButton(c, SG_INPUT_BUTT_RIGHT);
     }

    f32 distLength = distToPlayer(c);
    if (distLength < c->walkRange) {
        enterThinkingState(c, dt);
    }
}

void doRunAwayDecision (Cockroach *c, f32 dt) {
     Direction d = directionToPlayer(c);
     Direction runD = d == DIRECTION_LEFT ? DIRECTION_RIGHT : DIRECTION_LEFT;
     if (runD == DIRECTION_LEFT) {
         releaseCockroachButton(c, SG_INPUT_BUTT_RIGHT);
         pressCockroachButton(c, SG_INPUT_BUTT_LEFT);
     }
     else {
         releaseCockroachButton(c, SG_INPUT_BUTT_LEFT);
         pressCockroachButton(c, SG_INPUT_BUTT_RIGHT);
     }

     if (c->decisionTimer <= 0.0f) {
         enterThinkingState(c, dt);
     }
}

void doAttackDecision (Cockroach *c, f32 dt) {
   if (!c->fighter.isAttacking) {
       c->didAttack = true;
       c->attackHitPlayer = false;
       pressCockroachButton(c, SG_INPUT_BUTT_ATTACK);
       if (c->aiAttack == F_ATTACK_C_KICK) {
           pressCockroachButton(c, SG_INPUT_BUTT_DOWN);
       }
   }

   if (c->didAttack) {
       if (!c->fighter.isAttacking) {
           if (c->attackHitPlayer) {
               enterHitPlayerState(c, dt);
           }
           else {
               enterThinkingState(c, dt);
           }
       }
   }
}

void updateThinkingState (Cockroach *c, f32 dt) {
    if (c->fighter.pos.y > spongeGame->spongeMan.fighter.pos.y) {
        enterApproachingState(c, dt);
    }
    else if (c->fighter.pos.y < spongeGame->spongeMan.fighter.pos.y) {
        enterApproachingState(c, dt);
    }
    else {
        f32 distLength = distToPlayer(c);
        if (distLength <= 30.0f) {
            enterShortRangeState(c, dt);
        }
        else if (distLength <= 75.0f) {
            enterMedRangeState(c, dt);
        }
        else {
            enterApproachingState(c, dt);
        }
    }
}


void updateShortRangeState (Cockroach *c, f32 dt) {
    if (c->aiDecision == COCKROACH_AI_DECISION_NONE) {
        c->decisionTimer = 0.0f;
        f32 random = randomF32();

        releaseAllButtons(c, dt);

        if (random < 0.75f) {
            c->aiDecision = COCKROACH_AI_DECISION_ATTACK;
            c->aiAttack = F_ATTACK_C_JAB;
        }
        //else if (random < 0.9f) {
        // dodge
        //}
        else if (random < 0.95f) {
            c->aiDecision = COCKROACH_AI_DECISION_RUN_AWAY;
            c->decisionTimer = 0.5f + randomF32() * 0.75f;
        }
        else {
            c->aiDecision = COCKROACH_AI_DECISION_WAIT;
            c->decisionTimer = randomF32() * 0.4f;
        }
    }

    c->decisionTimer -= dt;

    switch (c->aiDecision) {
        case COCKROACH_AI_DECISION_ATTACK: {
           doAttackDecision(c, dt);
       } break;
        case COCKROACH_AI_DECISION_RUN_AWAY: { 
             doRunAwayDecision(c, dt);
       } break;
        case COCKROACH_AI_DECISION_WAIT: { 
             if (c->decisionTimer <= 0.0f) {
                 enterThinkingState(c, dt);
             }
       } break;
    }
}

void updateMedRangeState (Cockroach *c, f32 dt) {
    if (c->aiDecision == COCKROACH_AI_DECISION_NONE) {
        c->decisionTimer = 0.0f;
        f32 random = randomF32();

        releaseAllButtons(c, dt);

        if (random < 0.4f) {
            c->aiDecision = COCKROACH_AI_DECISION_ATTACK;
            c->aiAttack = F_ATTACK_C_KICK;
        }
        else if (random < 0.7f) {
            c->aiDecision = COCKROACH_AI_DECISION_WALK;
            c->walkRange = 30.0f;
        }
        //else if (random < 0.9f) {
        // dodge
        //}
        //else if (random < 0.9f) {
        // dash dance
        //}
        else if (random < 0.95f) {
            c->aiDecision = COCKROACH_AI_DECISION_RUN_AWAY;
            c->decisionTimer = 0.1f + randomF32() * 0.5f;
        }
        else {
            c->aiDecision = COCKROACH_AI_DECISION_WAIT;
            c->decisionTimer = randomF32() * 0.4f;
        }
    }

    c->decisionTimer -= dt;

    switch (c->aiDecision) {
        case COCKROACH_AI_DECISION_ATTACK: {
           doAttackDecision(c, dt);
       } break;
        case COCKROACH_AI_DECISION_WALK: {
             doWalkDecision(c, dt);
       } break;
        case COCKROACH_AI_DECISION_RUN_AWAY: { 
             doRunAwayDecision(c, dt);
       } break;
        case COCKROACH_AI_DECISION_WAIT: { 
             if (c->decisionTimer <= 0.0f) {
                 enterThinkingState(c, dt);
             }
       } break;
    }
}

void updateApproachingState (Cockroach *c, f32 dt) {
    if (c->aiDecision == COCKROACH_AI_DECISION_NONE) {
        c->decisionTimer = 0.0f;
        c->shortHop = false;
        f32 random = randomF32();

        releaseAllButtons(c, dt);

        if (c->fighter.pos.y < spongeGame->spongeMan.fighter.pos.y && 
            c->fighter.onPlatform) 
        {
            pressCockroachButton(c, SG_INPUT_BUTT_DOWN);
            enterApproachingState(c, dt);
        }
        else if (c->fighter.pos.y > spongeGame->spongeMan.fighter.pos.y) {
            c->aiDecision = COCKROACH_AI_DECISION_JUMP;
            if (randomF32() < 0.25f) {
                c->shortHop = true;
            }
        }
        else {
            if (random < 0.75f) {
                c->aiDecision = COCKROACH_AI_DECISION_WALK;
                c->walkRange = 70.0f;
            }
            //else if (random < 0.9f) {
            // dodge
            //}
            else if (random < 0.95f) {
                c->aiDecision = COCKROACH_AI_DECISION_JUMP;
                if (randomF32() < 0.5f) {
                    c->shortHop = true;
                }
            }
            else {
                c->aiDecision = COCKROACH_AI_DECISION_WAIT;
                c->decisionTimer = randomF32() * 0.4f;
            }
        }
    }

    c->decisionTimer -= dt;

    switch (c->aiDecision) {
        case COCKROACH_AI_DECISION_WALK: { 
             doWalkDecision(c, dt);
       } break;
        case COCKROACH_AI_DECISION_JUMP: { 
             Direction d = directionToPlayer(c);
             Direction runD = d == DIRECTION_LEFT ? DIRECTION_LEFT : DIRECTION_RIGHT;
             if (runD == DIRECTION_LEFT) {
                 releaseCockroachButton(c, SG_INPUT_BUTT_RIGHT);
                 pressCockroachButton(c, SG_INPUT_BUTT_LEFT);
             }
             else {
                 releaseCockroachButton(c, SG_INPUT_BUTT_LEFT);
                 pressCockroachButton(c, SG_INPUT_BUTT_RIGHT);
             }

             if (!c->didJump) {
                 c->didJump = true;
                 pressCockroachButton(c,SG_INPUT_BUTT_JUMP);
             }
             else {
                 if (c->shortHop) {

                 }
                 else {
                     pressCockroachButton(c,SG_INPUT_BUTT_JUMP);
                 }
             }

            if (c->fighter.grounded) {
                enterThinkingState(c, dt);
            }
       } break;
        case COCKROACH_AI_DECISION_WAIT: { 
             if (c->decisionTimer <= 0.0f) {
                 enterThinkingState(c, dt);
             }
       } break;
    }
}

void updateWasHitState (Cockroach *c, f32 dt) {

}

void updateHitPlayerState (Cockroach *c, f32 dt) {

}

SpongeGameInput doCockroachAI (Cockroach *c, f32 dt) {
    switch (c->aiState) {
        case COCKROACH_AI_STATE_THINKING: {
            updateThinkingState(c, dt);
        } break;
        case COCKROACH_AI_STATE_SHORT_RANGE: {
            updateShortRangeState(c, dt);
        } break;
        case COCKROACH_AI_STATE_MED_RANGE: {
            updateMedRangeState(c, dt);
        } break;
        case COCKROACH_AI_STATE_APPROACHING: {
            updateApproachingState(c, dt);
        } break;
        case COCKROACH_AI_STATE_WAS_HIT: {
            updateWasHitState(c, dt);
        } break;
        case COCKROACH_AI_STATE_HIT_PLAYER: {
            updateHitPlayerState(c, dt);
        } break;
    }

    c->globalTimer += dt;
    if (c->globalTimer > 3.0f) {
        enterThinkingState(c, dt);
        c->globalTimer = 0.0f;
    }

    return c->input;
}
