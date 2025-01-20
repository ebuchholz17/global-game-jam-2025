#ifndef GNG_AUDIO_H
#define GNG_AUDIO_H

#include "gng_types.h"
#include "gng_assets.h"

typedef struct playing_sound {
    b32 active;
    char *key;
    u32 currentSampleIndex;
} playing_sound;

#define SOUND_MAN_MAX_PLAYING_SOUNDS 30
typedef struct sound_man {
    playing_sound playingSounds[SOUND_MAN_MAX_PLAYING_SOUNDS];
    asset_man *assetMan;
} sound_man;

#endif
