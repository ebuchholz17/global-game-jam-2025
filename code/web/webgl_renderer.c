#include "webgl_renderer.h"

void webglSpriteBatchOnStart (webgl_renderer *wglr) {
    wglr->spriteBatchNumSpritesDrawn = 0;
    wglr->spriteBatchNumSeenTextures = 0;
    for (u32 textureIndex = 0; textureIndex < MAX_NUM_WGLR_TEXTURES; textureIndex++) {
        wglr->spriteBatchSeenTextures[textureIndex] = UNUSED_TEXTURE_VAL;
    }
}

void webglFlushSprites (webgl_renderer *wglr) {
    u32 textureIDs[8];
    u32 numTextureIDsToBind = 0;
    for (u32 textureIndex = 0; textureIndex < MAX_NUM_WGLR_TEXTURES; textureIndex++) {
        u32 shaderTextureIndex = wglr->spriteBatchSeenTextures[textureIndex];
        if (shaderTextureIndex != UNUSED_TEXTURE_VAL) {
            textureIDs[shaderTextureIndex] = wglr->textures[textureIndex].id;
            numTextureIDsToBind++;
        }
    }
    webglSpriteBatchFlush(wglr->batchStart, wglr->spriteBatchNumSpritesDrawn, sizeof(wglr_vertex), 
                          wglr->numSpritesDrawnThisFrame, textureIDs, numTextureIDsToBind);
    wglr->batchStart = wglr->currentVertex;
}


void webglRenderGame (webgl_renderer *wglr, mem_arena *renderMemory) {

    webglOnRenderStart();

    wglr->numSpritesDrawnThisFrame = 0;

    static f32 triT = 0.0f;
    triT += 1.0f / 144.0f;

    void *renderCursor = renderMemory->base;
    while (renderCursor < renderMemory->current) {
        render_cmd_header *header = (render_cmd_header *)renderCursor;
        renderCursor = (u8 *)renderCursor + sizeof(render_cmd_header);
        switch (header->type) {
            default: {
                ASSERT(false);
            } break;
            case RENDER_CMD_TYPE_LOAD_TEXTURE: {
                render_cmd_load_texture *cmd = (render_cmd_load_texture *)renderCursor;
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_load_texture);

                ASSERT(wglr->numTextures < MAX_NUM_WGLR_TEXTURES);

                wglr_texture *texture = &wglr->textures[wglr->numTextures++];
                texture->id = cmd->id;

                webglLoadTexture(texture->id, cmd->width, cmd->height, cmd->pixels);
            } break;
            case RENDER_CMD_TYPE_SPRITE_BATCH_START: {
                // NOTE: no data in render cmd
                webglSpriteBatchOnStart(wglr);
                webglSpriteBatchStart();
            } break;
            case RENDER_CMD_TYPE_SPRITE_BATCH_DRAW: {
                render_cmd_sprite_batch_draw *cmd = (render_cmd_sprite_batch_draw *)renderCursor;
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_sprite_batch_draw);

                ASSERT(cmd->numSprites + wglr->numSpritesDrawnThisFrame <= MAX_NUM_SPRITES);

                // TODO: alloc big working space to copy vertices into the right format
                // use wglr_vertex, then send pointer to web side
                wglr_vertex *vertexData = (wglr_vertex *)webAllocTempMemory(cmd->numSprites * 4 * sizeof(wglr_vertex));

                wglr->batchStart = vertexData;
                wglr->currentVertex = vertexData;
                for (u32 spriteIndex = 0; spriteIndex < cmd->numSprites; spriteIndex++) {
                    render_cmd_sprite_data *sprite = &cmd->sprites[spriteIndex];

                    ASSERT(sprite->textureID < MAX_NUM_WGLR_TEXTURES);

                    u32 shaderTextureIndex = UNUSED_TEXTURE_VAL;
                    if (wglr->spriteBatchSeenTextures[sprite->textureID] == UNUSED_TEXTURE_VAL) {
                        if (wglr->spriteBatchNumSeenTextures >= 8) {
                            webglFlushSprites(wglr);
                            webglSpriteBatchOnStart(wglr);
                        }
                        wglr->spriteBatchSeenTextures[sprite->textureID] = wglr->spriteBatchNumSeenTextures;
                        shaderTextureIndex = wglr->spriteBatchNumSeenTextures;
                        wglr->spriteBatchNumSeenTextures++;
                    }
                    else {
                        shaderTextureIndex = wglr->spriteBatchSeenTextures[sprite->textureID];
                    }

                    for (u32 vIndex = 0; vIndex < 4; vIndex++) {
                        wglr->currentVertex[vIndex].pos[0] = sprite->positions[vIndex * 3 + 0];
                        wglr->currentVertex[vIndex].pos[1] = sprite->positions[vIndex * 3 + 1];
                        wglr->currentVertex[vIndex].pos[2] = sprite->positions[vIndex * 3 + 2];
                        wglr->currentVertex[vIndex].texCoords[0] = sprite->texCoords[vIndex * 2 + 0];
                        wglr->currentVertex[vIndex].texCoords[1] = sprite->texCoords[vIndex * 2 + 1];
                        wglr->currentVertex[vIndex].color[0] = sprite->colors[vIndex * 4 + 0];
                        wglr->currentVertex[vIndex].color[1] = sprite->colors[vIndex * 4 + 1];
                        wglr->currentVertex[vIndex].color[2] = sprite->colors[vIndex * 4 + 2];
                        wglr->currentVertex[vIndex].color[3] = sprite->colors[vIndex * 4 + 3];
                        wglr->currentVertex[vIndex].textureID = (f32)shaderTextureIndex;
                    }

                    wglr->currentVertex += 4;

                    wglr->spriteBatchNumSpritesDrawn++;
                }

                // advance past the list of sprites
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_sprite_data) * cmd->numSprites;
            } break;
            case RENDER_CMD_TYPE_SPRITE_BATCH_END: {
                // NOTE: no data in render cmd

                // flush leftover sprites
                webglFlushSprites(wglr);
            } break;
        }
    }
}
