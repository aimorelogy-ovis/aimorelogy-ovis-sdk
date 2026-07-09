#ifndef __HAL_DEBUG_H__
#define __HAL_DEBUG_H__
#include <stdint.h>
#include <stdio.h>

typedef enum {
    HAL_OK          = 0,
    HAL_ERROR       = -1,
    HAL_BUSY        = -2,
    HAL_TIMEOUT     = -3,
    HAL_UNSUPPORTED = -4,
} hal_error_t;

#define HAL_PARAM_CHK(para, err)                          \
    do {                                                  \
        if ((unsigned long)para == (unsigned long)NULL) { \
            return (err);                                 \
        }                                                 \
    } while (0)

#define HAL_PARAM_CHK_NORETVAL(para)                      \
    do {                                                  \
        if ((unsigned long)para == (unsigned long)NULL) { \
            return;                                       \
        }                                                 \
    } while (0)

#endif