#include "web_platform.h"

// game code
#include "../game/gng_game.c"
#include "./web_memory.c"
#include "./webgl_renderer.c"

// TODO: seems like stack can clobber global variables easily. allocating a struct to hold all this
u32 windowWidth;
u32 windowHeight;

b32 justResized;

plat_api platAPI;
game_input input;

mem_arena renderMemory;
webgl_renderer wglr = {0};

plat_loaded_file loadedFiles[MAX_LOADED_FILES_PER_FRAME] = {0};
u32 numLoadedFiles;

b32 audioInitialized;
sound_sample *soundSamples;

// Replacements for functions that are called by compiler but not by us
void *memcpy (void *dest, const void *src, unsigned long size) {
    for (u32 i = 0; i < size; ++i) {
        ((u8 *)dest)[i] = ((u8 *)src)[i];
    }
    return dest;
}
void *memset (void *dest, i32 c, unsigned long size) {
    for (u32 i = 0; i < size; ++i) {
        ((u8 *)dest)[i] = (u8)c;
    }
    return dest;
}

WASM_EXPORT void initGame (void) {
   memCapacity = getMemCapacity();

   // reserve space at the beginning of memory for temp memory
   webAllocMemory(10 * 1024 * 1024);
   memStart = memCurrent; 

   u32 mainMemorySize = 10 * 1024 * 1024;
   u32 assetMemorySize = 16 * 1024 * 1024;
   u32 scratchMemorySize = 10 * 1024 * 1024;

   platAPI = (plat_api){
       .consoleLog = consoleLog,
       .loadFile = readFile,
       .rngSeedFromTime = rngSeedFromTime,
       .mainMemorySize = mainMemorySize,
       .mainMemory = webAllocMemory(mainMemorySize),
       .assetMemorySize = assetMemorySize,
       .assetMemory = webAllocMemory(assetMemorySize),
       .scratchMemorySize = scratchMemorySize,
       .scratchMemory = webAllocMemory(scratchMemorySize)
   };

   u32 renderMemorySize = 30 * 1024 * 1024;
   void *renderMemoryBase = webAllocMemory(renderMemorySize);

   renderMemory = (mem_arena){
       .base = renderMemoryBase,
       .current = renderMemoryBase,
       .capacity = renderMemorySize
   };

   input = (game_input){};
}

WASM_EXPORT void initAudio (u32 sampleRate) {
    platAPI.audioSampleRate = sampleRate;
    soundSamples = (sound_sample *)webAllocMemory(2 * sampleRate * 4);
    audioInitialized = true;
}

WASM_EXPORT void setVirtualInputEnabled (b32 enabled) {
    platAPI.hasTouchControls = true;
}

WASM_EXPORT void onMouseMove (u32 mouseX, u32 mouseY) {
    input.pointerX = mouseX;
    input.pointerY = mouseY;
}

WASM_EXPORT void onMouseDown () {
    if (!input.pointerDown) {
        input.pointerJustDown = true;
    }
    input.pointerDown = true;
}

WASM_EXPORT void onMouseUp () {
    input.pointerDown = false;
}

WASM_EXPORT void onLetterKeyDown (u32 letterCode) {
    switch (letterCode) {
        case 'a': {
            if (!input.aKey.down) {
                input.aKey.justPressed = true;
            }
            input.aKey.down = true;
        } break;
        case 's': {
            if (!input.sKey.down) {
                input.sKey.justPressed = true;
            }
            input.sKey.down = true;
        } break;
        case 'z': {
            if (!input.zKey.down) {
                input.zKey.justPressed = true;
            }
            input.zKey.down = true;
        } break;
        case 'x': {
            if (!input.xKey.down) {
                input.xKey.justPressed = true;
            }
            input.xKey.down = true;
        } break;
    }
}

WASM_EXPORT void onLetterKeyUp (u32 letterCode) {
    switch (letterCode) {
        case 'a': {
            input.aKey.down = false;
        } break;
        case 's': {
            input.sKey.down = false;
        } break;
        case 'z': {
            input.zKey.down = false;
        } break;
        case 'x': {
            input.xKey.down = false;
        } break;
    }
}

WASM_EXPORT void onArrowKeyDown (u32 arrowDir) {
    // 0: up, 1: down, 2: left, 3: right
    switch (arrowDir) {
        case 0: {
            if (!input.upArrow.down) {
                input.upArrow.justPressed = true;
            }
            input.upArrow.down = true;
        } break;
        case 1: {
            if (!input.downArrow.down) {
                input.downArrow.justPressed = true;
            }
            input.downArrow.down = true;
        } break;
        case 2: {
            if (!input.leftArrow.down) {
                input.leftArrow.justPressed = true;
            }
            input.leftArrow.down = true;
        } break;
        case 3: {
            if (!input.rightArrow.down) {
                input.rightArrow.justPressed = true;
            }
            input.rightArrow.down = true;
        } break;
    }
}

WASM_EXPORT void onArrowKeyUp (u32 arrowDir) {
    switch (arrowDir) {
        case 0: {
            input.upArrow.down = false;
        } break;
        case 1: {
            input.downArrow.down = false;
        } break;
        case 2: {
            input.leftArrow.down = false;
        } break;
        case 3: {
            input.rightArrow.down = false;
        } break;
    }
}

WASM_EXPORT void gameSetControllerConnected(u32 index, b32 connected) {
    input.controllers[index].connected = connected;
}

WASM_EXPORT void gameSetControllerIndexDPad(u32 index,
                                            b32 dPadUpDown, b32 dPadUpJustPressed,
                                            b32 dPadDownDown, b32 dPadDownJustPressed,
                                            b32 dPadLeftDown, b32 dPadLeftJustPressed,
                                            b32 dPadRightDown, b32 dPadRightJustPressed)
{

    input.controllers[index].dPadUp.down = dPadUpDown;
    input.controllers[index].dPadUp.justPressed = dPadUpJustPressed;
    input.controllers[index].dPadDown.down = dPadDownDown;
    input.controllers[index].dPadDown.justPressed = dPadDownJustPressed;
    input.controllers[index].dPadLeft.down = dPadLeftDown;
    input.controllers[index].dPadLeft.justPressed = dPadLeftJustPressed;
    input.controllers[index].dPadRight.down = dPadRightDown;
    input.controllers[index].dPadRight.justPressed = dPadRightJustPressed;
}

WASM_EXPORT void gameSetControllerIndexStartBack(u32 index,
                                                 b32 startDown, b32 startJustPressed,
                                                 b32 backDown, b32 backJustPressed)
{
    input.controllers[index].start.down = startDown;
    input.controllers[index].start.justPressed = startJustPressed;
    input.controllers[index].back.down = backDown;
    input.controllers[index].back.justPressed = backJustPressed;
}

WASM_EXPORT void gameSetControllerIndexStickButtons(u32 index,
                                                    b32 leftStickDown, b32 leftStickJustPressed,
                                                    b32 rightStickDown, b32 rightStickJustPressed)
{
    input.controllers[index].leftStick.down = leftStickDown;
    input.controllers[index].leftStick.justPressed = leftStickJustPressed;
    input.controllers[index].rightStick.down = rightStickDown;
    input.controllers[index].rightStick.justPressed = rightStickJustPressed;
}

WASM_EXPORT void gameSetControllerIndexBumpers(u32 index,
                                               b32 leftBumperDown, b32 leftBumperJustPressed,
                                               b32 rightBumperDown, b32 rightBumperJustPressed)
{
    input.controllers[index].leftBumper.down = leftBumperDown;
    input.controllers[index].leftBumper.justPressed = leftBumperJustPressed;
    input.controllers[index].rightBumper.down = rightBumperDown;
    input.controllers[index].rightBumper.justPressed = rightBumperJustPressed;
}

WASM_EXPORT void gameSetControllerIndexFaceButtons(u32 index,
                                                   b32 aButtonDown, b32 aButtonJustPressed,
                                                   b32 bButtonDown, b32 bButtonJustPressed,
                                                   b32 xButtonDown, b32 xButtonJustPressed,
                                                   b32 yButtonDown, b32 yButtonJustPressed)
{
    input.controllers[index].aButton.down = aButtonDown;
    input.controllers[index].aButton.justPressed = aButtonJustPressed;
    input.controllers[index].bButton.down = bButtonDown;
    input.controllers[index].bButton.justPressed = bButtonJustPressed;
    input.controllers[index].xButton.down = xButtonDown;
    input.controllers[index].xButton.justPressed = xButtonJustPressed;
    input.controllers[index].yButton.down = yButtonDown;
    input.controllers[index].yButton.justPressed = yButtonJustPressed;
}

WASM_EXPORT void gameSetControllerIndexTriggers(u32 index,
                                                b32 leftTrigger, 
                                                b32 leftTriggerButtonDown, b32 leftTriggerButtonJustPressed,
                                                b32 rightTrigger, 
                                                b32 rightTriggerButtonDown, b32 rightTriggerButtonJustPressed)
{
    input.controllers[index].leftTrigger = leftTrigger;
    input.controllers[index].leftTriggerButton.down = leftTriggerButtonDown;
    input.controllers[index].leftTriggerButton.justPressed = leftTriggerButtonJustPressed;
    input.controllers[index].rightTrigger = rightTrigger;
    input.controllers[index].rightTriggerButton.down = rightTriggerButtonDown;
    input.controllers[index].rightTriggerButton.justPressed = rightTriggerButtonJustPressed;
}

WASM_EXPORT void gameSetControllerIndexSticks(u32 index,
                                              b32 leftStickX,  b32 leftStickY,  
                                              b32 rightStickX,  b32 rightStickY)
{
    input.controllers[index].leftStickX = leftStickX;
    input.controllers[index].leftStickY = leftStickY;
    input.controllers[index].rightStickX = rightStickX;
    input.controllers[index].rightStickY = rightStickY;
}

WASM_EXPORT void gameSetControllerIndexStickDirections(u32 index,
                                                       b32 leftStickUpDown, b32 leftStickUpJustPressed,
                                                       b32 leftStickDownDown, b32 leftStickDownJustPressed,
                                                       b32 leftStickLeftDown, b32 leftStickLeftJustPressed,
                                                       b32 leftStickRightDown, b32 leftStickRightJustPressed,
                                                       b32 rightStickUpDown, b32 rightStickUpJustPressed,
                                                       b32 rightStickDownDown, b32 rightStickDownJustPressed,
                                                       b32 rightStickLeftDown, b32 rightStickLeftJustPressed,
                                                       b32 rightStickRightDown, b32 rightStickRightJustPressed)
{
    input.controllers[index].leftStickUp.down = leftStickUpDown;
    input.controllers[index].leftStickUp.justPressed = leftStickUpJustPressed;
    input.controllers[index].leftStickDown.down = leftStickDownDown;
    input.controllers[index].leftStickDown.justPressed = leftStickDownJustPressed;
    input.controllers[index].leftStickLeft.down = leftStickLeftDown;
    input.controllers[index].leftStickLeft.justPressed = leftStickLeftJustPressed;
    input.controllers[index].leftStickRight.down = leftStickRightDown;
    input.controllers[index].leftStickRight.justPressed = leftStickRightJustPressed;
    input.controllers[index].rightStickUp.down = rightStickUpDown;
    input.controllers[index].rightStickUp.justPressed = rightStickUpJustPressed;
    input.controllers[index].rightStickDown.down = rightStickDownDown;
    input.controllers[index].rightStickDown.justPressed = rightStickDownJustPressed;
    input.controllers[index].rightStickLeft.down = rightStickLeftDown;
    input.controllers[index].rightStickLeft.justPressed = rightStickLeftJustPressed;
    input.controllers[index].rightStickRight.down = rightStickRightDown;
    input.controllers[index].rightStickRight.justPressed = rightStickRightJustPressed;
}

WASM_EXPORT void setTouchInput (u32 touchIndex, b32 active, b32 down, b32 justPressed, u32 x, u32 y, f32 radiusX, f32 radiusY) {
    input.touches[touchIndex].active = active;
    input.touches[touchIndex].touchState.down = down;
    input.touches[touchIndex].touchState.justPressed = justPressed;
    input.touches[touchIndex].x = x;
    input.touches[touchIndex].y = y;
    input.touches[touchIndex].radiusX = radiusX;
    input.touches[touchIndex].radiusY = radiusY;
}

WASM_EXPORT void onResize (u32 newWidth, u32 newHeight) {
    windowWidth = newWidth;
    windowHeight = newHeight;
    justResized = true;
}

WASM_EXPORT void onFileFetched (char *assetName, void *fileData, u32 fileDataSize) {
    plat_loaded_file *platLoadedFile = &loadedFiles[numLoadedFiles++];

    platLoadedFile->id = assetName;
    platLoadedFile->data = fileData;
    platLoadedFile->dataSize = fileDataSize;
}

WASM_EXPORT void onFrameStart () {
   renderMemory.current = renderMemory.base;

   platAPI.windowWidth = windowWidth;
   platAPI.windowHeight = windowHeight;

   if (justResized) {
       rendererResize(windowWidth, windowHeight);
   }
   justResized = false;
}

WASM_EXPORT void updateGame (f32 dt) {
   updateGNGGame(platAPI, &renderMemory, &input, dt, numLoadedFiles, loadedFiles);
}

WASM_EXPORT void renderGame () {
   webglRenderGame(&wglr, &renderMemory);
}

WASM_EXPORT f32 *getGameSoundSamples (u32 numSamples) {
    if (audioInitialized) {
        game_sound_output gameSoundOutput = {};
        gameSoundOutput.samplesPerSecond = platAPI.audioSampleRate;
        gameSoundOutput.sampleCount = numSamples;
        gameSoundOutput.samples = soundSamples;

        getSoundSamplesGNGGame(platAPI, &gameSoundOutput);
        return (f32 *)soundSamples;
    }
    else {
        return 0;
    }
}

WASM_EXPORT void onFrameEnd (f32 dt) {
   numLoadedFiles = 0;
   webFreeTempMemory();
}
