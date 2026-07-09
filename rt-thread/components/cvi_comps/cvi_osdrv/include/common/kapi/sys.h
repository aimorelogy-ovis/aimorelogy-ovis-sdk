#ifndef __SYS_H__
#define __SYS_H__

#include "base.h"
#include "sys/types.h"
#include "rtos_types.h"
#include <cvi_comm_sys.h>
#include <stdbool.h>

CVI_U32 sys_get_chipid(void);
CVI_U8 *sys_get_version(void);
VPSS_MODE_E sys_get_vpssmode(void);

#endif /* __SYS_H__ */
