#include "gng_bool.h"
#include "gng_audio.h"

sound_man *soundMan;

void soundManInit(sound_man *sm, asset_man *assetMan) {
    soundMan = sm;
    soundMan->assetMan = assetMan;
}

void soundManPlaySound (char *key) {
    for (u32 i = 0; i < SOUND_MAN_MAX_PLAYING_SOUNDS; i++) {
        playing_sound *playingSound = &soundMan->playingSounds[i];
        if (!playingSound->active) {
            playingSound->active = true;
            sound_asset *soundAsset = getSound(soundMan->assetMan, key);
            playingSound->key = soundAsset->key; // use the pointer assigned to the sound asset, which is permanent
            playingSound->currentSampleIndex = 0;
            break;
        }
    }
}
