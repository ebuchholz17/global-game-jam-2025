#include "gng_util.h"
#include "gng_bool.h"

void zeroMemory (u8 *mem, u64 size) {
    // TODO: use more-optimized method
    for (u64 index = 0; index < size; ++index) {
        mem[index] = 0;
    }
}

void copyMemory (u8 *dest, u8 *src, u64 size) {
    for (u64 i = 0; i < size; ++i) {
        dest[i] = src[i];
    }
}

void readLine (char *data, char **start, char **end) {
    *start = data;
    u32 index = 0;
    while (data[index] != 0 && data[index] != '\n' && data[index] != '\r') {
        ++index;
    }
    *end = data + index;
}

void readToNextLine (char *data, char **start, char **end, char **nextLineStart) {
    *start = data;
    u32 index = 0;
    while (data[index] != 0 && data[index] != '\n' && data[index] != '\r') {
        ++index;
    }
    *end = data + index;

    if (**end == 0) {
        *nextLineStart = 0;
    }
    else {
        if (**end == '\r') {
            *nextLineStart = *end + 2;
        }
        else {
            *nextLineStart = *end + 1;
        }
    }
}

i32 stringToI32 (char *start, char *end) {
    i32 result = 0;
    b32 negative = false;
    if (*start == '-') {
        negative = true;
        ++start;
    }
    while (start <= end) {
        char digit = *start;
        result *= 10;
        result += (i32)(digit - '0');
        ++start;
    }
    if (negative) {
        result = -result;
    }
    return result;
}

u32 stringToU32 (char *start, char *end) {
    u32 result = 0;

    while (start <= end) {
        char digit = *start;
        result *= 10;
        result += (i32)(digit - '0');
        ++start;
    }

    return result;
}
