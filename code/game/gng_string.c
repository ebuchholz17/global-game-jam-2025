#include "gng_string.h"
#include "gng_bool.h"
#include "gng_memory.h"

mem_arena tempStringMemory;

b32 stringEquals (char *a, char *b) {
    if (a == 0 && b == 0) {
        return true;
    }
    else if (a == 0 && b != 0 || a != 0 && b == 0) {
        return false;
    }
    else {
        u32 index = 0;
        while (a[index] != 0 && b[index] != 0) {
            if (a[index] != b[index]) {
                return false;
            }
            index++;
        }

        if (a[index] == 0 && b[index] == 0) {
            return true;
        }

        return false;
    }
}

i32 stringLength (char *s) {
    i32 length = 0;
    char *cursor = s;
    while (*cursor != 0) {
        length++;
        cursor++;
    }
    return length;
}

char *tempStringAppend (char *first, char *second) {
    char *a = first;
    char *b = second;

    u32 firstLength = 0;
    u32 secondLength = 0;

    while (*a != 0) {
        ++a;
        ++firstLength;
    }
    while (*b != 0) {
        ++b;
        ++secondLength;
    }

    char *result = (char *)allocMemory(&tempStringMemory, firstLength + secondLength + 1);
    char *currentLetter = result;
    for (u32 i = 0; i < firstLength; ++i) {
        *currentLetter = first[i];
        ++currentLetter;
    }
    for (u32 i = 0; i < secondLength; ++i) {
        *currentLetter = second[i];
        ++currentLetter;
    }
    *currentLetter = 0;

    return result;
}

char *tempStringFromI32 (i32 num) {
    i32 maxStringLength = 20;
    char numberBuffer[20] = {0};

    char *currentDigit = numberBuffer + (maxStringLength - 1);
    char *stringEnd = currentDigit;

    b32 isNegative = false;
    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    do {
        --currentDigit;
        *currentDigit = '0' + num % 10;
        num /= 10;
    } while (num != 0);

    if (isNegative) {
        --currentDigit;
        *currentDigit = '-';
    }

    u32 stringLength = (u32)(stringEnd - currentDigit);
    char *string = (char *)allocMemory(&tempStringMemory, stringLength + 1);

    for (u32 i = 0; i < stringLength; ++i) {
        string[i] = *currentDigit;
        ++currentDigit;
    }
    string[stringLength] = 0;

    return string;
}

char *stringFromI32 (i32 num, char *buffer, u32 bufferLength) {
    i32 maxStringLength = 20;
    char numberBuffer[20] = {0};

    char *currentDigit = numberBuffer + (maxStringLength - 1);
    char *stringEnd = currentDigit;

    b32 isNegative = false;
    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    do {
        --currentDigit;
        *currentDigit = '0' + num % 10;
        num /= 10;
    } while (num != 0);

    if (isNegative) {
        --currentDigit;
        *currentDigit = '-';
    }

    u32 stringLength = (u32)(stringEnd - currentDigit);

    for (u32 i = 0; i < stringLength && i < bufferLength; ++i) {
        buffer[i] = *currentDigit;
        ++currentDigit;
    }
    u32 minLength = stringLength < bufferLength ? stringLength : bufferLength;
    buffer[minLength] = 0;
    return buffer;
}

char *stringFromU32 (i32 num, char *buffer, u32 bufferLength) {
    i32 maxStringLength = 20;
    char numberBuffer[20] = {0};

    char *currentDigit = numberBuffer + (maxStringLength - 1);
    char *stringEnd = currentDigit;

    do {
        --currentDigit;
        *currentDigit = '0' + num % 10;
        num /= 10;
    } while (num != 0);

    u32 stringLength = (u32)(stringEnd - currentDigit);

    for (u32 i = 0; i < stringLength && i < bufferLength; ++i) {
        buffer[i] = *currentDigit;
        ++currentDigit;
    }
    u32 minLength = stringLength < bufferLength ? stringLength : bufferLength;
    buffer[minLength] = 0;
    return buffer;
}
