#ifndef __MIS40H1_CMOS_EX_H_
#define __MIS40H1_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef ARCH_CV182X
#include <linux/cvi_vip_cif.h>
#include <linux/cvi_vip_snsr.h>
#include "cvi_type.h"
#else
#include <cvi_comm_cif.h>
#include <cvi_type.h>
#endif
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif


enum mis40h1_linear_regs_e {
	LINEAR_SHS_H_ADDR,
	LINEAR_SHS_L_ADDR,
	LINEAR_AGAIN_H_ADDR,
	LINEAR_AGAIN_L_ADDR,
	LINEAR_DGAIN_H_ADDR,
	LINEAR_DGAIN_L_ADDR,
	LINEAR_VMAX_H_ADDR,
	LINEAR_VMAX_L_ADDR,
	LINEAR_FLIP_MIRROR_ADDR,
	LINEAR_HCG_ADDR,
	LINEAR_LAUNCH,
	LINEAR_REGS_NUM
};

enum mis40h1_hcg_mode_e {
	LCG_MODE,
	HCG_MODE,
	MODE_NUM
};

typedef enum _MIS40H1_MODE_E {
	MIS40H1_MODE_1440P25_2L = 0,
	MIS40H1_MODE_NUM
} MIS40H1_MODE_E;

typedef struct _MIS40H1_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_LARGE_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} MIS40H1_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastMIS40H1[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunMIS40H1_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunMIS40H1_AddrInfo[];
extern CVI_U16 g_au16MIS40H1_GainMode[];
extern CVI_U16 g_au16MIS40H1_L2SMode[];
extern CVI_U16 g_au16MIS40H1_DcgValue[VI_MAX_PIPE_NUM];
extern CVI_U8 mis40h1_i2c_addr;
extern const CVI_U32 mis40h1_addr_byte;
extern const CVI_U32 mis40h1_data_byte;
extern void mis40h1_init(VI_PIPE ViPipe);
extern void mis40h1_exit(VI_PIPE ViPipe);
extern void mis40h1_standby(VI_PIPE ViPipe);
extern void mis40h1_restart(VI_PIPE ViPipe);
extern int  mis40h1_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  mis40h1_read_register(VI_PIPE ViPipe, int addr);
extern void mis40h1_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  mis40h1_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __MIS40H1_CMOS_EX_H_ */
