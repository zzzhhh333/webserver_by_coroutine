#ifndef NB_UTIL_H
#define NB_UTIL_H

#include "log.h"
#include <assert.h>

#define NB_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            NB_LOG_ERROR("Assertion failed:", msg); \
            assert(false); \
        } \
    } while(0)


#endif // NB_UTIL_H
