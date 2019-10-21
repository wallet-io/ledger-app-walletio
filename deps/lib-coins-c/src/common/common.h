#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef MEMMOVE
    #define MEMMOVE(target, src, len) memmove(target, src, len)
#endif

#ifndef MEMSET
    #define MEMSET(target, c, count) memset(target, c, count)
#endif

#ifndef PRINT
    #define PRINT(s, ...) printf((s), ##__VA_ARGS__)
#endif

#endif