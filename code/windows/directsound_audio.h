#ifndef DIRECTSOUND_AUDIO_H
#define DIRECTSOUND_AUDIO_H

struct win_sound_output {
    int samplesPerSecond;
    unsigned int dataCursor;
    int bytesPerSample;
    int secondaryBufferSize;
    LPDIRECTSOUNDBUFFER secondaryBuffer;
    bool soundValid;
};

typedef HRESULT WINAPI direct_sound_create (LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

#endif
