#ifndef __SC202CS_CMOS_EX_H_
#define __SC202CS_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

enum sc202cs_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_SHS1_2_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_DGAIN_0_ADDR,
	LINEAR_DGAIN_1_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,
	LINEAR_VTSRB_0_ADDR,
	LINEAR_VTSRB_1_ADDR,
	LINEAR_REGS_NUM
};

typedef enum _SC202CS_MODE_E {
	SC202CS_MODE_800X600P30 = 0,
	SC202CS_MODE_512X320P30_1L_MASTER_10BIT,
	SC202CS_MODE_512X320P30_1L_SLAVE_10BIT,
	SC202CS_MODE_NUM
} SC202CS_MODE_E;

typedef struct _SC202CS_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} SC202CS_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC202CS[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC202CS_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunSC202CS_AddrInfo[];
extern CVI_U16 g_au16SC202CS_GainMode[];
extern CVI_U16 g_au16SC202CS_L2SMode[];
extern const CVI_U8 sc202cs_i2c_addr;
extern const CVI_U32 sc202cs_addr_byte;
extern const CVI_U32 sc202cs_data_byte;
extern void sc202cs_init(VI_PIPE ViPipe);
extern void sc202cs_exit(VI_PIPE ViPipe);
extern int  sc202cs_i2c_exit(VI_PIPE ViPipe);
extern void sc202cs_standby(VI_PIPE ViPipe);
extern void sc202cs_restart(VI_PIPE ViPipe);
extern int  sc202cs_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc202cs_read_register(VI_PIPE ViPipe, int addr);
extern void sc202cs_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  sc202cs_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC202CS_CMOS_EX_H_ */
