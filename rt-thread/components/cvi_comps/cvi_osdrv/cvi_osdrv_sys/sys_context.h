#ifndef __SYS_CONTEXT_H__
#define __SYS_CONTEXT_H__

#include "base.h"
#include "sys/types.h"
// #include "k_atomic.h"
#include <cvi_comm_sys.h>

struct sys_info {
    char version[VERSION_NAME_MAXLEN];
    CVI_U32 chip_id;
};

struct sys_mode_cfg {
    VI_VPSS_MODE_S vivpss_mode;
    VPSS_MODE_S vpss_mode;
};

struct sys_ctx_info {
    struct sys_info sys_info;
    struct sys_mode_cfg mode_cfg;
    int sys_inited;
};

CVI_S32 sys_ctx_init(void);
struct sys_ctx_info *sys_get_ctx(void);

CVI_U32 sys_ctx_get_chipid(void);
CVI_U8 *sys_ctx_get_version(void);
void *sys_ctx_get_sysinfo(void);

VPSS_MODE_E sys_ctx_get_vpssmode(void);

#endif /* __SYS_CONTEXT_H__ */
