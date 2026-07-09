#include "sys_context.h"
#include "base_ctx.h"
#include "cvi_debug.h"
#include "cvi_errno.h"
#include "queue.h"
#include "stdio.h"
#include "sys.h"
#include <stdlib.h>

static struct sys_ctx_info ctx_info;
extern struct cvi_venc_vb_ctx venc_vb_ctx[VENC_MAX_CHN_NUM];

#if !CONFIG_DISABLE_VDEC
struct cvi_vdec_vb_ctx vdec_vb_ctx[VENC_MAX_CHN_NUM];
#endif

CVI_S32 sys_ctx_init(void)
{
    memset(&ctx_info, 0, sizeof(struct sys_ctx_info));
    ctx_info.sys_info.chip_id = 0xffffffff;

    return 0;
}

struct sys_ctx_info *sys_get_ctx(void)
{
    return &ctx_info;
}

VPSS_MODE_E sys_ctx_get_vpssmode(void)
{
    return ctx_info.mode_cfg.vpss_mode.enMode;
}

void *sys_ctx_get_sysinfo(void)
{
    return (void *)(&ctx_info.sys_info);
}