#ifndef _UTILS_H_
#define _UTILS_H_

#include "stdlib.h"

#define _UNUSED_1(arg) (void) arg
#define _UNUSED_2(arg1, arg2) _UNUSED_1(arg1); _UNUSED_1(arg2)
#define _UNUSED_3(arg1, arg2, arg3) _UNUSED_2(arg1, arg2); _UNUSED_1(arg3)
#define _CONCAT(a, b) a ## b
#define _UNUSED_COUNT(PREFIX, _1, _2, _3, NUM, ...) _CONCAT(PREFIX, NUM)
#define UNUSED(...) _UNUSED_COUNT(_UNUSED_,__VA_ARGS__, 3, 2, 1)(__VA_ARGS__)

#define TODO(msg) do {\
        fprintf(stderr, "TODO::%s %s\n", __PRETTY_FUNCTION__, msg); \
        exit(-1); \
    } while(0)

#endif/*_UTILS_H_*/
