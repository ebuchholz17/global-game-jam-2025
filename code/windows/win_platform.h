#ifndef WIN_PLATFORM_H
#define WIN_PLATFORM_H

#include "../game/gng_types.h"
#include "../game/gng_platform.h"

#define WIN_FILES_TO_LOAD_MAX 100

typedef struct win_file_to_load {
    char *id;
    char *path;
} win_file_to_load;

win_file_to_load filesToLoad[WIN_FILES_TO_LOAD_MAX];

// xinput functions
// NOTE(ebuchholz): avoiding redefinition of functions in xinput.h
typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE *pState);
static x_input_get_state *XInputGetState_;
#define XInputGetState XInputGetState_
typedef DWORD WINAPI x_input_set_state(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);
static x_input_set_state *XInputSetState_;
#define XInputSetState XInputSetState_

#endif
