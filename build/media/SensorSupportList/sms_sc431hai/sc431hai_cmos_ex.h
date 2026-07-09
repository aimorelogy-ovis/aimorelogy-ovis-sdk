#ifndef __SC431HAI_CMOS_EX_H_
#define __SC431HAI_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"


enum sc431hai_linear_regs_e {
	LINEAR_GROUP_HOLD_START,
	LINEAR_SHS_H_ADDR,
	LINEAR_SHS_M_ADDR,
	LINEAR_SHS_L_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_AGAIN_FINE_ADDR,
	LINEAR_DGAIN_ADDR,
	LINEAR_DGAIN_FINE_ADDR,
	LINEAR_VMAX_H_ADDR,
	LINEAR_VMAX_L_ADDR,
	LINEAR_FLIP_MIRROR,
	LINEAR_GROUP_HOLD_END,
	LINEAR_REGS_NUM
};


typedef enum _SC431HAI_MODE_E {
	SC431HAI_MODE_1440P12 = 0,
	SC431HAI_MODE_1440P30,
	SC431HAI_1L_MODE_1440P20,
	SC431HAI_MODE_LINEAR_NUM,
	SC431HAI_MODE_NUM
} SC431HAI_MODE_E;

typedef struct _SC431HAI_STATE_S {
	CVI_U32		u32Sexp_MAX;	/* (2*{16’h3e23,16’h3e24} – 'd21)/2 */
} SC431HAI_STATE_S;

typedef struct _SC431HAI_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_S stAgain[2];
	SNS_ATTR_S stDgain[2];
	CVI_U16 u16SexpMaxReg;		/* {16’h3e23,16’h3e24} */
	char name[64];
} SC431HAI_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSC431HAI[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSC431HAI_BusInfo[];
extern ISP_SNS_COMMADDR_U g_aunSC431HAI_AddrInfo[];
extern CVI_U16 g_au16SC431HAI_GainMode[];
extern CVI_U16 g_au16SC431HAI_L2SMode[];
extern const CVI_U8 sc431hai_i2c_addr;
extern const CVI_U32 sc431hai_addr_byte;
extern const CVI_U32 sc431hai_data_byte;
extern void sc431hai_init(VI_PIPE ViPipe);
extern void sc431hai_exit(VI_PIPE ViPipe);
extern void sc431hai_standby(VI_PIPE ViPipe);
extern void sc431hai_restart(VI_PIPE ViPipe);
extern int  sc431hai_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc431hai_read_register(VI_PIPE ViPipe, int addr);
extern int  sc431hai_probe(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC431HAI_CMOS_EX_H_ */
