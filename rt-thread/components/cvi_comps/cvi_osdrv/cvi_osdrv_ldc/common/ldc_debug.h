#ifndef _LDC_DEBUG_H_
#define _LDC_DEBUG_H_

#include "rtos_types.h"
#include <stdio.h>
#include "cvi_debug.h"


#define CVI_TRACE_LDC(level, fmt, ...) CVI_TRACE_GDC(level, fmt, ##__VA_ARGS__)


#endif /* _LDC_DEBUG_H_ */
