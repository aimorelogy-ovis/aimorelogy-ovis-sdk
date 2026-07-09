#include <stdbool.h>
#include <stdio.h>

#include "cvi_type.h"
#include "dsi_3aml069lp01g.h"
#include "dsi_gm8775c.h"
#include "dsi_hx8394_evb.h"
#include "dsi_hx8399_1080p.h"
#include "dsi_icn9707.h"
#include "dsi_ili9881c.h"
#include "dsi_ili9881d.h"
#include "dsi_jd9366ab.h"
#include "dsi_nt35521.h"
#include "dsi_ota7290b.h"
#include "dsi_ota7290b_1920.h"
#include "dsi_st7701.h"
#include "bt656_ms7024.h"
#include "lvds_lcm185x56.h"

#include "cvi_vo.h"

#include "app_ipcam_comm.h"
#include "app_ipcam_mipi_tx.h"
#include "app_ipcam_panel.h"
#include "app_ipcam_panel_i2c.h"
#include "app_ipcam_vo.h"

static CVI_BOOL app_ipcam_Panel_IsMs7024(const PANEL_TYPE_E enPanelType)
{
    switch (enPanelType) {
    case PANEL_BT656_MS7024_720x480_60:
    case PANEL_BT656_MS7024_720x480_30:
    case PANEL_BT656_MS7024_720x576_25:
    case PANEL_BT656_MS7024_720x576_50:
        return CVI_TRUE;
    default:
        return CVI_FALSE;
    }
}

static const CVI_CHAR *app_ipcam_Panel_Ms7024Name_Get(const PANEL_TYPE_E enPanelType)
{
    switch (enPanelType) {
    case PANEL_BT656_MS7024_720x480_60:
        return "MS7024-720x480-60";
    case PANEL_BT656_MS7024_720x480_30:
        return "MS7024-720x480-30";
    case PANEL_BT656_MS7024_720x576_25:
        return "MS7024-720x576-25";
    case PANEL_BT656_MS7024_720x576_50:
        return "MS7024-720x576-50";
    default:
        return "MS7024-unsupported";
    }
}

static VO_SYNC_INFO_S app_ipcam_Panel_Ms7024SyncInfo_Get(const PANEL_TYPE_E enPanelType)
{
    switch (enPanelType) {
    case PANEL_BT656_MS7024_720x480_30:
        return (VO_SYNC_INFO_S){.bSynm = 1, .bIop = 1, .u16FrameRate = 30,
            .u16Vact = 480, .u16Vbb = 30, .u16Vfb = 9,
            .u16Hact = 720, .u16Hbb = 60, .u16Hfb = 16,
            .u16Vpw = 6, .u16Hpw = 62, .bIdv = 0, .bIhs = 0, .bIvs = 0};
    case PANEL_BT656_MS7024_720x576_25:
        return (VO_SYNC_INFO_S){.bSynm = 1, .bIop = 1, .u16FrameRate = 25,
            .u16Vact = 576, .u16Vbb = 39, .u16Vfb = 5,
            .u16Hact = 720, .u16Hbb = 68, .u16Hfb = 12,
            .u16Vpw = 5, .u16Hpw = 64, .bIdv = 0, .bIhs = 0, .bIvs = 0};
    case PANEL_BT656_MS7024_720x576_50:
        return (VO_SYNC_INFO_S){.bSynm = 1, .bIop = 1, .u16FrameRate = 50,
            .u16Vact = 576, .u16Vbb = 39, .u16Vfb = 5,
            .u16Hact = 720, .u16Hbb = 68, .u16Hfb = 12,
            .u16Vpw = 5, .u16Hpw = 64, .bIdv = 0, .bIhs = 0, .bIvs = 0};
    case PANEL_BT656_MS7024_720x480_60:
    default:
        return (VO_SYNC_INFO_S){.bSynm = 1, .bIop = 1, .u16FrameRate = 60,
            .u16Vact = 480, .u16Vbb = 30, .u16Vfb = 9,
            .u16Hact = 720, .u16Hbb = 60, .u16Hfb = 16,
            .u16Vpw = 6, .u16Hpw = 62, .bIdv = 0, .bIhs = 0, .bIvs = 0};
    }
}

CVI_S32 app_ipcam_Panel_PanelDesc_Get(
    const PANEL_TYPE_E* const penPanelType,
    PANEL_DESC_T* const pstPanelDesc,
    APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(penPanelType, CVI_FAILURE);
    _NULL_POINTER_CHECK_(pstPanelDesc, CVI_FAILURE);

    switch(*penPanelType) {
        case PANEL_DSI_3AML069LP01G:
            pstPanelDesc->pchPanelName = "3AML069LP01G-600x1024";
            pstPanelDesc->pstDevCfg = &dev_cfg_3AML069LP01G_600x1024;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_3AML069LP01G_600x1024;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_3AML069LP01G_600x1024;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_3AML069LP01G_600x1024);
            break;

        case PANEL_DSI_GM8775C:
            pstPanelDesc->pchPanelName = "GM8775C";
            pstPanelDesc->pstDevCfg = &dev_cfg_gm8775c;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_gm8775c;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_gm8775c;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_gm8775c);
            break;

        case PANEL_DSI_HX8399_1080P:
            pstPanelDesc->pchPanelName = "HX8399_1080x1920";
            pstPanelDesc->pstDevCfg = &dev_cfg_hx8399_1080x1920;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_hx8399_1080x1920;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_hx8399_1080x1920;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_hx8399_1080x1920);
            break;

        case PANEL_DSI_ICN9707:
            pstPanelDesc->pchPanelName = "ICN9707-480x1920";
            pstPanelDesc->pstDevCfg = &dev_cfg_icn9707_480x1920;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_icn9707_480x1920;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_icn9707_480x1920;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_icn9707_480x1920);
            break;

        case PANEL_DSI_ILI9881C:
            pstPanelDesc->pchPanelName = "ILI9881C-720x1280";
            pstPanelDesc->pstDevCfg = &dev_cfg_ili9881c_720x1280;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_ili9881c_720x1280;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_ili9881c_720x1280;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_ili9881c_720x1280);
            break;

        case PANEL_DSI_ILI9881D:
            pstPanelDesc->pchPanelName = "ILI9881D-720x1280";
            pstPanelDesc->pstDevCfg = &dev_cfg_ili9881d_720x1280;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_ili9881d_720x1280;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_ili9881d_720x1280;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_ili9881d_720x1280);
            break;

        case PANEL_DSI_JD9366AB:
            pstPanelDesc->pchPanelName = "JD9366AB-800x1280";
            pstPanelDesc->pstDevCfg = &dev_cfg_jd9366ab_800x1280;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_jd9366ab_800x1280;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_jd9366ab_800x1280;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_jd9366ab_800x1280);
            break;

        // case PANEL_DSI_JD9852:
        //     pstPanelDesc->pchPanelName = "JD9852-240x320";
        //     pstPanelDesc->pstDevCfg = &dev_cfg_JD9852_240x320;
        //     pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_JD9852_240x320;
        //     pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_JD9852_320x480;
        //     pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_JD9852_320x480);
        //     break;

        // case PANEL_DSI_LT9611_1024x768_60:
        //     pstPanelDesc->pchPanelName = "LT9611-1024x768_60";
        //     pstPanelDesc->pstDevCfg = &dev_cfg_lt9611_1024x768_60Hz;
        //     pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_lt9611;
        //     pstPanelDesc->pstDsiInitCmds = NULL;
        //     pstPanelDesc->s32DsiInitCmdsSize = 0;
        //     break;

        // case PANEL_DSI_LT9611_1280x720_60:
        //     pstPanelDesc->pchPanelName = "LT9611-1280x720_60";
        //     pstPanelDesc->pstDevCfg = &dev_cfg_lt9611_1280x720_60Hz;
        //     pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_lt9611;
        //     pstPanelDesc->pstDsiInitCmds = NULL;
        //     pstPanelDesc->s32DsiInitCmdsSize = 0;
        //     break;

        // case PANEL_DSI_LT9611_1280x1024_60:
        //     pstPanelDesc->pchPanelName = "LT9611-1280x1024_60";
        //     pstPanelDesc->pstDevCfg = &dev_cfg_lt9611_1280x1024_60Hz;
        //     pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_lt9611;
        //     pstPanelDesc->pstDsiInitCmds = NULL;
        //     pstPanelDesc->s32DsiInitCmdsSize = 0;
        //     break;

        // case PANEL_DSI_LT9611_1600x1200_60:
        //     pstPanelDesc->pchPanelName = "LT9611-1600x1200_60";
        //     pstPanelDesc->pstDevCfg = &dev_cfg_lt9611_1600x1200_60Hz;
        //     pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_lt9611;
        //     pstPanelDesc->pstDsiInitCmds = NULL;
        //     pstPanelDesc->s32DsiInitCmdsSize = 0;
        //     break;

        // case PANEL_DSI_LT9611_1920x1080_30:
        //     pstPanelDesc->pchPanelName = "LT9611-1920x1080_30";
        //     pstPanelDesc->pstDevCfg = &dev_cfg_lt9611_1920x1080_30Hz;
        //     pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_lt9611;
        //     pstPanelDesc->pstDsiInitCmds = NULL;
        //     pstPanelDesc->s32DsiInitCmdsSize = 0;
        //     break;

        // case PANEL_DSI_LT9611_1920x1080_60:
        //     pstPanelDesc->pchPanelName = "LT9611-1920x1080_60";
        //     pstPanelDesc->pstDevCfg = &dev_cfg_lt9611_1920x1080_60Hz;
        //     pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_lt9611;
        //     pstPanelDesc->pstDsiInitCmds = NULL;
        //     pstPanelDesc->s32DsiInitCmdsSize = 0;
            break;

        case PANEL_DSI_NT35521:
            pstPanelDesc->pchPanelName = "NT35521-800x1280";
            pstPanelDesc->pstDevCfg = &dev_cfg_nt35521_800x1280;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_nt35521_800x1280;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_nt35521_800x1280;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_nt35521_800x1280);
            break;

        case PANEL_DSI_OTA7290B:
            pstPanelDesc->pchPanelName = "OTA7290B-320x1280";
            pstPanelDesc->pstDevCfg = &dev_cfg_ota7290b_320x1280;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_ota7290b_320x1280;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_ota7290b_320x1280;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_ota7290b_320x1280);
            break;

        case PANEL_DSI_OTA7290B_1920:
            pstPanelDesc->pchPanelName = "OTA7290B-440x1920";
            pstPanelDesc->pstDevCfg = &dev_cfg_ota7290b_440x1920;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_ota7290b_440x1920;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_ota7290b_440x1920;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_ota7290b_440x1920);
            break;

        case PANEL_DSI_ST7701:
            pstPanelDesc->pchPanelName = "ST7701-480x800";
            pstPanelDesc->pstDevCfg = &dev_cfg_st7701_480x800;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_st7701_480x800;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_st7701_480x800;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_st7701_480x800);
            break;

        // case PANEL_DSI_ST7785M:
        //     pstPanelDesc->pchPanelName = "ST7785M-240x320";
        //     pstPanelDesc->pstDevCfg = &dev_cfg_st7785m_240x320;
        //     pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_st7785m_240x320;
        //     pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_st7785m_240x320;
        //     pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_st7785m_240x320);
        //     break;

        // case PANEL_DSI_ST7796S:
        //     pstPanelDesc->pchPanelName = "ST7796S-320x480";
        //     pstPanelDesc->pstDevCfg = &dev_cfg_ST7796S_320x480;
        //     pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_ST7796S_320x480;
        //     pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_ST7796S_320x480;
        //     pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_ST7796S_320x480);
        //     break;

        case PANEL_BT656_MS7024_720x480_60:
        case PANEL_BT656_MS7024_720x480_30:
        case PANEL_BT656_MS7024_720x576_25:
        case PANEL_BT656_MS7024_720x576_50:
            pstPanelDesc->pchPanelName = app_ipcam_Panel_Ms7024Name_Get(*penPanelType);
            pstPanelDesc->pstDevCfg = NULL;
            pstPanelDesc->pstHsTimingCfg = NULL;
            pstPanelDesc->pstDsiInitCmds = NULL;
            pstPanelDesc->s32DsiInitCmdsSize = 0;
            if (pstVoCfg) {
                pstVoCfg->stVoPubAttr.enIntfType = VO_INTF_BT656;
                pstVoCfg->stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
                pstVoCfg->stVoPubAttr.stSyncInfo = app_ipcam_Panel_Ms7024SyncInfo_Get(*penPanelType);
            }
            break;
        case PANEL_LVDS_LCM185X56:
            pstPanelDesc->pchPanelName = "LCM185X56-1366x768";
            pstPanelDesc->pstDevCfg = NULL;
            pstPanelDesc->pstHsTimingCfg = NULL;
            pstPanelDesc->pstDsiInitCmds = NULL;
            pstPanelDesc->s32DsiInitCmdsSize = 0;
            if (pstVoCfg) {
                pstVoCfg->stVoPubAttr.enIntfType = VO_INTF_LVDS;
                pstVoCfg->stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
                pstVoCfg->stVoPubAttr.stSyncInfo = (VO_SYNC_INFO_S){.bSynm = 1, .bIop = 1, .u16FrameRate = 60,
                    .u16Vact = 768, .u16Vbb = 20, .u16Vfb = 10,
                    .u16Hact = 1366, .u16Hbb = 100, .u16Hfb = 88,
                    .u16Vpw = 2, .u16Hpw = 20, .bIdv = 0, .bIhs = 0, .bIvs = 0};
            }
            break;

        case PANEL_DSI_HX8394_EVB:
        case PANEL_BT656_TP2803:
        case PANEL_I80_ST7789V:
        default:
            pstPanelDesc->pchPanelName = "HX8394-720x1280";
            pstPanelDesc->pstDevCfg = &dev_cfg_hx8394_720x1280;
            pstPanelDesc->pstHsTimingCfg = &hs_timing_cfg_hx8394_720x1280;
            pstPanelDesc->pstDsiInitCmds = dsi_init_cmds_hx8394_720x1280;
            pstPanelDesc->s32DsiInitCmdsSize = ARRAY_SIZE(dsi_init_cmds_hx8394_720x1280);
            break;
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Panel_BT_Init(const APP_PARAM_VO_CFG_T* const pstVoCfg, const PANEL_TYPE_E enPanelType,
    const PANEL_I2C_CFG_T* const pstI2cCfg)
{
    CVI_S32 ret = CVI_SUCCESS;
    CVI_BOOL bNeedI2c = CVI_FALSE;
    VO_BT_ATTR_S stBtAttr;

    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    if (app_ipcam_Panel_IsMs7024(enPanelType)) {
        stBtAttr = stMS7024bt656cfg;
        bNeedI2c = CVI_TRUE;
    } else {
        // 其他 BT 面板初始化流程预留
        return CVI_SUCCESS;
    }

    // BT 参数设置
    ret = CVI_VO_SetPubAttr(pstVoCfg->s32VoDev, &pstVoCfg->stVoPubAttr);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VO_SetPubAttr failed with %#x!\n", ret);
        return ret;
    }

    ret = CVI_VO_SetBTParam(pstVoCfg->s32VoDev, &stBtAttr);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VO_SetBTParam failed with %#x!\n", ret);
        return ret;
    }

    // 回读校验 BT 参数
    ret = CVI_VO_GetBTParam(pstVoCfg->s32VoDev, &stBtAttr);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VO_GetBTParam failed with %#x!\n", ret);
        return ret;
    }

    if (bNeedI2c) {
        APP_CHK_RET(app_ipcam_Panel_I2c_Init(pstVoCfg->s32VoDev, pstI2cCfg),
                    "app_ipcam_Panel_I2c_Init");
        APP_CHK_RET(app_ipcam_Panel_I2c_SendInit(pstVoCfg->s32VoDev, enPanelType, pstI2cCfg),
                    "app_ipcam_Panel_I2c_SendInit");
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Panel_BT_Deinit(const APP_PARAM_VO_CFG_T* const pstVoCfg, const PANEL_TYPE_E enPanelType)
{
    CVI_BOOL bNeedI2c = CVI_FALSE;

    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    if (app_ipcam_Panel_IsMs7024(enPanelType)) {
        bNeedI2c = CVI_TRUE;
    } else {
        // 其他 BT 面板反初始化流程预留
        return CVI_SUCCESS;
    }

    // I2C 资源释放
    if (bNeedI2c) {
        APP_CHK_RET(app_ipcam_Panel_I2c_Exit(pstVoCfg->s32VoDev), "app_ipcam_Panel_I2c_Exit");
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Panel_Lvds_Init(const APP_PARAM_VO_CFG_T* const pstVoCfg, const PANEL_TYPE_E enPanelType)
{
    CVI_S32 ret = CVI_SUCCESS;
    VO_LVDS_ATTR_S stLvdsAttr;

    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    // 选择 LVDS 面板参数
    switch (enPanelType) {
    case PANEL_LVDS_LCM185X56:
        stLvdsAttr = lvds_lcm185x56_cfg;
        break;
    default:
        // 其他 LVDS 面板初始化流程预留
        return CVI_SUCCESS;
    }

    // LVDS 公共属性设置
    ret = CVI_VO_SetPubAttr(pstVoCfg->s32VoDev, &pstVoCfg->stVoPubAttr);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VO_SetPubAttr failed with %#x!\n", ret);
        return ret;
    }

    // LVDS 参数设置
    ret = CVI_VO_SetLVDSParam(pstVoCfg->s32VoDev, &stLvdsAttr);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VO_SetLVDSParam failed with %#x!\n", ret);
        return ret;
    }

    // 回读校验 LVDS 参数
    ret = CVI_VO_GetLVDSParam(pstVoCfg->s32VoDev, &stLvdsAttr);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VO_GetLVDSParam failed with %#x!\n", ret);
        return ret;
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Panel_Lvds_Deinit(const APP_PARAM_VO_CFG_T* const pstVoCfg, const PANEL_TYPE_E enPanelType)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    // 选择 LVDS 面板反初始化行为
    switch (enPanelType) {
    case PANEL_LVDS_LCM185X56:
        break;
    default:
        // 其他 LVDS 面板反初始化流程预留
        return CVI_SUCCESS;
    }

    // 目前无额外反初始化流程
    return CVI_SUCCESS;
}
