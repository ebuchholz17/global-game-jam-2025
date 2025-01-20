#include "block_game.h"
#include "../gng_bool.h"

#define GRID_ROW_START (-((f32)(NUM_GRID_ROWS - 1) * GRID_BLOCK_HEIGHT) * 0.5f)
#define GRID_COL_START (-((f32)(NUM_GRID_COLS - 1) * GRID_BLOCK_HEIGHT) * 0.5f)

void addNewBlockToGrid (block_game *blockGame, i32 row, i32 col, char *color) {
    i32 freeBlockIndex = -1;
    for (i32 i = 0; i < blockGame->numBlocks; ++i) {
        grid_block *block = blockGame->blocks + i;
        if (!block->active) {
            freeBlockIndex = i;
            break;
        }
    }

    grid_block *block;
    if (freeBlockIndex != -1) {
        block = blockGame->blocks + freeBlockIndex;
        zeroMemory((u8 *)block, sizeof(grid_block));
        block->id = freeBlockIndex;
    }
    else {
        block = &blockGame->blocks[blockGame->numBlocks];
        zeroMemory((u8 *)block, sizeof(grid_block));
        block->id = blockGame->numBlocks;
        blockGame->numBlocks++;
    }
    block->active = true;

    block->row = row;
    block->col = col;
    block->color = color;

    // TODO(ebuchholz): un-copy-paste gridrow/colstart
    block->x = GRID_COL_START + GRID_BLOCK_WIDTH * block->col;
    block->y = GRID_ROW_START + GRID_BLOCK_HEIGHT * block->row;

    blockGame->grid[row * NUM_GRID_COLS + col] = block->id;
}

void addBlockIfUnoccupied (block_game *blockGame, i32 row, i32 col, char *color) {
    if (blockGame->grid[row * NUM_GRID_COLS + col] == -1) {
        addNewBlockToGrid(blockGame, row, col, color);
    }
}


direction startMovingSheep (block_game *blockGame, block_game_input *input) {
    block_sheep *sheep = &blockGame->sheep;
    if (blockGame->nextMoveDirection == DIRECTION_NONE) {
        if (input->up.down) {
            blockGame->nextMoveDirection = DIRECTION_UP;
        }
        else if (input->down.down) {
            blockGame->nextMoveDirection = DIRECTION_DOWN;
        }
        else if (input->left.down) {
            blockGame->nextMoveDirection = DIRECTION_LEFT;
        }
        else if (input->right.down) {
            blockGame->nextMoveDirection = DIRECTION_RIGHT;
        }
    }
    return blockGame->nextMoveDirection;
}

void moveSheep (block_game *blockGame, block_game_input *input, f32 dt) {
    block_sheep *sheep = &blockGame->sheep;
    sheep->moveTime += dt;
    if (sheep->moveTime > BLOCK_MOVE_SPEED) { 
        sheep->moveTime = BLOCK_MOVE_SPEED; 
    }

    f32 prevX = GRID_COL_START + GRID_BLOCK_WIDTH * sheep->prevCol;
    f32 prevY = GRID_ROW_START + GRID_BLOCK_WIDTH * sheep->prevRow;

    f32 nextX = GRID_COL_START + GRID_BLOCK_WIDTH * sheep->col;
    f32 nextY = GRID_ROW_START + GRID_BLOCK_WIDTH * sheep->row;

    f32 t = sheep->moveTime / BLOCK_MOVE_SPEED;
    sheep->x = prevX + t * (nextX - prevX);
    sheep->y = prevY + t * (nextY - prevY);

    if (sheep->moveTime == BLOCK_MOVE_SPEED) {
        sheep->moving = false;
    }
}

void moveBlocks (block_game *blockGame, block_game_input *input, f32 dt) {
    grid_block *blocks = blockGame->blocks;
    for (i32 i = 0; i < blockGame->numBlocks; ++i) {
        grid_block *block = blocks + i;

        if (block->active) {
            if (block->moving) {
                block->moveTime += dt;
                if (block->moveTime > BLOCK_MOVE_SPEED) { block->moveTime = BLOCK_MOVE_SPEED; }

                f32 prevX = GRID_COL_START + GRID_BLOCK_WIDTH * block->prevCol;
                f32 prevY = GRID_ROW_START + GRID_BLOCK_WIDTH * block->prevRow;

                f32 nextX = GRID_COL_START + GRID_BLOCK_WIDTH * block->col;
                f32 nextY = GRID_ROW_START + GRID_BLOCK_WIDTH * block->row;

                f32 t = block->moveTime / BLOCK_MOVE_SPEED;
                block->x = prevX + t * (nextX - prevX);
                block->y = prevY + t * (nextY - prevY);

                if (block->moveTime == BLOCK_MOVE_SPEED) {
                    block->moving = false;
                }
            }
            else {
                block->x = GRID_COL_START + GRID_BLOCK_WIDTH * block->col;
                block->y = GRID_ROW_START + GRID_BLOCK_HEIGHT * block->row;
            }
        } 
    }
}

void nextRowCol (i32 row, i32 col, direction dir, i32 *outRow, i32 *outCol) {
    *outRow = row;
    *outCol = col;
    if (dir == DIRECTION_UP) {
        *outRow = row - 1;
    }
    else if (dir == DIRECTION_DOWN) {
        *outRow = row + 1;
    }
    else if (dir == DIRECTION_LEFT) {
        *outCol = col - 1;
    }
    else if (dir == DIRECTION_RIGHT) {
        *outCol = col + 1;
    }
}

b32 tryMoveBlockInDirection (block_game *blockGame, i32 row, i32 col, direction dir) {
    i32 nextRow, nextCol;
    nextRowCol(row, col, dir, &nextRow, &nextCol);
    if (nextRow < 0 || nextRow >= NUM_GRID_ROWS || 
        nextCol < 0 || nextCol >= NUM_GRID_COLS) 
    {
        return false;
    }
    i32 nextGridID = blockGame->grid[nextRow * NUM_GRID_ROWS + nextCol];

    b32 canMoveBlock = false;
    if (nextGridID == -1) {
        canMoveBlock = true;
    }
    else {
        canMoveBlock = tryMoveBlockInDirection(blockGame, nextRow, nextCol, dir);
    }

    if (canMoveBlock) {
        i32 blockID = blockGame->grid[row * NUM_GRID_ROWS + col];
        grid_block *block = &blockGame->blocks[blockID];
        block->prevRow = block->row;
        block->prevCol = block->col;
        block->row = nextRow;
        block->col = nextCol;
        block->moving = true;
        block->moveTime = 0.0f;
        blockGame->grid[nextRow * NUM_GRID_ROWS + nextCol] = blockID;
        blockGame->grid[row * NUM_GRID_ROWS + col] = -1;
    }

    return canMoveBlock;
}

void tryMoveSheep (block_game *blockGame, block_sheep *sheep, block_game_input *input) {
    if (!sheep->moving) {
        sheep->prevRow = sheep->row;
        sheep->prevCol = sheep->col;

        sheep->x = GRID_COL_START + GRID_BLOCK_WIDTH * sheep->col;
        sheep->y = GRID_ROW_START + GRID_BLOCK_HEIGHT * sheep->row;

        direction sheepMoveDir = startMovingSheep(blockGame, input);
        if (sheepMoveDir != DIRECTION_NONE) {
            blockGame->nextMoveDirection = DIRECTION_NONE;

            i32 nextRow = sheep->row;
            i32 nextCol = sheep->col;

            b32 validMove = true;
            nextRowCol(nextRow, nextCol, sheepMoveDir, &nextRow, &nextCol);

            if (nextRow < 0 || nextRow >= NUM_GRID_ROWS || 
                nextCol < 0 || nextCol >= NUM_GRID_COLS) 
            {
                validMove = false;
            }
            else if (blockGame->grid[nextRow * NUM_GRID_COLS + nextCol] != -1) {
                validMove = tryMoveBlockInDirection(blockGame, nextRow, nextCol, sheepMoveDir);
            }
            
            if (validMove) {
                sheep->row = nextRow;
                sheep->col = nextCol;
                sheep->moving = true;
                sheep->moveTime = 0.0f;
            }
            else {
                soundManPlaySound("sfx_impact");
            }
        }
    }
}

void blockBubbleSort (grid_block *blocks, block_game *blockGame) {
    for (i32 i = 0; i < blockGame->numBlocks; ++i) {
        for (i32 j = i; j > 0; --j) {
            grid_block firstBlock = blocks[j-1];
            grid_block secondBlock = blocks[j];
            if (secondBlock.row < firstBlock.row) {
                blocks[j-1] = secondBlock;
                blocks[j] = firstBlock;
            }
        }
    }
}

b32 blockFitsOnGrid (block_piece *blockPiece, i32 row, i32 col) {
    for (i32 i = -2; i <= 2; ++i) {
        for (i32 j = -2; j <= 2; ++j) {
            if (blockPiece->filledCells[(i+2) * 5 + (j+2)] == 1) {
                i32 gridRow = i + row;
                i32 gridCol = j + col;
                if (gridRow < 0 || gridRow >= NUM_GRID_ROWS) { return false; }
                if (gridCol < 0 || gridCol >= NUM_GRID_COLS) { return false; }
            }
        }
    }
    return true;
}

void copyBlockType (block_piece *nextPiece, i32 *blockType) {
    for (i32 i = 0; i < 5*5; ++i) {
        nextPiece->filledCells[i] = blockType[i];
    }
}

void chooseNextBlockPiece (block_piece *blockPiece) {
    u32 nextPieceType = randomU32() % 19;

    i32 *nextPiece;
    switch (nextPieceType) {
        default: {
            nextPiece = 0;
            ASSERT(false);
        } break;
        case 0: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 1: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 1, 1, 0,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 2: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 3: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 1, 1, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 4: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 5: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                0, 1, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 6: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                0, 0, 1, 1, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 7: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 1, 1, 0,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 8: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 9: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                0, 1, 1, 1, 0,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 10: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 1, 1, 0,
                0, 0, 1, 1, 0,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 11: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 1, 1, 1, 0,
                0, 1, 1, 1, 0,
                0, 1, 1, 1, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 12: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 1, 1, 1, 0,
                0, 1, 0, 0, 0,
                0, 1, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 13: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 1, 1, 1, 0,
                0, 0, 0, 1, 0,
                0, 0, 0, 1, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 14: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 1, 0, 0, 0,
                0, 1, 0, 0, 0,
                0, 1, 1, 1, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 15: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 0, 1, 0,
                0, 0, 0, 1, 0,
                0, 1, 1, 1, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 16: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                0, 1, 1, 1, 1,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 17: {
            i32 blockType[] = {
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 18: {
            i32 blockType[] = {
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0,
                1, 1, 1, 1, 1,
                0, 0, 0, 0, 0,
                0, 0, 0, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
        case 19: {
            i32 blockType[] = {
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 1, 0, 0
            };
            copyBlockType(blockPiece, blockType);
        } break;
    }


    u32 randomColor = randomU32() % 6;
    switch (randomColor) {
        case 0: {
            blockPiece->color = "red_tile";
        } break;
        case 1: {
            blockPiece->color = "green_tile";
        } break;
        case 2: {
            blockPiece->color = "blue_tile";
        } break;
        case 3: {
            blockPiece->color = "cyan_tile";
        } break;
        case 4: {
            blockPiece->color = "magenta_tile";
        } break;
        case 5: {
            blockPiece->color = "yellow_tile";
        } break;
    }
}

void chooseNextBlockPieceLocation (block_game *blockGame) {
    block_piece *blockPiece = &blockGame->nextPiece;

    i32 row = 0;
    i32 col = 0;
    do {
        row = randomU32() % NUM_GRID_ROWS;
        col = randomU32() % NUM_GRID_COLS;
    } while (!blockFitsOnGrid(blockPiece, row, col));

    blockGame->nextBlockPieceRow = row;
    blockGame->nextBlockPieceCol = col;
}

void loadBlockGameHitboxData (block_game *blockGame, char *key, mem_arena *memory) {
    data_asset *hitboxData = getDataAsset(key);

    char_anim_data *animData = (char_anim_data *)allocMemory(memory, sizeof(char_anim_data));
    loadHitboxData(assetMan, hitboxData->data, animData);

    char_anim_data_ptr_hash_mapStore(&blockGame->animations, animData, key);
}

void startCharacterAnimState (animation_state *playerAnimState, char *key) {
    playerAnimState->currentFrameStep = 0;
    playerAnimState->currentFrame = 0;
    playerAnimState->key = key;
}

b32 updateCharacterAnimState (animation_state *playerAnimState, block_game *blockGame) {
    char_anim_data *animData = char_anim_data_ptr_hash_mapGetVal(&blockGame->animations, playerAnimState->key);
    char_frame_data *currentFrame = &animData->frames[playerAnimState->currentFrame];

    b32 animationComplete = false;
    ++playerAnimState->currentFrameStep;
    if (playerAnimState->currentFrameStep >= currentFrame->duration) {
        playerAnimState->currentFrameStep = 0;
        ++playerAnimState->currentFrame;
        if (playerAnimState->currentFrame == animData->numFrames) {
            playerAnimState->currentFrame = 0;
            animationComplete = true;
        }
    }
    return animationComplete;
}

void initBlockGame (block_game* blockGame, mem_arena *memory) {
    zeroMemory((u8 *)blockGame, sizeof(block_game));
    grid_block *blocks = blockGame->blocks;

    for (i32 i = 0; i < NUM_GRID_ROWS; ++i) {
        for (i32 j = 0; j < NUM_GRID_COLS; ++j) {
            blockGame->grid[i * NUM_GRID_COLS + j] = -1;
        }
    }

    blockGame->sheep = (block_sheep){
        .row = 6,
        .col = 6
    };

    blockGame->score = 0;

    blockGame->timeToSpawnNextPiece = 5.0f;
    blockGame->nextPieceTimer = 5.0f;

    blockGame->blockIndicatorTimer = 0.0f;
    blockGame->fadingInBlockIndicator = true;

    chooseNextBlockPiece(&blockGame->nextPiece);
    chooseNextBlockPieceLocation(blockGame);

    blockGame->gameState = BLOCK_GAME_STATE_MOVING_BLOCKS;
    blockGame->animations = char_anim_data_ptr_hash_mapInit(memory, 50);


    if (!blockGame->isInitialized) {
        blockGame->isInitialized = true;
    }
}

void updateSheepAndBlocks (block_game *blockGame, block_game_input* input, f32 blinkRate, f32 dt) {
    block_sheep *sheep = &blockGame->sheep;
    tryMoveSheep(blockGame, sheep, input);
    if (sheep->moving) {
        moveSheep(blockGame, input, dt);
    }
    moveBlocks(blockGame, input, dt);

    // check for lines
    b32 clearedLines = false;
    for (i32 i = 0; i < NUM_GRID_ROWS; ++i) {
        b32 foundFullRow = true;
        for (i32 j = 0; j < NUM_GRID_COLS; ++j) {
            i32 blockID = blockGame->grid[i * NUM_GRID_ROWS + j];
            if (blockID == -1) {
                foundFullRow = false;
                break;
            }
            else {
                grid_block *block = blockGame->blocks + blockID;
                if (block->moving) {
                    foundFullRow = false;
                    break;
                }
            }
        }
        if (foundFullRow) {
            blockGame->rowsBeingCleared[i] = true;
            clearedLines = true;
        }
        else {
            blockGame->rowsBeingCleared[i] = false;
        }
    }

    for (i32 i = 0; i < NUM_GRID_COLS; ++i) {
        b32 foundFullCol = true;
        for (i32 j = 0; j < NUM_GRID_ROWS; ++j) {
            i32 blockID = blockGame->grid[j * NUM_GRID_ROWS + i];
            if (blockID == -1) {
                foundFullCol = false;
                break;
            }
            else {
                grid_block *block = blockGame->blocks + blockID;
                if (block->moving) {
                    foundFullCol = false;
                    break;
                }
            }
        }
        if (foundFullCol) {
            blockGame->colsBeingCleared[i] = true;
            clearedLines = true;
        }
        else {
            blockGame->colsBeingCleared[i] = false;
        }
    }

    if (clearedLines) {
        blockGame->gameState = BLOCK_GAME_STATE_CLEARING_LINES;
        blockGame->clearedBlocksVisible = true;
        blockGame->clearingBlocksTimer = 0.0f;
        blockGame->score += 100;
    }

    // show an indicator of where the next piece will be
    if (blockGame->fadingInBlockIndicator) {
        blockGame->blockIndicatorTimer += dt;
        if (blockGame->blockIndicatorTimer > blinkRate) {
            blockGame->blockIndicatorTimer = blinkRate - (blockGame->blockIndicatorTimer - blinkRate);
            blockGame->fadingInBlockIndicator = false;
        }
    }
    else {
        blockGame->blockIndicatorTimer -= dt;
        if (blockGame->blockIndicatorTimer <= 0.0f) {
            blockGame->blockIndicatorTimer = -blockGame->blockIndicatorTimer;
            blockGame->fadingInBlockIndicator = true;
        }
    }

    blockGame->nextPieceTimer += dt;
    if (blockGame->nextPieceTimer >= blockGame->timeToSpawnNextPiece) {
        blockGame->nextPieceTimer -= blockGame->timeToSpawnNextPiece;

        block_piece *nextPiece = &blockGame->nextPiece;
        for (i32 i = -2; i <= 2; ++i) {
            for (i32 j = -2; j <= 2; ++j) {
                if (nextPiece->filledCells[(i+2) * 5 + (j+2)]) {
                    i32 gridRow = i + blockGame->nextBlockPieceRow;
                    i32 gridCol = j + blockGame->nextBlockPieceCol;
                    addBlockIfUnoccupied(blockGame, gridRow, gridCol, nextPiece->color);
                }
            }
        }
        chooseNextBlockPiece(&blockGame->nextPiece);
        chooseNextBlockPieceLocation(blockGame);

        blockGame->blockIndicatorTimer = 0.0f;
        blockGame->fadingInBlockIndicator = true;
    }

}

block_game_input parseGameInput (game_input *input, virtual_input *vInput, block_game *blockGame) {
    block_game_input result = {0};
    if (input->leftArrow.down || 
        input->rightArrow.down || 
        input->upArrow.down || 
        input->downArrow.down ||
        input->aKey.down) 
    {
        blockGame->inputSource = INPUT_SOURCE_KEYBOARD;
    }
    else if (vInput->dPadUp.button.down || 
             vInput->dPadDown.button.down || 
             vInput->dPadLeft.button.down || 
             vInput->dPadRight.button.down)
    {
        blockGame->inputSource = INPUT_SOURCE_VIRTUAL;
    }
    else {
        for (u32 controllerIndex = 0; controllerIndex < MAX_NUM_CONTROLLERS; controllerIndex++) {
            game_controller_input *cont = &input->controllers[controllerIndex];

            if (cont->connected) {
                b32 useController;
                if (cont->dPadUp.down || cont->dPadLeft.down || cont->dPadDown.down || cont->dPadRight.down) {
                    blockGame->inputSource = INPUT_SOURCE_GAMEPAD;
                    break;
                }
            }
        }
    }

    switch (blockGame->inputSource) {
        case INPUT_SOURCE_KEYBOARD: {
            result.left = input->leftArrow;
            result.right = input->rightArrow;
            result.up = input->upArrow;
            result.down = input->downArrow;
            result.a = input->aKey;
            result.z = input->zKey;
        } break;
        case INPUT_SOURCE_VIRTUAL: {
            result.up = vInput->dPadUp.button;;
            result.down = vInput->dPadDown.button;
            result.left = vInput->dPadLeft.button;
            result.right = vInput->dPadRight.button;
            result.a = vInput->bottomButton.button;
            result.z = vInput->rightButton.button;
        } break;
        case INPUT_SOURCE_GAMEPAD: {
            for (u32 controllerIndex = 0; controllerIndex < MAX_NUM_CONTROLLERS; controllerIndex++) {
                game_controller_input *cont = &input->controllers[controllerIndex];

                if (cont->connected) {
                    result.left = cont->dPadLeft;
                    result.right = cont->dPadRight;
                    result.up = cont->dPadUp;
                    result.down = cont->dPadDown;
                    result.a = cont->aButton;
                    result.z = cont->bButton;
                    break;
                }
            }
        } break;
    }

    return result;
}

void updateBlockGame (block_game *blockGame, game_input *input, virtual_input *vInput, f32 dt, plat_api platAPI, mem_arena *memory) {
    if (!blockGame->animationsLoaded) {

        loadBlockGameHitboxData(blockGame, "sheep_circle_attack", memory);
        loadBlockGameHitboxData(blockGame, "sheep_stomp_attack", memory);
        blockGame->animationsLoaded = true;
    }

    block_game_input bgInput = parseGameInput(input, vInput, blockGame);

    if (!blockGame->sheepAnimating) {
        if (bgInput.up.justPressed) {
            blockGame->nextMoveDirection = DIRECTION_UP;
        }
        else if (bgInput.down.justPressed) {
            blockGame->nextMoveDirection = DIRECTION_DOWN;
        }
        else if (bgInput.left.justPressed) {
            blockGame->nextMoveDirection = DIRECTION_LEFT;
        }
        else if (bgInput.right.justPressed) {
            blockGame->nextMoveDirection = DIRECTION_RIGHT;
        }
        else if (bgInput.a.justPressed) {
            blockGame->sheepAnimating = true;
            startCharacterAnimState(&blockGame->sheepAnimState, "sheep_circle_attack");
        }
    }

    if (blockGame->sheepAnimating) {
        b32 animComplete = updateCharacterAnimState(&blockGame->sheepAnimState, blockGame);
        blockGame->sheepAnimating = !animComplete;
    }

    f32 blinkRate = (blockGame->timeToSpawnNextPiece - blockGame->nextPieceTimer) * 0.2f;
    if (blinkRate < 1.0f / 30.0f) { blinkRate = 1.0f / 30.0f; }

    switch (blockGame->gameState) {
        case BLOCK_GAME_STATE_MOVING_BLOCKS: {
            updateSheepAndBlocks(blockGame, &bgInput, blinkRate, dt);
        } break;
        case BLOCK_GAME_STATE_CLEARING_LINES: {
            blockGame->clearingBlocksTimer += dt;
            if (blockGame->clearingBlocksTimer > 0.5f) {
                for (i32 i = 0; i < NUM_GRID_ROWS; ++i) {
                    if (blockGame->rowsBeingCleared[i]) {
                        for (i32 j = 0; j < NUM_GRID_COLS; ++j) {
                            i32 blockID = blockGame->grid[i * NUM_GRID_ROWS + j];
                            grid_block *block = blockGame->blocks + blockID;

                            blockGame->grid[i * NUM_GRID_ROWS + j] = -1;
                            block->active = false;
                        }
                    }
                }
                for (i32 i = 0; i < NUM_GRID_COLS; ++i) {
                    if (blockGame->colsBeingCleared[i]) {
                        for (i32 j = 0; j < NUM_GRID_ROWS; ++j) {
                            i32 blockID = blockGame->grid[j * NUM_GRID_ROWS + i];
                            // may have been cleared in a row
                            if (blockID != -1) {
                                grid_block *block = blockGame->blocks + blockID;

                                blockGame->grid[j * NUM_GRID_ROWS + i] = -1;
                                block->active = false;
                            }
                        }
                    }
                }
                blockGame->gameState = BLOCK_GAME_STATE_MOVING_BLOCKS;
            }
            else {
                blockGame->clearedBlocksVisible = !blockGame->clearedBlocksVisible;
            }

        } break;
    }


    // some kind of pixel art antialiasing test
    //static f32 testT = 0.0f;
    //testT += DELTA_TIME * 0.1f;
    //addSprite(200.0f + 150.0f * sinf(testT+1.6f), 100.0f, assets, "atlas", "sheep", spriteList, 0.5f, 0.5f, 5.0f + 4.0f * sinf(testT), (testT * 1.0f) / 3.14f);
}

void drawBlockGame (block_game *blockGame, plat_api platAPI) {
    static float t = 0.0f;
    t += 0.007f;
    while (t >= 1000.0f) {
        t -= 1000.0f;
    }

    float scale = 3.0f + fastSin2PI(t * 0.05f) * 3.0f;
    float rotation = fastSin2PI(t * 0.0125f);
    vec2 pos = (vec2){
        .x = fastCos2PI(t * 0.03f) * 100.0f,
        .y = fastSin2PI(t * 0.03f) * 100.0f,
    };

    spriteManPushTransform((sprite_transform){
        .pos = { .x = 520.0f / 2.0f, .y = 240.0f / 2.0f },//vec2Add(pos, (vec2){ .x = 520.0f / 2.0f, .y = 240.0f / 2.0f }),
        .scale = 0.5f + scale,
        .rotation = rotation
    });

    spriteManAddText((sprite_text){
        .x = -184.0f,
        .y = -100.0f,
        .text = tempStringAppend("score: ", tempStringFromI32(blockGame->score)),
        .fontKey = "font"
    });

    for (i32 i = 0; i < NUM_GRID_ROWS; ++i) {
        for (i32 j = 0; j < NUM_GRID_COLS; ++j) {
            sprite s = defaultSprite();
            s.pos.x = GRID_COL_START + GRID_BLOCK_WIDTH * j;
            s.pos.y = GRID_ROW_START + GRID_BLOCK_HEIGHT * i;
            s.atlasKey = "atlas";
            s.frameKey = "tile_backing";
            s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
            spriteManAddSprite(s);
        }
    }

    grid_block sortedBlocks[NUM_GRID_ROWS * NUM_GRID_COLS];
    // TODO(ebuchholz): memory copy
    for (i32 i = 0; i < blockGame->numBlocks; ++i) {
        sortedBlocks[i] = blockGame->blocks[i];
    }
    blockBubbleSort(sortedBlocks, blockGame);

    for (i32 i = 0; i < blockGame->numBlocks; ++i) {
        grid_block *block = &sortedBlocks[i];
        if (!blockGame->clearedBlocksVisible && 
            (blockGame->rowsBeingCleared[block->row] || blockGame->colsBeingCleared[block->col])) 
        { 
            continue; 
        }
        if (block->active) {
            sprite s = defaultSprite();
            s.pos.x = block->x;
            s.pos.y = block->y;
            s.atlasKey = "atlas";
            s.frameKey = block->color;
            s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
            spriteManAddSprite(s);
        }
    }

    // indicator
    // TODO(ebuchholz): organize better? draw everything in switch statement?
    f32 blinkRate = (blockGame->timeToSpawnNextPiece - blockGame->nextPieceTimer) * 0.2f;
    if (blinkRate < 1.0f / 30.0f) { blinkRate = 1.0f / 30.0f; }

    f32 indicatorAlpha = blockGame->blockIndicatorTimer / blinkRate;
    block_piece *nextPiece = &blockGame->nextPiece;
    for (i32 i = -2; i <= 2; ++i) {
        for (i32 j = -2; j <= 2; ++j) {
            if (nextPiece->filledCells[(i+2) * 5 + (j+2)]) {
                i32 gridRow = i + blockGame->nextBlockPieceRow;
                i32 gridCol = j + blockGame->nextBlockPieceCol;
                if (gridRow >= 0 && gridRow < NUM_GRID_ROWS && 
                    gridCol >= 0 && gridCol < NUM_GRID_COLS) 
                {
                    sprite s = defaultSprite();
                    s.pos.x = GRID_COL_START + GRID_BLOCK_WIDTH * gridCol;
                    s.pos.y = GRID_ROW_START + GRID_BLOCK_HEIGHT * gridRow;
                    s.atlasKey = "atlas";
                    s.frameKey = "block_blink";
                    s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
                    s.alpha = indicatorAlpha;
                    spriteManAddSprite(s);
                }
            }
        }
    }

    if (blockGame->sheepAnimating) {
        char_anim_data *animData = char_anim_data_ptr_hash_mapGetVal(&blockGame->animations, blockGame->sheepAnimState.key);
        char_frame_data *currentFrame = &animData->frames[blockGame->sheepAnimState.currentFrame];

        sprite sheepSprite = defaultSprite();

        sheepSprite.pos.x = blockGame->sheep.x + currentFrame->xOffset;
        sheepSprite.pos.y = blockGame->sheep.y + currentFrame->yOffset;
        sheepSprite.atlasKey = "atlas";
        sheepSprite.frameKey = "sheep";
        sheepSprite.anchor = (vec2){ .x = 0.0f, .y = 1.0f };
        spriteManAddSprite(sheepSprite);
    }
    else {
        sprite sheepSprite = defaultSprite();

        sheepSprite.pos.x = blockGame->sheep.x;
        sheepSprite.pos.y = blockGame->sheep.y;
        sheepSprite.atlasKey = "atlas";
        sheepSprite.frameKey = "sheep";
        sheepSprite.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(sheepSprite);
    }

    spriteManPopMatrix();
}
