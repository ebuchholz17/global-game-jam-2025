#include "sponge_game.h"
#include "../gng_bool.h"
#include "../gng_math.h"
#include "../gng_sprites.h"
#include "../gng_virtual_input.h"
#include "../gng_string.h"
#include "../hitbox/hitbox.h"

#include "sponge_common.c"
#include "fighter_logic.c"
#include "cockroach_ai.c"

SpongeGame *spongeGame;
extern sprite_man *spriteMan;

void loadCockroachHitboxData (char *key, mem_arena *memory) {
    data_asset *hitboxData = getDataAsset(key);

    char_anim_data *animData = (char_anim_data *)allocMemory(memory, sizeof(char_anim_data));
    animData->key = key;
    loadHitboxData(assetMan, "cockroach_atlas", hitboxData->data, animData);

    char_anim_data_ptr_hash_mapStore(&spongeGame->animations, animData, key);
}

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
            result.attack = input->sKey;
            result.shield = input->xKey;
        } break;
        case INPUT_SOURCE_VIRTUAL: {
            result.up = vInput->dPadUp.button;;
            result.down = vInput->dPadDown.button;
            result.left = vInput->dPadLeft.button;
            result.right = vInput->dPadRight.button;
            result.jump = vInput->bottomButton.button;
            result.attack = vInput->leftButton.button;
            result.shield = vInput->rightButton.button;
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
                    result.attack = cont->xButton;
                    result.shield = cont->bButton;
                    break;
                }
            }
        } break;
    }

    return result;
}

void spawnCockroach (void) {
    if (spongeGame->cockroaches.numValues < spongeGame->cockroaches.capacity) {
        Cockroach_listPush(&spongeGame->cockroaches, (Cockroach){
            .fighter = (Fighter){
                .type = FIGHTER_TYPE_COCKROACH,
                .pos = (vec2){
                    .x = randomF32() * 356.0f,
                    .y = -32.0f
                },
                .hitPoints = 200
            }
        });
        Cockroach *c = &spongeGame->cockroaches.values[spongeGame->cockroaches.numValues - 1];
        startFighterAnimState(&c->fighter, F_ANIM_STATE_IDLE);
    }
}

void spawnSudEmitter (SudEmitter s) {
    if (spongeGame->sudEmitters.numValues < spongeGame->sudEmitters.capacity) {
        SudEmitter_listPush(&spongeGame->sudEmitters, s);
    }
}

void spawnSud (vec2 pos, f32 vel, f32 angle, b32 flyToSponge) {
    for (i32 i = 0; i < MAX_NUM_SUDS;i++) {
        Sud *s = &spongeGame->suds[i];
        if (!s->active) {
            s->active = true;
            s->pos = pos;
            s->vel = (vec2){ .x = vel * cos2PI(angle), .y = vel * sin2PI(angle)};
            s->flyToSponge = flyToSponge;
            break;
        }
    }
}

void startGame (mem_arena *memory) {
    zeroMemory((u8 *)&spongeGame->spongeMan, sizeof(SpongeMan));
    SpongeMan *spongeMan = &spongeGame->spongeMan;
    spongeMan->fighter.type = FIGHTER_TYPE_SPONGE;
    spongeMan->fighter.pos = (vec2){ .x = 160.0f, .y = GROUND_Y - 16.0f };
    spongeMan->fighter.grounded = true;
    spongeMan->fighter.facing = DIRECTION_LEFT;

    startFighterAnimState(&spongeGame->spongeMan.fighter, F_ANIM_STATE_IDLE);
    spongeGame->spongeMan.fighter.hitPoints = 300;

    spongeGame->cockroachTimer = 5.0f;
    spongeGame->cockroaches.numValues = 0;
    spongeGame->sudEmitters.numValues = 0;
    zeroMemory((u8 *)&spongeGame->suds, sizeof(Sud) * MAX_NUM_SUDS);
    spongeGame->numSuds = 0;
    spongeGame->lastHitEnemy = 0;

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
}

void initSpongeGame(SpongeGame *sg, mem_arena *memory) {
    spongeGame = sg;
    zeroMemory((u8 *)spongeGame, sizeof(SpongeGame));

    spongeGame->animations = char_anim_data_ptr_hash_mapInit(memory, 200);
    loadCockroachHitboxData("cockroach_dodge", memory);
    loadCockroachHitboxData("cockroach_getup", memory);
    loadCockroachHitboxData("cockroach_hit", memory);
    loadCockroachHitboxData("cockroach_idle", memory);
    loadCockroachHitboxData("cockroach_jab", memory);
    loadCockroachHitboxData("cockroach_jump", memory);
    loadCockroachHitboxData("cockroach_jumpsquat", memory);
    loadCockroachHitboxData("cockroach_leg_attack", memory);
    loadCockroachHitboxData("cockroach_on_ground", memory);
    loadCockroachHitboxData("cockroach_walk", memory);
    loadSpongeHitboxData("sponge_bubble_attack", memory);
    loadSpongeHitboxData("sponge_dodge", memory);
    loadSpongeHitboxData("sponge_dp", memory);
    loadSpongeHitboxData("sponge_fp", memory);
    loadSpongeHitboxData("sponge_getup", memory);
    loadSpongeHitboxData("sponge_hit", memory);
    loadSpongeHitboxData("sponge_idle", memory);
    loadSpongeHitboxData("sponge_jab", memory);
    loadSpongeHitboxData("sponge_jump_falling", memory);
    loadSpongeHitboxData("sponge_jump_rising", memory);
    loadSpongeHitboxData("sponge_jumpsquat", memory);
    loadSpongeHitboxData("sponge_on_ground", memory);
    loadSpongeHitboxData("sponge_run", memory);
    loadSpongeHitboxData("sponge_shield", memory);
    loadSpongeHitboxData("sponge_slide", memory);
    loadSpongeHitboxData("sponge_slide_startup", memory);
    loadSpongeHitboxData("sponge_special_jab", memory);
    loadSpongeHitboxData("sponge_suicide_dair", memory);

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


    spongeGame->cockroaches = Cockroach_listInit(memory, MAX_NUM_COCKROACHES);

    spongeGame->sudEmitters = SudEmitter_listInit(memory, 5);
    //spawnSudEmitter((SudEmitter){
    //    .pos = (vec2){.x = 64.0f, .y = 40.0f },
    //    .minVel = 200.0f,
    //    .maxVel = 300.0f,
    //    .rate = 0.01f,
    //    .duration = 5.0f,
    //    .minAngle = .60f,
    //    .maxAngle = 0.85f
    //});
    spongeGame->isInitialized = true;

    startGame(memory);
}

b32 testFighterAttackHitFighter (Fighter *enemy, char_frame_data *attackerFrame, mat3x3 *attackerTransform) 
{
    AnimationState *enemyAnimState = &enemy->animState;
    char_anim_data *enemyAnimData = char_anim_data_ptr_hash_mapGetVal(&spongeGame->animations, enemyAnimState->key);
    char_frame_data *enemyCurrentFrame = &enemyAnimData->frames[enemyAnimState->currentFrame];

    b32 flip = false;
    if (enemy->type == FIGHTER_TYPE_SPONGE) {
        flip = enemy->facing == DIRECTION_RIGHT;
    }
    else if (enemy->type == FIGHTER_TYPE_COCKROACH) {
        flip = enemy->facing == DIRECTION_LEFT;
    }

    mat3x3 enemyTransform;
    if (flip) {
        mat3x3 translation = mat3x3Translate(enemy->pos.x, enemy->pos.y);
        mat3x3 scaling = mat3x3ScaleXY(-1.0f, 1.0f); 
        mat3x3 translation2 = mat3x3Translate((float)-enemyCurrentFrame->xOffset, (float)-enemyCurrentFrame->yOffset);
        enemyTransform = mat3x3MatrixMul(translation, scaling);
    }
    else {
        enemyTransform = mat3x3Translate(enemy->pos.x, enemy->pos.y);
    }

    b32 attackerHitEnemy = false;
    for (int hitboxIndex = 0; hitboxIndex < attackerFrame->numHitboxes; ++hitboxIndex) {
        rect hitbox = attackerFrame->hitboxes[hitboxIndex];
        vec2 hitMin = vec2Mat3x3Mul(*attackerTransform, hitbox.min);
        vec2 hitMax = vec2Mat3x3Mul(*attackerTransform, hitbox.max);
        rect transformedHitbox = (rect){ .min = hitMin, .max = hitMax };

        rect newHitbox;
        newHitbox.min.x = hitMin.x < hitMax.x ? hitMin.x : hitMax.x;
        newHitbox.max.x = hitMin.x > hitMax.x ? hitMin.x : hitMax.x;
        newHitbox.min.y = hitMin.y < hitMax.y ? hitMin.y : hitMax.y;
        newHitbox.max.y = hitMin.y > hitMax.y ? hitMin.y : hitMax.y;


        for (int hurtboxIndex = 0; hurtboxIndex < enemyCurrentFrame->numHurtboxes; ++hurtboxIndex) {
            rect hurtbox = enemyCurrentFrame->hurtboxes[hurtboxIndex];

            vec2 hurtMin = vec2Mat3x3Mul(enemyTransform, hurtbox.min);
            vec2 hurtMax = vec2Mat3x3Mul(enemyTransform, hurtbox.max);
            rect transformedHurtbox = (rect){ .min = hurtMin, .max = hurtMax };

            rect newHurtbox;
            newHurtbox.min.x = hurtMin.x < hurtMax.x ? hurtMin.x : hurtMax.x;
            newHurtbox.max.x = hurtMin.x > hurtMax.x ? hurtMin.x : hurtMax.x;
            newHurtbox.min.y = hurtMin.y < hurtMax.y ? hurtMin.y : hurtMax.y;
            newHurtbox.max.y = hurtMin.y > hurtMax.y ? hurtMin.y : hurtMax.y;

            if (rectsIntersect(newHitbox, newHurtbox)) {
                return true;
            }
        }
    }
    return false;
}

void handleFighterAttackingFighter (Fighter *a, Fighter *b) {
    AnimationState *playerAnimState = &a->animState;
    char_anim_data *playerAnimData = char_anim_data_ptr_hash_mapGetVal(&spongeGame->animations, playerAnimState->key);
    char_frame_data *playerCurrentFrame = &playerAnimData->frames[playerAnimState->currentFrame];

    b32 flip = false;
    if (a->type == FIGHTER_TYPE_SPONGE) {
        flip = a->facing == DIRECTION_RIGHT;
    }
    else if (a->type == FIGHTER_TYPE_COCKROACH) {
        flip = a->facing == DIRECTION_LEFT;
    }

    mat3x3 playerTransform;
    if (flip) {
        mat3x3 translation = mat3x3Translate(a->pos.x, a->pos.y);
        mat3x3 scaling = mat3x3ScaleXY(-1.0f, 1.0f); 
        mat3x3 translation2 = mat3x3Translate((float)-playerCurrentFrame->xOffset, (float)-playerCurrentFrame->yOffset);
        playerTransform = mat3x3MatrixMul(translation, scaling);
    }
    else {
        playerTransform = mat3x3Translate(a->pos.x, a->pos.y);
    }

    if (b == a || b->iframesTimer > 0.0f || b->defeatedTimer > 0.0f || b->fadingTimer > 0.0f) {
        return;
    }

    if (b->lastAttackHitByID == a->attack.id) {
        return;
    }

    b32 hit = testFighterAttackHitFighter(b, playerCurrentFrame, &playerTransform);

    if (hit) {
        if (b->dodging) {
            b->lastAttackHitByID = a->attack.id;
        }
        else {
            a->justHitEnemy = true;
            if (a->type == FIGHTER_TYPE_SPONGE) {
                i32 t = 2;
            }
            b->hitByInfo = (FighterHitByInfo){
                .wasHit = true,
                .attackID = a->attack.id,
                .attackOrigin = a->pos,
                .damage = a->attack.damage,
                .knockbackSpeed = getKnockbackAmountForAttack(a->attackType),
                .attackType = a->attackType
            };
            if (b->type == FIGHTER_TYPE_COCKROACH) {

            spongeGame->lastHitEnemy = b;
            }
        }
    }
}

void handleFighterHit (Fighter *f, f32 dt) {
    // update
    if (f->hitByInfo.wasHit) {
        //soundManPlaySound("impact");
        f->lastAttackHitByID = f->hitByInfo.attackID;

        f32 shieldStunFactor = 1.0f;
        if (f->isShielding) {
            shieldStunFactor = 0.3f;
            enterShieldStunState(f, dt);
            f->hitstunTimer = 0.2f;
            f->hitPoints -= f->hitByInfo.damage * 0.1f;
        }
        else {
            enterHitState(f, dt);
            f->hitstunTimer = 0.5f;
            f->hitPoints -= f->hitByInfo.damage;
        }

        vec2 knockBackDir = getKnockbackAngleForAttack(f->hitByInfo.attackType);
        if (f->hitByInfo.attackOrigin.x > f->pos.x) {
            knockBackDir.x = -knockBackDir.x;
        }
        knockBackDir = vec2Normalize(knockBackDir);

        //f->vel = (vec2){0};
        f->vel = vec2ScalarMul(f->hitByInfo.knockbackSpeed * shieldStunFactor, knockBackDir);
        if (f->vel.y < 0.0f) {
            f->grounded = false;
        }
        f->knockbackTimer = 0.05f;

        //f->iframesTimer = 0.1f;
        if (f->hitPoints <= 0.0f) {
            enterDeadState(f, dt);
            //f->state = CARDMAN_STATE_DEFEATED;
            //setAnimState(c, getCardmanHitstunAnim(c));

            //if (f == spongeGame->lastHitEnemy) {
            //    spongeGame->lastHitEnemy = 0;
            //}
        }
        else {
        }
    }
}

void updateSpongeGame (SpongeGame *spongeGame, game_input *input, virtual_input *vInput, f32 dt, plat_api platAPI, mem_arena *memory) {
    if (spongeGame->spongeMan.fighter.state == F_STATE_DEAD && spongeGame->spongeMan.fighter.stateTimer > 4.0f) {
        startGame(memory);
    }

    SpongeGameInput sgInput = parseGameInput(input, vInput);
    updateSpongeManState(&spongeGame->spongeMan.fighter, &sgInput, dt);

    for (i32 i = spongeGame->cockroaches.numValues - 1; i >= 0; i--) {
        Cockroach *c = &spongeGame->cockroaches.values[i];
        if (c->fighter.state == F_STATE_DEAD && c->fighter.stateTimer > 2.0f) {
            Cockroach_listSplice(&spongeGame->cockroaches, i);
        }
    }

    spongeGame->cockroachTimer -= dt;
    if (spongeGame->cockroachTimer <= 0.0f) {
        spongeGame->cockroachTimer += 30.0f;
        spawnCockroach();
    }

    for (i32 i = 0; i < spongeGame->cockroaches.numValues; i++) {
        Cockroach *c = &spongeGame->cockroaches.values[i];
        // update
        SpongeGameInput input = doCockroachAI(c, dt);
        resetCockroachInput(c);
        updateSpongeManState(&c->fighter, &input, dt);
    }

    // Handle attacksbu 
    for (i32 i = 0; i < spongeGame->cockroaches.numValues; i++) {
        Cockroach *c = &spongeGame->cockroaches.values[i];
        // update
        if (spongeGame->spongeMan.fighter.isAttacking) {
            handleFighterAttackingFighter(&spongeGame->spongeMan.fighter, &c->fighter);
        }
        if (c->fighter.isAttacking) {
            handleFighterAttackingFighter(&c->fighter, &spongeGame->spongeMan.fighter);
        }
    }

    handleFighterHit(&spongeGame->spongeMan.fighter, dt);
    for (i32 i = 0; i < spongeGame->cockroaches.numValues; i++) {
        Cockroach *c = &spongeGame->cockroaches.values[i];
        Fighter *f = &c->fighter;
        handleFighterHit(f, dt);
    }

    if (spongeGame->spongeMan.fighter.isAttacking) {
        SpongeMan* sm = &spongeGame->spongeMan;
        Fighter* f = &sm->fighter;
        AnimationState *spongeAnimState = &f->animState;
        char_anim_data *spongeAnimData = char_anim_data_ptr_hash_mapGetVal(&spongeGame->animations, spongeAnimState->key);
        char_frame_data *spongeCurrentFrame = &spongeAnimData->frames[spongeAnimState->currentFrame];

        b32 flip = false;
        flip = f->facing == DIRECTION_RIGHT;

        mat3x3 spongeTransform;
        if (flip) {
            mat3x3 translation = mat3x3Translate(f->pos.x, f->pos.y);
            mat3x3 scaling = mat3x3ScaleXY(-1.0f, 1.0f); 
            mat3x3 translation2 = mat3x3Translate((float)-spongeCurrentFrame->xOffset, (float)-spongeCurrentFrame->yOffset);
            spongeTransform = mat3x3MatrixMul(translation, scaling);
        }
        else {
            spongeTransform = mat3x3Translate(f->pos.x, f->pos.y);
        }

        b32 attackerHitsponge = false;
        if (spongeCurrentFrame->numHitboxes > 0) {
            rect hitbox = spongeCurrentFrame->hitboxes[0];
            vec2 hitMin = vec2Mat3x3Mul(spongeTransform, hitbox.min);
            vec2 hitMax = vec2Mat3x3Mul(spongeTransform, hitbox.max);
            rect transformedHitbox = (rect){ .min = hitMin, .max = hitMax };

            rect newHitbox;
            newHitbox.min.x = hitMin.x < hitMax.x ? hitMin.x : hitMax.x;
            newHitbox.max.x = hitMin.x > hitMax.x ? hitMin.x : hitMax.x;
            newHitbox.min.y = hitMin.y < hitMax.y ? hitMin.y : hitMax.y;
            newHitbox.max.y = hitMin.y > hitMax.y ? hitMin.y : hitMax.y;
            f32 centerX = hitMin.x + 0.5 * (hitMax.x - hitMin.x);
            f32 centerY = hitMin.y + 0.5 * (hitMax.y - hitMin.y);

            spawnSudEmitter((SudEmitter){
                .pos = (vec2){.x = centerX, .y = centerY + 16.0f},
                .minVel = 80.0f,
                .maxVel = 100.0f,
                .rate = 0.1f,
                .duration = 0.2f,
                .minAngle = .00f,
                .maxAngle = 1.0f
            });
        }
    }

    if (spongeGame->spongeMan.fighter.isShielding) {
        for (i32 i = 0; i < NUM_TILE_ROWS; i++) {
            for (i32 j = 0; j < NUM_TILE_COLS; j++) {
                TileCoating *c = &spongeGame->levelTileCoatings[i * NUM_TILE_COLS + j];
                if (c->isGround && c->type == TILE_COATING_TYPE_SUDS && c->amount > 0) {
                    vec2 tileCenter = {
                        .x = 8.0f * j + 4.0f,
                        .y = 8.0f * i + 4.0f
                    };
                    f32 length = vec2Length((vec2Subtract(spongeGame->spongeMan.fighter.pos, tileCenter)));
                    if (length < 100.0f) {
                        f32 speed = (8.0f / length);
                        if (speed > 1.0f) {
                            speed = 1.0f;
                        }
                        c->sudTime += dt * speed;
                        while (c->sudTime >= 0.1f) {
                            c->sudTime -= 0.1f;
                            c->amount--;
                            if (c->amount == 0) {
                                c->type = TILE_COATING_TYPE_NONE;
                                break;
                            }
                            spawnSud(tileCenter, 0.0f, 0.0f, true);
                        }
                    }
                }
            }
        }

    }

    for (i32 i = spongeGame->sudEmitters.numValues - 1; i >= 0; i--) {
        SudEmitter *e = &spongeGame->sudEmitters.values[i];
        e->t += dt;
        while (e->t >= e->rate) {
            f32 angle = e->minAngle + randomF32() * (e->maxAngle - e->minAngle);
            f32 vel = e->minVel + randomF32() * (e->maxVel - e->minVel);
            spawnSud(e->pos, vel, angle, false);
            e->t -= e->rate;
        }
        e->timeAlive += dt;
        if (e->timeAlive >= e->duration) {
            SudEmitter_listSplice(&spongeGame->sudEmitters, i);
        }
    }

    for (i32 i = 0; i < MAX_NUM_SUDS; i++) {
        Sud *sud = &spongeGame->suds[i];
        if (sud->active) {
            if (sud->flyToSponge) {
                vec2 dir = vec2Subtract(spongeGame->spongeMan.fighter.pos, sud->pos);
                if (vec2Length(dir) < 5.0f) {
                    sud->active = false;
                    spongeGame->spongeMan.fighter.hitPoints++;
                }
                else {
                dir = vec2Normalize(dir);
                sud->pos.x += dir.x * 100.0f * dt;
                sud->pos.y += dir.y * 100.0f * dt;
                }
            }
            else {
                sud->vel.y += SPONGE_GRAVITY * dt;
                sud->pos.x += sud->vel.x * dt;
                sud->pos.y += sud->vel.y * dt;

                i32 sudX = sud->pos.x / 8.0f;
                i32 sudY = sud->pos.y / 8.0f;
                if (sudX >= 0 && sudX < NUM_TILE_COLS && sudY >= 0 && sudY < NUM_TILE_ROWS) {
                    TileCoating *c = &spongeGame->levelTileCoatings[sudY * NUM_TILE_COLS + sudX];
                    if (c->isGround) {
                        if (c->type == TILE_COATING_TYPE_GRIME) {
                            c->amount--;
                            if (c->amount <= 0) {
                                c->type = TILE_COATING_TYPE_NONE;
                            }
                        }
                        else if (c->type == TILE_COATING_TYPE_NONE) {
                            c->type = TILE_COATING_TYPE_SUDS;
                            c->amount = 1;
                        }
                        else {
                            c->amount++;
                        }
                        sud->active = false;
                    }
                }
                else {
                    sud->active = false;
                }
            }
        }
    }

    for (i32 i = 0; i < spongeGame->cockroaches.numValues; i++) {
        Cockroach *c = &spongeGame->cockroaches.values[i];

        vec2 feet = fighterFeet(&c->fighter);
        i32 feetX = feet.x / 8.0f;
        i32 feetY = feet.y / 8.0f;
        if (feetX >= 0 && feetX < NUM_TILE_COLS && feet.y >= 0 && feetY < NUM_TILE_ROWS) {
            TileCoating *c = &spongeGame->levelTileCoatings[feetY * NUM_TILE_COLS + feetX];
            if (c->isGround) {
                c->grimeTime += dt;
                while (c->grimeTime >= 0.1f) {
                    c->grimeTime -= 0.1f;
                    if (c->type == TILE_COATING_TYPE_SUDS) {
                        c->amount--;
                        if (c->amount <= 0) {
                            c->type = TILE_COATING_TYPE_NONE;
                        }
                    }
                    else if (c->type == TILE_COATING_TYPE_NONE) {
                        c->type = TILE_COATING_TYPE_GRIME;
                        c->amount = 1;
                    }
                    else {
                        c->amount++;
                    }
                }
            }
        }
    }

}

void drawBox (vec2 origin, rect box, char *boxKey) {
    spriteManPushTransform((sprite_transform){
        .pos = (vec2) {.x = origin.x, .y = origin.y + 16.0f},
        .scale = 1.0f
    });
    spriteManPushTransform((sprite_transform){
        .pos = box.min,
        .scale = 1.0f
    });
    vec2 rectDims = vec2Subtract(box.max, box.min);
    vec2 rectScale = vec2ScalarMul(1.0f / 10.0f, rectDims);
    mat3x3 boxTransform = mat3x3ScaleXY(rectScale.x, rectScale.y);

    spriteManPushMatrix(boxTransform);

    sprite boxSprite = defaultSprite();
    boxSprite.atlasKey = "game_atlas";
    boxSprite.frameKey = boxKey;
    boxSprite.alpha = 0.3f;
    spriteManAddSprite(boxSprite);

    spriteManPopMatrix();
    spriteManPopMatrix();
    spriteManPopMatrix();
}

void drawHitBoxes (char_frame_data *currentFrame, vec2 origin) {
    for (int i = 0; i < currentFrame->numHitboxes; ++i) {
        rect box = currentFrame->hitboxes[i];
        char *boxKey = "hitbox_frame_data";
        drawBox(origin, box, boxKey);
    }
    for (int i = 0; i < currentFrame->numHurtboxes; ++i) {
        rect box = currentFrame->hurtboxes[i];
        char *boxKey = "hurtbox_frame_data";
        drawBox(origin, box, boxKey);
    }
}


void drawFighter (Fighter *f, plat_api platAPI) {
    char_anim_data *animData = char_anim_data_ptr_hash_mapGetVal(&spongeGame->animations, f->animState.key);
    char_frame_data *currentFrame = &animData->frames[f->animState.currentFrame];

    sprite smSprite = defaultSprite();
    b32 flip = false;
    if (f->type == FIGHTER_TYPE_SPONGE) {
        smSprite.atlasKey = "sponge_atlas";
        flip = f->facing == DIRECTION_RIGHT;
    }
    else if (f->type == FIGHTER_TYPE_COCKROACH) {
        smSprite.atlasKey = "cockroach_atlas";
        flip = f->facing == DIRECTION_LEFT;
    }
    else {
        ASSERT(false);
    }
    smSprite.frameKey = currentFrame->frameKey;
    ASSERT(smSprite.frameKey != 0);

    if (flip) {
        mat3x3 posMatrix = mat3x3Translate(f->pos.x - currentFrame->xOffset, f->pos.y + currentFrame->yOffset);
        spriteManPushMatrix(posMatrix);
        mat3x3 scaleTransform = mat3x3ScaleXY(-1.0f, 1.0f);
        spriteManPushMatrix(scaleTransform);

        smSprite.pos = (vec2){ .x = 0.0f, .y = 16.0f };
        smSprite.anchor = (vec2){ .x = 0.0f, .y = 1.0f };
        spriteManAddSprite(smSprite);

        //drawHitBoxes(currentFrame, (vec2) {.x = (f32)-currentFrame->xOffset, .y = (f32)-currentFrame->yOffset});
        //drawHitBoxes(currentFrame, spriteList, assets, (float)-currentFrame->xOffset, (float)-currentFrame->yOffset);
        spriteManPopMatrix();
        spriteManPopMatrix();

    }
    else {
        vec2 smSpriteOrigin = vec2Add(f->pos, 
                                      (vec2){ .x = currentFrame->xOffset, .y = currentFrame->yOffset + 16.0f });
        smSprite.pos = smSpriteOrigin;
            smSprite.anchor = (vec2){ .x = 0.0f, .y = 1.0f };
            spriteManAddSprite(smSprite);
        //drawHitBoxes(currentFrame, f->pos);
    }

}

f32 textWidth (sprite_text *text) {
    f32 width = 0;
    char *cursor = text->text;
    while (*cursor != 0) {
        bitmap_font_letter_coord c = spriteMan->fontLetterCoords[*cursor];
        width += c.advance;
        cursor++;
    }
    return width;
}

void centerText (sprite_text *text) {
    f32 width = textWidth(text);
    f32 offset = -((f32)width / 2.0f);
    text->x += offset;
    text->y -= 4.0f;
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
                } break;
                case TILE_COATING_TYPE_SUDS: {
                    {
                        sprite s = defaultSprite();
                        s.pos = coatingPos;
                        s.atlasKey = "game_atlas";
                        s.frameKey = "suds_0";
                        if (c->amount > 10)  {
                            s.frameKey = "suds_1";
                        }
                        if (c->amount > 20)  {
                            s.frameKey = "suds_2";
                        }
                        spriteManAddSprite(s);
                    }
                } break;
                case TILE_COATING_TYPE_GRIME: {
                    {
                        sprite s = defaultSprite();
                        s.pos = coatingPos;
                        s.atlasKey = "game_atlas";
                        s.frameKey = "grime_0";
                        if (c->amount > 10)  {
                            s.frameKey = "grime_1";
                        }
                        if (c->amount > 20)  {
                            s.frameKey = "grime_2";
                        }
                        spriteManAddSprite(s);
                    }
                } break;
            }
        }
    }

    // SPONGEMAN
    drawFighter(&spongeGame->spongeMan.fighter, platAPI);

    for (i32 i = 0; i < spongeGame->cockroaches.numValues; i++) {
        Cockroach *c = &spongeGame->cockroaches.values[i];
        drawFighter(&c->fighter, platAPI);
    }

    for (i32 i = 0; i < MAX_NUM_SUDS; i++) {
        Sud *sud = &spongeGame->suds[i];
        if (sud->active) {
            sprite s = defaultSprite();
            s.pos = sud->pos;
            s.atlasKey = "game_atlas";
            s.frameKey = "particle_white_big";
            spriteManAddSprite(s);
        }
    }
    //{
    //    sprite s = defaultSprite();
    //    s.pos = spongeGame->spongeMan.pos;
    //    s.atlasKey = "game_atlas";
    //    s.frameKey = "sponge_man";
    //    s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
    //}

    
    {
    char *text = tempStringAppend("Sponge HP: ", tempStringFromI32(spongeGame->spongeMan.fighter.hitPoints));
        sprite_text labelText = {
            .text = text,
            .fontKey = "font",
            .x = 8.0f,
            .y = 8.0f
        };
        //centerText(&labelText);
        spriteManAddText(labelText);
    }

    if (spongeGame->lastHitEnemy) {
    char *text = tempStringAppend("Enemy HP: ", tempStringFromI32(spongeGame->lastHitEnemy->hitPoints));
        sprite_text labelText = {
            .text = text,
            .fontKey = "font",
            .x = 8.0f,
            .y = 20.0f
        };
        //centerText(&labelText);
        spriteManAddText(labelText);
    }

}
