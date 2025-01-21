#ifndef GNG_VIRTUAL_INPUT_H
#define GNG_VIRTUAL_INPUT_H

#include "gng_math.h"
#include "gng_platform.h"

typedef struct virtual_button {
    rect boundingBox;
    input_key button;
    b32 wasDown;
} virtual_button;

typedef struct virtual_input {
    virtual_button dPadUp;
    virtual_button dPadDown;
    virtual_button dPadLeft;
    virtual_button dPadRight;

    virtual_button topButton;
    virtual_button bottomButton;
    virtual_button leftButton;
    virtual_button rightButton;
} virtual_input;

#endif

