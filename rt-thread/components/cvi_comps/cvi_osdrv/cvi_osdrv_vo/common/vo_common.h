/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __VO_COMMON_H__
#define __VO_COMMON_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "rtos_types.h"
// #include <aos/types.h>
#include <cvi_reg.h>
#include <vip_atomic.h>
#include <vip_spinlock.h>
#include <errno.h>
#include "cvi_debug.h"

#define VIP_ALIGNMENT 0x40
#define GOP_ALIGNMENT 0x10
#ifndef ALIGN
#define ALIGN(x, align) (((x) + (align) - 1) & ~((align) - 1))
#endif

enum vo_msg_pri {
	CVI_VO_ERR = 3,
	CVI_VO_WARN = 4,
	CVI_VO_NOTICE = 5,
	CVI_VO_INFO = 6,
	CVI_VO_DBG = 7,
};

#ifdef __cplusplus
}
#endif

#endif /* __VO_COMMON_H__ */
