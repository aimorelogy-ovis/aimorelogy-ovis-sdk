#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Uvc(const char *file)
{
    APP_PARAM_UVC_HOST_CFG_S *pstHostCfg = app_ipcam_UvcHost_Param_Get();
    CVI_BOOL bUvcEnable = CVI_FALSE;
    char tmp_section[32] = {0};

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading UVC config ------------------> start \n");

    // What changed: Keep only host-owned parsing fields and remove legacy mode/device_out branch gating.
    // Previous behavior: Parser still converted [uvc_config].mode and used bHostMode to gate host enable.
    // Impact: Host parser is now purely host-owned; enable state depends on host-side switch only.

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "uvc_config");

    bUvcEnable = (ini_getl(tmp_section, "uvc_enable", 1, file) != 0) ? CVI_TRUE : CVI_FALSE;

    if (pstHostCfg == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "uvc host cfg storage is null\n");
        return CVI_FAILURE;
    }

    memset(pstHostCfg, 0, sizeof(*pstHostCfg));
    pstHostCfg->bEnable = bUvcEnable;

    ini_gets(tmp_section, "host_dev", "", pstHostCfg->szDevPath, sizeof(pstHostCfg->szDevPath), file);
    pstHostCfg->u32Width    = (CVI_U32)ini_getl(tmp_section, "host_width", 0, file);
    pstHostCfg->u32Height   = (CVI_U32)ini_getl(tmp_section, "host_height", 0, file);
    pstHostCfg->u32SkipFrame = (CVI_U32)ini_getl(tmp_section, "host_skip", 0, file);
    ini_gets(tmp_section, "host_pixfmt", "", pstHostCfg->szPixFmt, sizeof(pstHostCfg->szPixFmt), file);
    pstHostCfg->bFeedVpss = (ini_getl(tmp_section, "host_feed_vpss", 0, file) != 0) ? CVI_TRUE : CVI_FALSE;
    pstHostCfg->u32VpssGrp  = (CVI_U32)ini_getl(tmp_section, "host_vpss_grp", 0, file);

    APP_PROF_LOG_PRINT(LEVEL_INFO,
                       "uvc_enable:%d (host parser)\n",
                       (int)bUvcEnable);

    APP_PROF_LOG_PRINT(LEVEL_INFO,
                       "uvc host route enable:%d host_dev:%s host=%ux%u fmt=%s skip=%u feed_vpss=%d grp=%u\n",
                       (int)pstHostCfg->bEnable,
                       pstHostCfg->szDevPath,
                       pstHostCfg->u32Width,
                       pstHostCfg->u32Height,
                       pstHostCfg->szPixFmt,
                       pstHostCfg->u32SkipFrame,
                       pstHostCfg->bFeedVpss,
                       pstHostCfg->u32VpssGrp);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading UVC config ------------------> done \n\n");
    return CVI_SUCCESS;
}
