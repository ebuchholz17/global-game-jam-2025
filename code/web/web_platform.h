#ifndef WEB_PLATFORM_H
#define WEB_PLATFORM_H

#include "../game/gng_types.h"

// tag to export specific functions to wasm
#define WASM_EXPORT __attribute__((visibility("default")))

#define MAX_LOADED_FILES_PER_FRAME 10000

// imports from JS
u32 getMemCapacity();
void consoleLog(char *);
void readFile(char *, char *);
u64 rngSeedFromTime(void);
void onError(char *);
void rendererResize(u32 windowWidth, u32 windowHeight);

#endif
