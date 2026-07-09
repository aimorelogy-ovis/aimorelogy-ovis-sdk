/*
 * Copyright Bitmain Technologies Inc.
 *
 * Created Time: Dec, 2019
 */
#ifndef __MAIN_ENC_CVITEST_H__
#define __MAIN_ENC_CVITEST_H__

#include "cvi_defines.h"
#define H264_VENC_INTR_NUM 76
#define H265_VENC_INTR_NUM 77
//#define SUPPORT_INTERRUPT

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef TEST_H264_ENC
int cvitest_h264_venc_main(int argc, char **argv);
#ifdef SUPPORT_INTERRUPT
int irq_handler_h264_venc(int irqn, void *priv);
#endif
#endif

#ifdef TEST_H265_ENC
int cvitest_venc_main(int argc, char **argv);
#ifdef SUPPORT_INTERRUPT
int irq_handler_h265_venc(int irqn, void *priv);
#endif
#endif

enum TestEnc
{
    TE_STA_ENC_BREAK          = 20,
    TE_STA_ENC_TIMEOUT        = 21,
    TE_ERR_ENC_INIT           = -10,
    TE_ERR_ENC_OPEN           = -11,
    TE_ERR_ENC_USER_DATA      = -12,
    TE_ERR_ENC_H264_ENTROPY   = -13,
    TE_ERR_ENC_H264_TRANS     = -14,
    TE_ERR_ENC_H265_TRANS     = -15,
    TE_ERR_ENC_IS_SUPER_FRAME = -16,
    TE_ERR_ENC_H264_VUI       = -17,
    TE_ERR_ENC_H265_VUI       = -18,
};

typedef struct _venc_sbm_info_t
{
    int  targetChnNum;
    int  sbmChnNum; // only one H26x chn in SBM mode
    int  frmChnNum[VENC_MAX_CHN_NUM];
    int  numEncChn;
    BOOL bSBModeEn;
    BOOL bFRModeEn;
    int  rrChnIdx;
} venc_sbm_info_t;
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
