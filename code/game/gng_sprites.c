#include "gng_sprites.h"

sprite_man *spriteMan; 

void spriteManInit (sprite_man *sm, asset_man *am) {
    spriteMan = sm;
    spriteMan->assetMan = am;

    bitmap_font_letter_coord coord;

    // initialize everything to space
    // TODO(ebuchholz): maybe add support for bitmap fonts loaded from files
    coord.x = 121.0f;
    coord.y = 41.0f;
    coord.advance = 4;
    for (int i = 0; i < 255; ++i) {
        sm->fontLetterCoords[i] = coord;
    }

    coord.x = 1.0f;
    coord.y = 1.0f;
    coord.advance = 7;
    sm->fontLetterCoords['a'] = coord;
    sm->fontLetterCoords['A'] = coord;

    coord.x = 11.0f;
    coord.y = 1.0f;
    coord.advance = 6;
    sm->fontLetterCoords['b'] = coord;
    sm->fontLetterCoords['B'] = coord;

    coord.x = 21.0f;
    coord.y = 1.0f;
    coord.advance = 7;
    sm->fontLetterCoords['c'] = coord;
    sm->fontLetterCoords['C'] = coord;

    coord.x = 31.0f;
    coord.y = 1.0f;
    coord.advance = 6;
    sm->fontLetterCoords['d'] = coord;
    sm->fontLetterCoords['D'] = coord;

    coord.x = 41.0f;
    coord.y = 1.0f;
    coord.advance = 6;
    sm->fontLetterCoords['e'] = coord;
    sm->fontLetterCoords['E'] = coord;

    coord.x = 51.0f;
    coord.y = 1.0f;
    coord.advance = 6;
    sm->fontLetterCoords['f'] = coord;
    sm->fontLetterCoords['F'] = coord;

    coord.x = 61.0f;
    coord.y = 1.0f;
    coord.advance = 7;
    sm->fontLetterCoords['g'] = coord;
    sm->fontLetterCoords['G'] = coord;

    coord.x = 71.0f;
    coord.y = 1.0f;
    coord.advance = 5;
    sm->fontLetterCoords['h'] = coord;
    sm->fontLetterCoords['H'] = coord;

    coord.x = 81.0f;
    coord.y = 1.0f;
    coord.advance = 6;
    sm->fontLetterCoords['i'] = coord;
    sm->fontLetterCoords['I'] = coord;

    coord.x = 91.0f;
    coord.y = 1.0f;
    coord.advance = 6;
    sm->fontLetterCoords['j'] = coord;
    sm->fontLetterCoords['J'] = coord;

    coord.x = 101.0f;
    coord.y = 1.0f;
    coord.advance = 6;
    sm->fontLetterCoords['k'] = coord;
    sm->fontLetterCoords['K'] = coord;

    coord.x = 111.0f;
    coord.y = 1.0f;
    coord.advance = 7;
    sm->fontLetterCoords['l'] = coord;
    sm->fontLetterCoords['L'] = coord;

    coord.x = 121.0f;
    coord.y = 1.0f;
    coord.advance = 6;
    sm->fontLetterCoords['m'] = coord;
    sm->fontLetterCoords['M'] = coord;

    coord.x = 131.0f;
    coord.y = 1.0f;
    coord.advance = 6;
    sm->fontLetterCoords['n'] = coord;
    sm->fontLetterCoords['N'] = coord;

    coord.x = 1.0f;
    coord.y = 11.0f;
    coord.advance = 7;
    sm->fontLetterCoords['o'] = coord;
    sm->fontLetterCoords['O'] = coord;

    coord.x = 11.0f;
    coord.y = 11.0f;
    coord.advance = 5;
    sm->fontLetterCoords['p'] = coord;
    sm->fontLetterCoords['P'] = coord;

    coord.x = 21.0f;
    coord.y = 11.0f;
    coord.advance = 6;
    sm->fontLetterCoords['q'] = coord;
    sm->fontLetterCoords['Q'] = coord;

    coord.x = 31.0f;
    coord.y = 11.0f;
    coord.advance = 7;
    sm->fontLetterCoords['r'] = coord;
    sm->fontLetterCoords['R'] = coord;

    coord.x = 41.0f;
    coord.y = 11.0f;
    coord.advance = 6;
    sm->fontLetterCoords['s'] = coord;
    sm->fontLetterCoords['S'] = coord;

    coord.x = 51.0f;
    coord.y = 11.0f;
    coord.advance = 6;
    sm->fontLetterCoords['t'] = coord;
    sm->fontLetterCoords['T'] = coord;

    coord.x = 61.0f;
    coord.y = 11.0f;
    coord.advance = 6;
    sm->fontLetterCoords['u'] = coord;
    sm->fontLetterCoords['U'] = coord;

    coord.x = 71.0f;
    coord.y = 11.0f;
    coord.advance = 7;
    sm->fontLetterCoords['v'] = coord;
    sm->fontLetterCoords['V'] = coord;

    coord.x = 81.0f;
    coord.y = 11.0f;
    coord.advance = 7;
    sm->fontLetterCoords['w'] = coord;
    sm->fontLetterCoords['W'] = coord;

    coord.x = 91.0f;
    coord.y = 11.0f;
    coord.advance = 6;
    sm->fontLetterCoords['x'] = coord;
    sm->fontLetterCoords['X'] = coord;

    coord.x = 101.0f;
    coord.y = 11.0f;
    coord.advance = 6;
    sm->fontLetterCoords['y'] = coord;
    sm->fontLetterCoords['Y'] = coord;

    coord.x = 111.0f;
    coord.y = 11.0f;
    coord.advance = 6;
    sm->fontLetterCoords['z'] = coord;
    sm->fontLetterCoords['Z'] = coord;

    coord.x = 121.0f;
    coord.y = 11.0f;
    coord.advance = 5;
    sm->fontLetterCoords['0'] = coord;

    coord.x = 131.0f;
    coord.y = 11.0f;
    coord.advance = 4;
    sm->fontLetterCoords['1'] = coord;

    coord.x = 1.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['2'] = coord;

    coord.x = 11.0f;
    coord.y = 21.0f;
    coord.advance = 4;
    sm->fontLetterCoords['3'] = coord;

    coord.x = 21.0f;
    coord.y = 21.0f;
    coord.advance = 5;
    sm->fontLetterCoords['4'] = coord;

    coord.x = 31.0f;
    coord.y = 21.0f;
    coord.advance = 5;
    sm->fontLetterCoords['5'] = coord;

    coord.x = 41.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['6'] = coord;

    coord.x = 51.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['7'] = coord;

    coord.x = 61.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['8'] = coord;

    coord.x = 71.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['9'] = coord;

    coord.x = 81.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['`'] = coord;

    coord.x = 91.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['~'] = coord;

    coord.x = 101.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['!'] = coord;

    coord.x = 111.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['@'] = coord;

    coord.x = 121.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['#'] = coord;

    coord.x = 131.0f;
    coord.y = 21.0f;
    coord.advance = 6;
    sm->fontLetterCoords['$'] = coord;

    coord.x = 1.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['%'] = coord;

    coord.x = 11.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['^'] = coord;

    coord.x = 21.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['&'] = coord;

    coord.x = 31.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['*'] = coord;

    coord.x = 41.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['('] = coord;

    coord.x = 51.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords[')'] = coord;

    coord.x = 61.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['-'] = coord;

    coord.x = 71.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['_'] = coord;

    coord.x = 81.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['='] = coord;

    coord.x = 91.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['+'] = coord;

    coord.x = 101.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['['] = coord;

    coord.x = 111.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['{'] = coord;

    coord.x = 121.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords[']'] = coord;

    coord.x = 131.0f;
    coord.y = 31.0f;
    coord.advance = 6;
    sm->fontLetterCoords['}'] = coord;

    coord.x = 1.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords[';'] = coord;

    coord.x = 11.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords[':'] = coord;

    coord.x = 21.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords['\''] = coord;

    coord.x = 31.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords['"'] = coord;

    coord.x = 41.0f;
    coord.y = 41.0f;
    coord.advance = 5;
    sm->fontLetterCoords[','] = coord;

    coord.x = 51.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords['<'] = coord;

    coord.x = 61.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords['.'] = coord;

    coord.x = 71.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords['>'] = coord;

    coord.x = 81.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords['/'] = coord;

    coord.x = 91.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords['?'] = coord;

    coord.x = 101.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords['\\'] = coord;

    coord.x = 111.0f;
    coord.y = 41.0f;
    coord.advance = 6;
    sm->fontLetterCoords['|'] = coord;

    coord.x = 121.0f;
    coord.y = 41.0f;
    coord.advance = 4;
    sm->fontLetterCoords[' '] = coord;
}

void spriteManPushMatrix (mat3x3 transform) {
    ASSERT(spriteMan->matrixStackIndex < SPRITE_LIST_MAT_STACK_MAX);
    spriteMan->matrixStackIndex++;
    spriteMan->matrixStack[spriteMan->matrixStackIndex] = 
        mat3x3MatrixMul(spriteMan->matrixStack[spriteMan->matrixStackIndex - 1], transform);
}

mat3x3 spriteManPeekMatrix () {
    return spriteMan->matrixStack[spriteMan->matrixStackIndex];
}

mat3x3 spriteManPopMatrix () {
    ASSERT(spriteMan->matrixStackIndex > 0);
    mat3x3 result = spriteMan->matrixStack[spriteMan->matrixStackIndex];
    spriteMan->matrixStackIndex--;
    return result;
}

void spriteManPushTransform (sprite_transform transform) {
    mat3x3 matrix = mat3x3Scale(transform.scale);
    matrix = mat3x3MatrixMul(mat3x3Rotate2PI(transform.rotation), matrix);
    matrix = mat3x3MatrixMul(mat3x3Translate(transform.pos.x, transform.pos.y), matrix);
    spriteManPushMatrix(matrix);
}

sprite defaultSprite (void) {
    sprite result = {
        .scale = 1.0f,
        .alpha = 1.0f,
        .tint = 0xffffff,
        .textureID = SPRITE_LIST_SPRITE_TEX_UNSET
    };
    return result;
}

void spriteManAddSprite (sprite s) {
    ASSERT(spriteMan->sprites.numValues < SPRITE_LIST_MAX_NUM_SPRITES);

    s.parentTransform = spriteManPeekMatrix();

    // use pre-filled textureID, width+height to avoid going to hash map
    if (s.textureID != SPRITE_LIST_SPRITE_TEX_UNSET) {
        ASSERT(s.textureID >= 0 && s.textureID < spriteMan->assetMan->currentTextureID);
        ASSERT(s.width != 0 && s.height != 0);

        s.frameCorners[0] = (vec2){ .x = 0.0f, .y = 0.0f };
        s.frameCorners[1] = (vec2){ .x = 1.0f, .y = 0.0f };
        s.frameCorners[2] = (vec2){ .x = 0.0f, .y = 1.0f };
        s.frameCorners[3] = (vec2){ .x = 1.0f, .y = 1.0f };
    }
    // get texture from hash map then set properties
    else if (s.textureKey != 0) {
        texture_asset *texAsset = texture_asset_hash_mapGetPtr(&spriteMan->assetMan->textures, s.textureKey);
        ASSERT(texAsset != 0);
        s.textureID = texAsset->id;
        s.width = (float)texAsset->width;
        s.height = (float)texAsset->height;

        s.frameCorners[0] = (vec2){ .x = 0.0f, .y = 0.0f };
        s.frameCorners[1] = (vec2){ .x = 1.0f, .y = 0.0f };
        s.frameCorners[2] = (vec2){ .x = 0.0f, .y = 1.0f };
        s.frameCorners[3] = (vec2){ .x = 1.0f, .y = 1.0f };
    }
    // fills in width/height, textureID, and uvs from atlas
    else {
        ASSERT(s.atlasKey != 0);
        ASSERT(s.frameKey != 0);

        atlas_asset *atlas = getAtlas(spriteMan->assetMan, s.atlasKey);
        s.textureID = atlas->textureID;

        atlas_frame *frame = getAtlasFrame(spriteMan->assetMan, s.atlasKey, s.frameKey);
        s.width = (f32)frame->frameWidth;
        s.height = (f32)frame->frameHeight;
        for (int i = 0; i < 4; ++i) {
            s.frameCorners[i] = frame->frameCorners[i];
        }
    }

    sprite_listPush(&spriteMan->sprites, s);
}


void spriteManAddText (sprite_text textInfo) {
    char *currentLetter = textInfo.text;
    texture_asset *texAsset = getTexture(spriteMan->assetMan, textInfo.fontKey);
    ASSERT(texAsset != 0);
    f32 letterTexWidth = 8.0f / (f32)texAsset->width;
    f32 letterTexHeight = 8.0f / (f32)texAsset->height;

    f32 xOffset = 0.0f;
    f32 yOffset = 0.0f;
    while (*currentLetter != 0) {
        char letter = *currentLetter;

        if (letter == '\n') {
            xOffset = 0.0f;
            yOffset += 10.0f;
            ++currentLetter;
        }
        else {
            sprite letterSprite = defaultSprite();

            letterSprite.parentTransform = spriteManPeekMatrix();

            letterSprite.pos.x = textInfo.x + xOffset;
            letterSprite.pos.y = textInfo.y + yOffset;

            // TODO(ebuchholz): maybe pointer to texture info instead of copying?
            letterSprite.textureID = texAsset->id;
            letterSprite.width = 8.0f;//(float)texAsset->width;
            letterSprite.height = 8.0f;//(float)texAsset->height;

            bitmap_font_letter_coord *coord = &spriteMan->fontLetterCoords[letter];
            f32 coordX = coord->x / (f32)texAsset->width;
            f32 coordY = coord->y / (f32)texAsset->height;
            
            letterSprite.frameCorners[0] = (vec2){ .x = coordX, .y = coordY };
            letterSprite.frameCorners[1] = (vec2){ .x = coordX + letterTexWidth, .y = coordY };
            letterSprite.frameCorners[2] = (vec2){ .x = coordX, .y = coordY + letterTexHeight };
            letterSprite.frameCorners[3] = (vec2){ .x = coordX + letterTexWidth, .y = coordY + letterTexHeight };

            ++currentLetter;
            xOffset += coord->advance;

            sprite_listPush(&spriteMan->sprites, letterSprite);
        }
    }
}
