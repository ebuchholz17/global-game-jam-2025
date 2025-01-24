#include "hitbox.h"

void loadHitboxData (asset_man *assetMan, char *atlasKey, void *fileData, char_anim_data *animationData) {
    char *start;
    char *end;
    char *nextLineStart;
    char *file = (char *)fileData;

    readToNextLine(file, &start, &end, &nextLineStart);

    u32 numFrames = stringToU32(start, end-1);
    animationData->numFrames = numFrames;

    start = nextLineStart;

    for (u32 i = 0; i < numFrames; ++i) {
        char_frame_data *frameData = &animationData->frames[i];

        char frameName[255] = {0};
        readToNextLine(start, &start, &end, &nextLineStart);
        char *frameNameStart = start;
        u32 numLettersInFrameName = 0;
        while (frameNameStart != end) {
            frameName[numLettersInFrameName] = *frameNameStart;
            ++numLettersInFrameName;
            ++frameNameStart;
        }
        frameName[numLettersInFrameName] = 0;

        atlas_frame *frame = getAtlasFrame(assetMan, atlasKey, frameName);
        frameData->frameKey = frame->key;
        //int mapIndex = (int)getAtlasFrameIndex(assetMan, ATLAS_KEY_GAME, frameName);
        //for (int j = 0; j < 500; ++j) {
        //    if (editor->atlasIndices[j] == mapIndex) {
        //        editor->frameAtlasIndices[i] = j;
        //        break;
        //    }
        //}
        start = nextLineStart;

        readToNextLine(start, &start, &end, &nextLineStart);
        int offsetX = stringToI32(start, end-1);
        frameData->xOffset = offsetX;
        start = nextLineStart;

        readToNextLine(start, &start, &end, &nextLineStart);
        int offsetY = stringToI32(start, end-1);
        frameData->yOffset = offsetY;
        start = nextLineStart;

        readToNextLine(start, &start, &end, &nextLineStart);
        int duration = stringToU32(start, end-1);
        frameData->duration = duration;
        start = nextLineStart;

        readToNextLine(start, &start, &end, &nextLineStart);
        int numHitboxes = stringToU32(start, end-1);
        frameData->numHitboxes = numHitboxes;
        start = nextLineStart;

        for (int hitboxIndex = 0; hitboxIndex < numHitboxes; ++hitboxIndex) {
            rect hitbox = {0};

            readToNextLine(start, &start, &end, &nextLineStart);
            int minX = stringToI32(start, end-1);
            hitbox.min.x = (float)minX;
            start = nextLineStart;

            readToNextLine(start, &start, &end, &nextLineStart);
            int minY = stringToI32(start, end-1);
            hitbox.min.y = (float)minY;
            start = nextLineStart;
            
            readToNextLine(start, &start, &end, &nextLineStart);
            int maxX = stringToI32(start, end-1);
            hitbox.max.x = (float)maxX;
            start = nextLineStart;

            readToNextLine(start, &start, &end, &nextLineStart);
            int maxY = stringToI32(start, end-1);
            hitbox.max.y = (float)maxY;
            start = nextLineStart;

            frameData->hitboxes[hitboxIndex] = hitbox;
        }

        readToNextLine(start, &start, &end, &nextLineStart);
        int numHurtboxes = stringToU32(start, end-1);
        frameData->numHurtboxes = numHurtboxes;
        start = nextLineStart;

        for (int hurtboxIndex = 0; hurtboxIndex < numHurtboxes; ++hurtboxIndex) {
            rect hurtbox = {0};

            readToNextLine(start, &start, &end, &nextLineStart);
            int minX = stringToI32(start, end-1);
            hurtbox.min.x = (float)minX;
            start = nextLineStart;

            readToNextLine(start, &start, &end, &nextLineStart);
            int minY = stringToI32(start, end-1);
            hurtbox.min.y = (float)minY;
            start = nextLineStart;
            
            readToNextLine(start, &start, &end, &nextLineStart);
            int maxX = stringToI32(start, end-1);
            hurtbox.max.x = (float)maxX;
            start = nextLineStart;

            readToNextLine(start, &start, &end, &nextLineStart);
            int maxY = stringToI32(start, end-1);
            hurtbox.max.y = (float)maxY;
            start = nextLineStart;

            frameData->hurtboxes[hurtboxIndex] = hurtbox;
        }
    }
}
