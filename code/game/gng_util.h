#ifndef GNG_UTIL_H
#define GNG_UTIL_H

#include "gng_types.h"

void zeroMemory (u8 *, u64);
void readLine (char *data, char **start, char **end);
i32 stringToI32 (char *start, char *end);
u32 stringToU32 (char *start, char *end);

#endif
