#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "cv2005_cmos_ex.h"
#include "sensor_i2c.h"

#define CV2005_CHIP_ID_ADDR_H	0x3003
#define CV2005_CHIP_ID_ADDR_L	0x3002
#define CV2005_CHIP_ID			0x2005

static void cv2005_linear_1080P30_init(VI_PIPE ViPipe);

CVI_U8 cv2005_i2c_addr = 0x35;
const CVI_U32 cv2005_addr_byte = 2;
const CVI_U32 cv2005_data_byte = 1;

int cv2005_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunCV2005_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunCV2005_AddrInfo[ViPipe].s8I2cAddr);
}

int cv2005_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunCV2005_BusInfo[ViPipe].s8I2cDev);
}

int cv2005_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunCV2005_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunCV2005_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							cv2005_addr_byte, cv2005_data_byte);
}

int cv2005_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunCV2005_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunCV2005_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							cv2005_addr_byte, (CVI_U32)data, cv2005_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void cv2005_standby(VI_PIPE ViPipe)
{
	cv2005_write_register(ViPipe, 0x3000, 0x1);

	printf("%s\n", __func__);
}

void cv2005_restart(VI_PIPE ViPipe)
{
	cv2005_write_register(ViPipe, 0x3000, 0x01);
	delay_ms(20);
	cv2005_write_register(ViPipe, 0x3000, 0x00);

	printf("%s\n", __func__);
}

void cv2005_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastCV2005[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		cv2005_write_register(ViPipe,
				g_pastCV2005[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastCV2005[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int cv2005_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (cv2005_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = cv2005_read_register(ViPipe, CV2005_CHIP_ID_ADDR_H);
	nVal2 = cv2005_read_register(ViPipe, CV2005_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != CV2005_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void cv2005_init(VI_PIPE ViPipe)
{
	CVI_U8            u8ImgMode;

	u8ImgMode   = g_pastCV2005[ViPipe]->u8ImgMode;

	cv2005_i2c_init(ViPipe);

	if (u8ImgMode == CV2005_MODE_1920X1080P30) {
		cv2005_linear_1080P30_init(ViPipe);
	}

	g_pastCV2005[ViPipe]->bInit = CVI_TRUE;
}

static void cv2005_linear_1080P30_init(VI_PIPE ViPipe)
{
    delay_ms(10);

	//15fps
	cv2005_write_register(ViPipe, 0x301C, 0x38);
	cv2005_write_register(ViPipe, 0x301D, 0x09);
	cv2005_write_register(ViPipe, 0x301E, 0x00);
	cv2005_write_register(ViPipe, 0x3020, 0x4E);
	cv2005_write_register(ViPipe, 0x3021, 0x04);
	cv2005_write_register(ViPipe, 0x3401, 0x00);
	cv2005_write_register(ViPipe, 0x3418, 0x77);
	cv2005_write_register(ViPipe, 0x3419, 0x00);
	cv2005_write_register(ViPipe, 0x341A, 0x37);
	cv2005_write_register(ViPipe, 0x341B, 0x00);
	cv2005_write_register(ViPipe, 0x341C, 0x37);
	cv2005_write_register(ViPipe, 0x341D, 0x00);
	cv2005_write_register(ViPipe, 0x341E, 0xE7);
	cv2005_write_register(ViPipe, 0x341F, 0x00);
	cv2005_write_register(ViPipe, 0x3420, 0x37);
	cv2005_write_register(ViPipe, 0x3421, 0x00);
	cv2005_write_register(ViPipe, 0x3422, 0x67);
	cv2005_write_register(ViPipe, 0x3423, 0x00);
	cv2005_write_register(ViPipe, 0x3424, 0x37);
	cv2005_write_register(ViPipe, 0x3425, 0x00);
	cv2005_write_register(ViPipe, 0x3426, 0x57);
	cv2005_write_register(ViPipe, 0x3427, 0x00);
	cv2005_write_register(ViPipe, 0x3428, 0x2F);
	cv2005_write_register(ViPipe, 0x3429, 0x00);
	cv2005_write_register(ViPipe, 0x3442, 0x00);
	cv2005_write_register(ViPipe, 0x3808, 0x41);
	cv2005_write_register(ViPipe, 0x380A, 0x02);
	cv2005_write_register(ViPipe, 0x3499, 0x01);
	cv2005_write_register(ViPipe, 0x356F, 0x0F);
	cv2005_write_register(ViPipe, 0x310C, 0x64);
	cv2005_write_register(ViPipe, 0x359D, 0x00);
	cv2005_write_register(ViPipe, 0x3031, 0x00);
	cv2005_write_register(ViPipe, 0x3204, 0x40);

	cv2005_write_register(ViPipe, 0x3014, 0x00);
	cv2005_write_register(ViPipe, 0x3030, 0x01);
	cv2005_write_register(ViPipe, 0x3038, 0x00);
	cv2005_write_register(ViPipe, 0x3039, 0x00);
	cv2005_write_register(ViPipe, 0x303A, 0x88);
	cv2005_write_register(ViPipe, 0x303B, 0x07);
	cv2005_write_register(ViPipe, 0x3034, 0x00);
	cv2005_write_register(ViPipe, 0x3035, 0x00);
	cv2005_write_register(ViPipe, 0x3036, 0x40);
	cv2005_write_register(ViPipe, 0x3037, 0x04);

	cv2005_default_reg_init(ViPipe);
	delay_ms(100);
	cv2005_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d,===CV2005 1080P 15fps 10bit LINEAR Init OK!===\n", ViPipe);
}
