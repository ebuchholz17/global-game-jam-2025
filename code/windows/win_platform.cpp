#include <windows.h>
#include <xinput.h>

#include "win_platform.h"

#include "./win_dx_renderer.cpp"

#include "./directsound_audio.cpp"

static b32 gameRunning;

static b32 justResized = false;
static u32 windowWidth = 1280;
static u32 windowHeight = 720;

static b32 isFullscreen = false;
static b32 windowHidden = false;
static RECT windowedRect = {};

unsigned int numFilesToLoad;

LRESULT CALLBACK windowCallback (HWND window, u32 message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch (message) {
        case WM_CLOSE: {
            gameRunning = false;
        } break;
        case WM_SIZE: {
            RECT clientRect;
            GetClientRect(window, &clientRect);
            u32 newWidth = clientRect.right - clientRect.left;
            u32 newHeight = clientRect.bottom - clientRect.top;

            if (newWidth != windowWidth || newHeight != windowHeight) {
                justResized = true;
                windowWidth = newWidth;
                windowHeight = newHeight;

                windowHidden = (windowWidth == 0 || windowHeight == 0);
            }
        } break;
        default: {
            result = DefWindowProcA(window, message, wParam, lParam);
            break;
        }
    }
    return result;
}

static void processWindowsMessages (HWND window, game_input *input) {
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
        switch (message.message) {
            case WM_QUIT: {
                gameRunning = false;
            } break; 
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                unsigned int keyCode = (unsigned int)message.wParam;
                bool keyDown = (message.lParam & (1 << 31)) == 0;

                if (keyCode == 'A') {
                    if (!input->aKey.down && keyDown) {
                        input->aKey.justPressed = true;
                    }
                    input->aKey.down = keyDown;
                }
                else if (keyCode == 'S') {
                    if (!input->sKey.down && keyDown) {
                        input->sKey.justPressed = true;
                    }
                    input->sKey.down = keyDown;
                }
                else if (keyCode == 'Z') {
                    if (!input->zKey.down && keyDown) {
                        input->zKey.justPressed = true;
                    }
                    input->zKey.down = keyDown;
                }
                else if (keyCode == 'X') {
                    if (!input->xKey.down && keyDown) {
                        input->xKey.justPressed = true;
                    }
                    input->xKey.down = keyDown;
                }
                else if (keyCode == VK_UP) {
                    if (!input->upArrow.down && keyDown) {
                        input->upArrow.justPressed = true;
                    }
                    input->upArrow.down = keyDown;
                }
                else if (keyCode == VK_DOWN) {
                    if (!input->downArrow.down && keyDown) {
                        input->downArrow.justPressed = true;
                    }
                    input->downArrow.down = keyDown;
                }
                else if (keyCode == VK_LEFT) {
                    if (!input->leftArrow.down && keyDown) {
                        input->leftArrow.justPressed = true;
                    }
                    input->leftArrow.down = keyDown;
                }
                else if (keyCode == VK_RIGHT) {
                    if (!input->rightArrow.down && keyDown) {
                        input->rightArrow.justPressed = true;
                    }
                    input->rightArrow.down = keyDown;
                }

                if (keyDown) {
                    int altDown = (message.lParam & (1 << 29));
                    if((keyCode == VK_RETURN) && altDown) {
                        // Toggle fullscreen
                        isFullscreen = !isFullscreen;
                        if (isFullscreen) {
                            GetWindowRect(window, &windowedRect);
                            DWORD windowStyle = WS_OVERLAPPEDWINDOW & 
                                                ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | 
                                                  WS_MINIMIZEBOX | WS_MAXIMIZEBOX); 
                            SetWindowLongA(window, GWL_STYLE, windowStyle);

                            HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
                            MONITORINFOEX monitorInfo = {};
                            monitorInfo.cbSize = sizeof(MONITORINFOEX);
                            GetMonitorInfo(monitor, &monitorInfo);

                            SetWindowPos(window, HWND_TOPMOST,
                                         monitorInfo.rcMonitor.left,
                                         monitorInfo.rcMonitor.top,
                                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                                         SWP_FRAMECHANGED | SWP_NOACTIVATE);

                            ShowWindow(window, SW_MAXIMIZE);
                        }
                        else {
                            SetWindowLongA(window, GWL_STYLE, WS_OVERLAPPEDWINDOW);

                            SetWindowPos(window, HWND_NOTOPMOST,
                                         windowedRect.left,
                                         windowedRect.top,
                                         windowedRect.right - windowedRect.left,
                                         windowedRect.bottom - windowedRect.top,
                                         SWP_FRAMECHANGED | SWP_NOACTIVATE);

                            ShowWindow(window, SW_NORMAL);
                        }
                    }
                    else if((keyCode == VK_F4) && altDown) {
                        gameRunning = false;
                    }
                }
            } break;
            case WM_MOUSEMOVE: {
                POINT newMousePos;
                GetCursorPos(&newMousePos);
                ScreenToClient(window, &newMousePos);
                input->pointerX = newMousePos.x;
                input->pointerY = newMousePos.y;
            } break;
            case WM_LBUTTONDOWN: {
                if (!input->pointerDown) {
                    input->pointerJustDown = true;
                }
                input->pointerDown = true;
            } break;
            case WM_LBUTTONUP: {
                input->pointerDown = false;
            } break;
            default: {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }
}

static int stringLength (char *string) {
    int length = 0;
    while (*string++) {
        ++length;
    }
    return length;
}

static void concatStringsInBuffer (char *a, int aLength, char *b, int bLength, 
                                   char *destination, int destinationLength) 
{
    if (aLength + bLength <= destinationLength) {
        for (int i = 0; i < aLength; ++i) {
            *destination++ = *a++;
        }
        for (int i = 0; i < bLength; ++i) {
            *destination++ = *b++;
        }
        *destination = 0;
    }
    else {
        // error
    }
}


game_api loadGameDLL () {
    char exePath[MAX_PATH];
    DWORD exePathSize = GetModuleFileNameA(0, exePath, sizeof(exePath));

    char *exeDirectoryPlusOne = exePath;
    for (char *cursor = exePath; *cursor; ++cursor) {
        if (*cursor == '\\') {
            exeDirectoryPlusOne = cursor + 1;
        }
    }

    char gameDLLPath[MAX_PATH];
    char *dllName = "gng.dll";
    concatStringsInBuffer(exePath, exeDirectoryPlusOne - exePath, 
            dllName, stringLength(dllName),
            gameDLLPath, sizeof(gameDLLPath));
    HMODULE gameCode = LoadLibraryA(gameDLLPath);

    game_api gameAPI = {};
    gameAPI.updateGNGGame = (update_gng_game *)GetProcAddress(gameCode, "updateGNGGame");
    ASSERT(gameAPI.updateGNGGame != 0);
    gameAPI.getSoundSamplesGNGGame = (get_sound_samples_gng_game *)GetProcAddress(gameCode, "getSoundSamplesGNGGame");
    ASSERT(gameAPI.getSoundSamplesGNGGame != 0);
    return gameAPI;
}

void consoleLog (char *msg) {
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
}

LONGLONG rngSeedFromTime () {
    FILETIME fileTime; 
    GetSystemTimeAsFileTime(&fileTime);

    LONGLONG result = (LONGLONG)fileTime.dwLowDateTime + ((LONGLONG)fileTime.dwHighDateTime << 32LL);
    return result;
}

void readFile (char *id, char *path) {
    ASSERT(numFilesToLoad < WIN_FILES_TO_LOAD_MAX);

    win_file_to_load file = {};
    file.id = id;
    file.path = path;
    filesToLoad[numFilesToLoad++] = file;
}

// TODO: replace with accurate function that accounts for infinity+NaN
void floatToStringLossy(f32 num, char *buffer, u32 bufferLength) {
    i32 intPart = (i32)num;
    f32 floatPart = num - (f32)intPart;

    char *cursor = buffer;
    if (num < 0.0f) {
        *cursor++ = '-';
    }

    char *intStart = cursor;
    do {
        *cursor++ = '0' + intPart % 10;
        intPart /= 10;
    } while (intPart != 0 && cursor - buffer < bufferLength);

    char *intEnd = cursor - 1; 

    while (intEnd > intStart) {
        char temp = *intEnd;
        *intEnd = *intStart;
        *intStart = temp;
        ++intStart;
        --intEnd;
    }

    if (cursor - buffer < bufferLength) {
        *cursor++ = '.';

        // just do 3 digits
        for (u32 fracIndex = 0; fracIndex < 3 && cursor - buffer < bufferLength; ++fracIndex) {
            floatPart *= 10.0f;
            i32 digit = (i32)floatPart;
            floatPart -= (f32)digit;
            *cursor++ = '0' + digit;
        }
    }

    *cursor = 0;
}

void updateControllerButton (XINPUT_GAMEPAD gamepad, input_key *button, WORD buttonBitmask) {
    button->justPressed = false;
    if (gamepad.wButtons & buttonBitmask) {
        if (!button->down) {
            button->justPressed = true;
        }
        button->down = true;
    }
    else {
        button->down = false;
    }
}

void updateControllerTrigger (float *trigger, input_key *triggerButton, BYTE triggerValue, BYTE triggerMax) {
    triggerButton->justPressed = false;
    if (triggerValue >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
        triggerValue -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
        *trigger = (float)((float)triggerValue / (float)triggerMax);
        if (!triggerButton->down) {
            triggerButton->justPressed = true;
        }
        triggerButton->down = true;
    }
    else {
        *trigger = 0.0f;
        triggerButton->down = false;
    }

}

float winSqrtf (float num) {
    if (num < 0.0f) {
        return -1.0f; // error
    }
    float error = 0.00000001f * num;

    float estimate = num;
    float test;
    do {
        estimate = (num / estimate + estimate) / 2.0f;
        test = estimate - (num / estimate);
        if (test < 0) { test = -test; }
    } while (test > error);
    return estimate;
}

void updateControllerStick (SHORT gamepadStickX, SHORT gamepadStickY, float *stickX, float *stickY, SHORT deadzone) {
    float floatStickX = (float)gamepadStickX;
    float floatStickY = -(float)gamepadStickY;
    float magnitude = winSqrtf(floatStickX * floatStickX + floatStickY * floatStickY);
    float normalizedSX = floatStickX / magnitude;
    float normalizedSY = floatStickY / magnitude;

    if (magnitude > (float)deadzone) {
        if (magnitude > 32767.0f) {
            magnitude = 32767.0f;
        }
        magnitude -= deadzone;
        float normalizedMagnitude = magnitude / (32767.0f - deadzone);
        *stickX = normalizedSX * normalizedMagnitude;
        *stickY = normalizedSY * normalizedMagnitude;
    }
    else {
        *stickX = 0.0f;
        *stickY = 0.0f;
    }
}

void updateControllerStickDirection (input_key *dir0, input_key *dir1, SHORT gamepadVal, SHORT deadzone) {
    dir0->justPressed = false;
    if (gamepadVal < -deadzone) {
        if (!dir0->down) {
            dir0->justPressed = true;
        }
        dir0->down = true;
    }
    else {
        dir0->down = false;
    }
    dir1->justPressed = false;
    if (gamepadVal > deadzone) {
        if (!dir1->down) {
            dir1->justPressed = true;
        }
        dir1->down = true;
    }
    else {
        dir1->down = false;
    }
}

void updateController (game_controller_input *controller, XINPUT_STATE state) {
    XINPUT_GAMEPAD gamepad = state.Gamepad;
    updateControllerButton(gamepad, &controller->dPadUp, XINPUT_GAMEPAD_DPAD_UP);
    updateControllerButton(gamepad, &controller->dPadDown, XINPUT_GAMEPAD_DPAD_DOWN);
    updateControllerButton(gamepad, &controller->dPadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
    updateControllerButton(gamepad, &controller->dPadRight, XINPUT_GAMEPAD_DPAD_RIGHT);
    updateControllerButton(gamepad, &controller->start, XINPUT_GAMEPAD_START);
    updateControllerButton(gamepad, &controller->back, XINPUT_GAMEPAD_BACK);
    updateControllerButton(gamepad, &controller->leftStick, XINPUT_GAMEPAD_LEFT_THUMB);
    updateControllerButton(gamepad, &controller->rightStick, XINPUT_GAMEPAD_RIGHT_THUMB);
    updateControllerButton(gamepad, &controller->leftBumper, XINPUT_GAMEPAD_LEFT_SHOULDER);
    updateControllerButton(gamepad, &controller->rightBumper, XINPUT_GAMEPAD_RIGHT_SHOULDER);
    updateControllerButton(gamepad, &controller->aButton, XINPUT_GAMEPAD_A);
    updateControllerButton(gamepad, &controller->bButton, XINPUT_GAMEPAD_B);
    updateControllerButton(gamepad, &controller->xButton, XINPUT_GAMEPAD_X);
    updateControllerButton(gamepad, &controller->yButton, XINPUT_GAMEPAD_Y);

    BYTE triggerMax = 255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
    updateControllerTrigger(&controller->leftTrigger, &controller->leftTriggerButton, gamepad.bLeftTrigger, triggerMax);
    updateControllerTrigger(&controller->rightTrigger, &controller->rightTriggerButton, gamepad.bRightTrigger, triggerMax);

    updateControllerStick(gamepad.sThumbLX, gamepad.sThumbLY, &controller->leftStickX, &controller->leftStickY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    updateControllerStick(gamepad.sThumbRX, gamepad.sThumbRY, &controller->rightStickX, &controller->rightStickY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

    updateControllerStickDirection(&controller->leftStickUp, &controller->leftStickDown, -gamepad.sThumbLY, 20000);
    updateControllerStickDirection(&controller->leftStickLeft, &controller->leftStickRight, gamepad.sThumbLX, 20000);
    updateControllerStickDirection(&controller->rightStickUp, &controller->rightStickDown, -gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    updateControllerStickDirection(&controller->rightStickLeft, &controller->rightStickRight, gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
}

void loadXInput () {
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary) {
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    if(!XInputLibrary) {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }

    // TODO(ebuchholz): replace with error-handling that doesn't crash program
    if(XInputLibrary) {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) { 
            ASSERT(0);
        }
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) { 
            ASSERT(0);
        }
    }
    else {
        ASSERT(0); // couldn't load xinput
    }
}

i32 WINAPI WinMain (HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, i32 showCode) {
    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    i64 perfCountFrequency = perfCountFrequencyResult.QuadPart;

    // 100% pixel scaling
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSA windowClass = {0};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = windowCallback;
    windowClass.lpszClassName = "GNGClass";

    if (!RegisterClassA(&windowClass)) {
        return 1;
    }

    RECT targetWindowSize;
    targetWindowSize.left = 0;
    targetWindowSize.top = 0;
    targetWindowSize.right = windowWidth;
    targetWindowSize.bottom = windowHeight;

    DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE; 
    AdjustWindowRectEx(&targetWindowSize, windowStyle, false, WS_EX_APPWINDOW);

    HWND window = CreateWindowExA(
        WS_EX_APPWINDOW, 
        windowClass.lpszClassName, 
        "GNG", 
        windowStyle,
        0, 
        0, 
        targetWindowSize.right - targetWindowSize.left, 
        targetWindowSize.bottom - targetWindowSize.top, 
        0, 
        0,
        instance, 
        0
    );

    if (!window) {
        return 2;
    }

    game_api gameAPI = loadGameDLL();

    dx_renderer renderer = {};
    initRenderer(&renderer, window);

    // NOTE: virtualalloc memory guaranteed to be zeroed
    // Platform API (screen size, platform options, etc) + main game memory
    plat_api platAPI = {};
    platAPI.consoleLog = (plat_console_log *)consoleLog;
    platAPI.loadFile = (plat_load_file *)readFile;
    platAPI.rngSeedFromTime = (plat_rng_seed_from_time *)rngSeedFromTime;
    platAPI.mainMemorySize = 5 * 1024 * 1024;
    platAPI.mainMemory = VirtualAlloc(0, platAPI.mainMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    platAPI.assetMemorySize = 10 * 1024 * 1024;
    platAPI.assetMemory = VirtualAlloc(0, platAPI.assetMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    platAPI.scratchMemorySize = 5 * 1024 * 1024;
    platAPI.scratchMemory = VirtualAlloc(0, platAPI.scratchMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    // Memory for renderer
    mem_arena renderMemory = {};
    renderMemory.capacity = 10 * 1024 * 1024;
    renderMemory.base = VirtualAlloc(0, renderMemory.capacity, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    renderMemory.current = renderMemory.base;

    // init sound
    win_sound_output soundOutput = {};
    soundOutput.samplesPerSecond = 44100;
    soundOutput.bytesPerSample = 2;
    soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;
    directSoundInit(window, &soundOutput);
    directSoundClearSecBuffer(&soundOutput);
    soundOutput.secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    soundOutput.soundValid = false;

    sound_sample *soundSamples = (sound_sample *)VirtualAlloc(0, 2 * soundOutput.samplesPerSecond * sizeof(sound_sample), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    ASSERT(soundSamples);

    loadXInput();

    game_input input = {};

    LARGE_INTEGER lastCounter;
    QueryPerformanceCounter(&lastCounter);

    LARGE_INTEGER lastGameUpdateCounter;
    QueryPerformanceCounter(&lastGameUpdateCounter);

    LARGE_INTEGER lastControllerCheckTime = lastCounter;
    bool shouldCheckForNewControllers = true;

    gameRunning = true;
    while (gameRunning) {
        processWindowsMessages(window, &input);

        LARGE_INTEGER workCounter;
        QueryPerformanceCounter(&workCounter);

        LARGE_INTEGER controllerCheckTime;
        QueryPerformanceCounter(&controllerCheckTime);

        const unsigned int MAX_FILES_LOADED_PER_FRAME = 10000;
        plat_loaded_file loadedFiles[MAX_FILES_LOADED_PER_FRAME] = {};
        unsigned int numLoadedFiles = 0;

        // TODO(ebuchholz): threaded way to load files? need to figure out equivalent for web build
        if (numFilesToLoad > 0) {
            LARGE_INTEGER fileReadStartCounter;
            QueryPerformanceCounter(&fileReadStartCounter);
            float fileReadElapsed = 0.0f;

            while (numFilesToLoad > 0 && 
                   numLoadedFiles < MAX_FILES_LOADED_PER_FRAME && 
                   fileReadElapsed < 0.010f) 
            {
                win_file_to_load file = filesToLoad[0];

                HANDLE fileHandle = CreateFileA(file.path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                ASSERT(fileHandle != INVALID_HANDLE_VALUE);

                LARGE_INTEGER fileSize; 
                GetFileSizeEx(fileHandle, &fileSize);
                unsigned long long fileSizeToAlloc = fileSize.QuadPart;
                DWORD bytesRead = 0;

                plat_loaded_file *platLoadedFile = &loadedFiles[numLoadedFiles++];

                platLoadedFile->id = file.id;
                platLoadedFile->data = VirtualAlloc(0, fileSizeToAlloc, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                platLoadedFile->dataSize = fileSizeToAlloc;

                bool readSuccess = ReadFile(fileHandle, platLoadedFile->data, fileSizeToAlloc, &bytesRead, 0);
                ASSERT(readSuccess);
                ASSERT(bytesRead == fileSizeToAlloc);

                CloseHandle(fileHandle);

                filesToLoad[0] = filesToLoad[numFilesToLoad-- - 1];

                LARGE_INTEGER currentFileReadCounter;
                QueryPerformanceCounter(&currentFileReadCounter);
                long long fileReadCounterElapsed = currentFileReadCounter.QuadPart - fileReadStartCounter.QuadPart;
                fileReadElapsed = (float)fileReadCounterElapsed / (float)perfCountFrequency;
            }
        }

        // Set up platform api
        renderMemory.current = renderMemory.base;
        platAPI.windowWidth = windowWidth;
        platAPI.windowHeight = windowHeight;
        platAPI.audioSampleRate = soundOutput.samplesPerSecond;

        // Resize renderer buffers/etc. if window was just resized
        if (justResized && !windowHidden) {
            rendererResize(&renderer, windowWidth, windowHeight);
        }
        justResized = false;

        long long controllerTimeElapsed = controllerCheckTime.QuadPart - lastControllerCheckTime.QuadPart;
        float elapsedControllerSeconds = (float)controllerTimeElapsed / (float)perfCountFrequency;
        if (elapsedControllerSeconds >= 3.0f) {
            shouldCheckForNewControllers = true;
            lastControllerCheckTime = controllerCheckTime;
        }
        for (int i = 0; i < 4; ++i) {
            game_controller_input *controller = &input.controllers[i];
            if (controller->connected || shouldCheckForNewControllers) {
                XINPUT_STATE state = {};
                DWORD result = XInputGetState(i, &state);
                if (result == ERROR_SUCCESS) {
                    controller->connected = true;
                    updateController(controller, state);
                }
                else {
                    controller->connected = false;
                }
            }
        }
        if (shouldCheckForNewControllers) {
            shouldCheckForNewControllers = false;
        }


        LARGE_INTEGER currentGameUpdateCounter;
        QueryPerformanceCounter(&currentGameUpdateCounter);
        long long counterElapsed = currentGameUpdateCounter.QuadPart - lastGameUpdateCounter.QuadPart;
        float elapsed = (float)counterElapsed / (float)perfCountFrequency;
        lastGameUpdateCounter = currentGameUpdateCounter;

        // update game
        gameAPI.updateGNGGame(platAPI, &renderMemory, &input, elapsed, numLoadedFiles, loadedFiles);

        // Play sound
        LARGE_INTEGER audioStartTime;
        QueryPerformanceCounter(&audioStartTime);
        float timeFromFrameBeginning = ((float)(audioStartTime.QuadPart - lastCounter.QuadPart) /
                                        (float)perfCountFrequency);
        int numSoundSampleBytes;
        DWORD byteToLock;
        directSoundGetNumSoundSamples(&soundOutput, &numSoundSampleBytes, &byteToLock, timeFromFrameBeginning);
        if (numSoundSampleBytes > 0) {
            game_sound_output gameSoundOutput = {};
            gameSoundOutput.samplesPerSecond = soundOutput.samplesPerSecond;
            gameSoundOutput.sampleCount = numSoundSampleBytes / soundOutput.bytesPerSample;
            gameSoundOutput.samples = soundSamples;

            gameAPI.getSoundSamplesGNGGame(platAPI, &gameSoundOutput);
            directSoundFillSecBuffer(&soundOutput, byteToLock, numSoundSampleBytes, &gameSoundOutput);
        }

        if (!windowHidden){
            renderGame(&renderer, &renderMemory);
        }

        for (int loadedFileIndex = 0; loadedFileIndex < numLoadedFiles; loadedFileIndex++) {
            plat_loaded_file *platLoadedFile = &loadedFiles[loadedFileIndex];
            VirtualFree(platLoadedFile->data, 0, MEM_RELEASE);
        }

        //long long counterElapsed = workCounter.QuadPart - lastCounter.QuadPart;
        //float elapsed = (float)counterElapsed / (float)perfCountFrequency;
        QueryPerformanceCounter(&lastCounter);
    }

    rendererFlush(&renderer);
    shutdownRenderer(&renderer);

    return 0;
}
