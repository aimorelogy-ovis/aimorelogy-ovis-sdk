#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc431hai_cmos_ex.h"
#include "sensor_i2c.h"

static void sc431hai_linear_1440p12_init(VI_PIPE ViPipe);
static void sc431hai_linear_1440p30_init(VI_PIPE ViPipe);
static void sc431hai_linear_1440p20_1l_init(VI_PIPE ViPipe);

const CVI_U8 sc431hai_i2c_addr = 0x30;        /* I2C Address of SC431HAI */
const CVI_U32 sc431hai_addr_byte = 2;
const CVI_U32 sc431hai_data_byte = 1;

int sc431hai_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC431HAI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC431HAI_AddrInfo[ViPipe].s8I2cAddr);
}

int sc431hai_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC431HAI_BusInfo[ViPipe].s8I2cDev);
}

int sc431hai_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC431HAI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC431HAI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc431hai_addr_byte, sc431hai_data_byte);
}

int sc431hai_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC431HAI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC431HAI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc431hai_addr_byte, (CVI_U32)data, sc431hai_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc431hai_standby(VI_PIPE ViPipe)
{
	sc431hai_write_register(ViPipe, 0x0100, 0x00);
}

void sc431hai_restart(VI_PIPE ViPipe)
{
	sc431hai_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc431hai_write_register(ViPipe, 0x0100, 0x01);
}

void sc431hai_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC431HAI[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc431hai_write_register(ViPipe,
				g_pastSC431HAI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC431HAI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define SC431HAI_CHIP_ID_HI_ADDR		0x3107
#define SC431HAI_CHIP_ID_LO_ADDR		0x3108
#define SC431HAI_CHIP_ID			0xcd6b

// void sc431hai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
// {
// 	CVI_U8 val = 0;

// 	switch (eSnsMirrorFlip) {
// 	case ISP_SNS_NORMAL:
// 		break;
// 	case ISP_SNS_MIRROR:
// 		val |= 0x6;
// 		break;
// 	case ISP_SNS_FLIP:
// 		val |= 0x60;
// 		break;
// 	case ISP_SNS_MIRROR_FLIP:
// 		val |= 0x66;
// 		break;
// 	default:
// 		return;
// 	}

// 	sc431hai_write_register(ViPipe, 0x3221, val);
// }

int sc431hai_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc431hai_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc431hai_read_register(ViPipe, SC431HAI_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc431hai_read_register(ViPipe, SC431HAI_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC431HAI_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc431hai_init(VI_PIPE ViPipe)
{
	WDR_MODE_E       enWDRMode;
	CVI_BOOL          bInit;
	CVI_U8            u8ImgMode;

	bInit       = g_pastSC431HAI[ViPipe]->bInit;
	enWDRMode   = g_pastSC431HAI[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastSC431HAI[ViPipe]->u8ImgMode;

	sc431hai_i2c_init(ViPipe);

	/* When sensor first init, config all registers */
	if (bInit == CVI_FALSE) {
		if (enWDRMode == WDR_MODE_2To1_LINE) {
		} else {
			if (u8ImgMode == SC431HAI_MODE_1440P12)
				sc431hai_linear_1440p12_init(ViPipe);
			else if (u8ImgMode == SC431HAI_MODE_1440P30) {
				sc431hai_linear_1440p30_init(ViPipe);
			} else if (u8ImgMode == SC431HAI_1L_MODE_1440P20) {
				sc431hai_linear_1440p20_1l_init(ViPipe);
			}
		}
	}
	else {
		if (enWDRMode == WDR_MODE_2To1_LINE) {
		} else {
			if (u8ImgMode == SC431HAI_MODE_1440P12)
				sc431hai_linear_1440p12_init(ViPipe);
			else if (u8ImgMode == SC431HAI_MODE_1440P30) {
				sc431hai_linear_1440p30_init(ViPipe);
			} else if (u8ImgMode == SC431HAI_1L_MODE_1440P20) {
				sc431hai_linear_1440p20_1l_init(ViPipe);
			}
		}
	}
	g_pastSC431HAI[ViPipe]->bInit = CVI_TRUE;
}

void sc431hai_exit(VI_PIPE ViPipe)
{
	sc431hai_i2c_exit(ViPipe);
}
static void sc431hai_linear_1440p12_init(VI_PIPE ViPipe)
{
	printf("ViPipe:%d,===SC431HAI 1440p 12fps 10bit LINE Init OK!===\n", ViPipe);
}

static void sc431hai_linear_1440p30_init(VI_PIPE ViPipe)
{
	sc431hai_write_register(ViPipe, 0x0100, 0x00);
	sc431hai_write_register(ViPipe, 0x36e9, 0x80);
	sc431hai_write_register(ViPipe, 0x37f9, 0x80);
	sc431hai_write_register(ViPipe, 0x3018, 0x3a);
	sc431hai_write_register(ViPipe, 0x3019, 0x0c);
	sc431hai_write_register(ViPipe, 0x301f, 0x05);
	sc431hai_write_register(ViPipe, 0x3058, 0x21);
	sc431hai_write_register(ViPipe, 0x3059, 0x53);
	sc431hai_write_register(ViPipe, 0x305a, 0x40);
	sc431hai_write_register(ViPipe, 0x320c, 0x05);//1400
	sc431hai_write_register(ViPipe, 0x320d, 0x78);
	sc431hai_write_register(ViPipe, 0x320e, 0x05);//3750 12FPS
	sc431hai_write_register(ViPipe, 0x320f, 0xdc);
	sc431hai_write_register(ViPipe, 0x3222, 0x00);
	sc431hai_write_register(ViPipe, 0x300a, 0x24);
	sc431hai_write_register(ViPipe, 0x3032, 0xa0);
	sc431hai_write_register(ViPipe, 0x3250, 0x00);
	sc431hai_write_register(ViPipe, 0x3301, 0x0c);
	sc431hai_write_register(ViPipe, 0x3304, 0x50);
	sc431hai_write_register(ViPipe, 0x3305, 0x00);
	sc431hai_write_register(ViPipe, 0x3306, 0x50);
	sc431hai_write_register(ViPipe, 0x3307, 0x04);
	sc431hai_write_register(ViPipe, 0x3308, 0x0a);
	sc431hai_write_register(ViPipe, 0x3309, 0x60);
	sc431hai_write_register(ViPipe, 0x330b, 0xc8);
	sc431hai_write_register(ViPipe, 0x330d, 0x08);
	sc431hai_write_register(ViPipe, 0x330e, 0x38);
	sc431hai_write_register(ViPipe, 0x331e, 0x41);
	sc431hai_write_register(ViPipe, 0x331f, 0x51);
	sc431hai_write_register(ViPipe, 0x3333, 0x10);
	sc431hai_write_register(ViPipe, 0x3334, 0x40);
	sc431hai_write_register(ViPipe, 0x3364, 0x5e);
	sc431hai_write_register(ViPipe, 0x338e, 0xe2);
	sc431hai_write_register(ViPipe, 0x338f, 0x80);
	sc431hai_write_register(ViPipe, 0x3390, 0x08);
	sc431hai_write_register(ViPipe, 0x3391, 0x18);
	sc431hai_write_register(ViPipe, 0x3392, 0xb8);
	sc431hai_write_register(ViPipe, 0x3393, 0x12);
	sc431hai_write_register(ViPipe, 0x3394, 0x14);
	sc431hai_write_register(ViPipe, 0x3395, 0x10);
	sc431hai_write_register(ViPipe, 0x3396, 0x88);
	sc431hai_write_register(ViPipe, 0x3397, 0x98);
	sc431hai_write_register(ViPipe, 0x3398, 0xb8);
	sc431hai_write_register(ViPipe, 0x3399, 0x10);
	sc431hai_write_register(ViPipe, 0x339a, 0x16);
	sc431hai_write_register(ViPipe, 0x339b, 0x1c);
	sc431hai_write_register(ViPipe, 0x339c, 0x40);
	sc431hai_write_register(ViPipe, 0x33ac, 0x0a);
	sc431hai_write_register(ViPipe, 0x33ad, 0x10);
	sc431hai_write_register(ViPipe, 0x33ae, 0x4f);
	sc431hai_write_register(ViPipe, 0x33af, 0x5e);
	sc431hai_write_register(ViPipe, 0x33b1, 0x80);
	sc431hai_write_register(ViPipe, 0x33b2, 0x50);
	sc431hai_write_register(ViPipe, 0x33b3, 0x10);
	sc431hai_write_register(ViPipe, 0x33f8, 0x00);
	sc431hai_write_register(ViPipe, 0x33f9, 0x50);
	sc431hai_write_register(ViPipe, 0x33fa, 0x00);
	sc431hai_write_register(ViPipe, 0x33fb, 0x50);
	sc431hai_write_register(ViPipe, 0x33fc, 0x48);
	sc431hai_write_register(ViPipe, 0x33fd, 0x78);
	sc431hai_write_register(ViPipe, 0x349f, 0x03);
	sc431hai_write_register(ViPipe, 0x34a6, 0x40);
	sc431hai_write_register(ViPipe, 0x34a7, 0x58);
	sc431hai_write_register(ViPipe, 0x34a8, 0x10);
	sc431hai_write_register(ViPipe, 0x34a9, 0x10);
	sc431hai_write_register(ViPipe, 0x34f8, 0x78);
	sc431hai_write_register(ViPipe, 0x34f9, 0x10);
	sc431hai_write_register(ViPipe, 0x3633, 0x44);
	sc431hai_write_register(ViPipe, 0x363b, 0x8f);
	sc431hai_write_register(ViPipe, 0x363c, 0x02);
	sc431hai_write_register(ViPipe, 0x3641, 0x08);
	sc431hai_write_register(ViPipe, 0x3654, 0x20);
	sc431hai_write_register(ViPipe, 0x3674, 0xc2);
	sc431hai_write_register(ViPipe, 0x3675, 0xb4);
	sc431hai_write_register(ViPipe, 0x3676, 0x88);
	sc431hai_write_register(ViPipe, 0x367c, 0x88);
	sc431hai_write_register(ViPipe, 0x367d, 0xb8);
	sc431hai_write_register(ViPipe, 0x3690, 0x34);
	sc431hai_write_register(ViPipe, 0x3691, 0x44);
	sc431hai_write_register(ViPipe, 0x3692, 0x54);
	sc431hai_write_register(ViPipe, 0x3693, 0x88);
	sc431hai_write_register(ViPipe, 0x3694, 0x98);
	sc431hai_write_register(ViPipe, 0x3696, 0x80);
	sc431hai_write_register(ViPipe, 0x3697, 0x83);
	sc431hai_write_register(ViPipe, 0x3698, 0x81);
	sc431hai_write_register(ViPipe, 0x3699, 0x81);
	sc431hai_write_register(ViPipe, 0x369a, 0x84);
	sc431hai_write_register(ViPipe, 0x369b, 0x82);
	sc431hai_write_register(ViPipe, 0x36a2, 0x80);
	sc431hai_write_register(ViPipe, 0x36a3, 0x88);
	sc431hai_write_register(ViPipe, 0x36a4, 0xf8);
	sc431hai_write_register(ViPipe, 0x36a5, 0xb8);
	sc431hai_write_register(ViPipe, 0x36a6, 0x98);
	sc431hai_write_register(ViPipe, 0x36d0, 0x15);
	sc431hai_write_register(ViPipe, 0x36ec, 0x55);
	sc431hai_write_register(ViPipe, 0x36ed, 0x18);
	sc431hai_write_register(ViPipe, 0x370f, 0x01);
	sc431hai_write_register(ViPipe, 0x3722, 0x03);
	sc431hai_write_register(ViPipe, 0x3724, 0x92);
	sc431hai_write_register(ViPipe, 0x3727, 0x14);
	sc431hai_write_register(ViPipe, 0x37b0, 0x17);
	sc431hai_write_register(ViPipe, 0x37b1, 0x9b);
	sc431hai_write_register(ViPipe, 0x37b2, 0x9b);
	sc431hai_write_register(ViPipe, 0x37b3, 0x88);
	sc431hai_write_register(ViPipe, 0x37b4, 0xb8);
	sc431hai_write_register(ViPipe, 0x37fa, 0x23);
	sc431hai_write_register(ViPipe, 0x37fb, 0x54);
	sc431hai_write_register(ViPipe, 0x37fc, 0x21);
	sc431hai_write_register(ViPipe, 0x391f, 0x41);
	sc431hai_write_register(ViPipe, 0x3926, 0xe0);
	sc431hai_write_register(ViPipe, 0x3933, 0x80);
	sc431hai_write_register(ViPipe, 0x3934, 0xf8);
	sc431hai_write_register(ViPipe, 0x3935, 0x00);
	sc431hai_write_register(ViPipe, 0x3936, 0x45);
	sc431hai_write_register(ViPipe, 0x3937, 0x66);
	sc431hai_write_register(ViPipe, 0x3938, 0x66);
	sc431hai_write_register(ViPipe, 0x3939, 0x00);
	sc431hai_write_register(ViPipe, 0x393a, 0x03);
	sc431hai_write_register(ViPipe, 0x393b, 0x00);
	sc431hai_write_register(ViPipe, 0x393c, 0x00);
	sc431hai_write_register(ViPipe, 0x393d, 0x02);
	sc431hai_write_register(ViPipe, 0x393e, 0x80);
	sc431hai_write_register(ViPipe, 0x3e00, 0x00);
	sc431hai_write_register(ViPipe, 0x3e01, 0xba);
	sc431hai_write_register(ViPipe, 0x3e02, 0xd0);
	sc431hai_write_register(ViPipe, 0x3e16, 0x00);
	sc431hai_write_register(ViPipe, 0x3e17, 0xc5);
	sc431hai_write_register(ViPipe, 0x3e18, 0x00);
	sc431hai_write_register(ViPipe, 0x3e19, 0xc5);
	sc431hai_write_register(ViPipe, 0x4509, 0x20);
	sc431hai_write_register(ViPipe, 0x450d, 0x0b);
	sc431hai_write_register(ViPipe, 0x4819, 0x08);
	sc431hai_write_register(ViPipe, 0x481b, 0x05);
	sc431hai_write_register(ViPipe, 0x481d, 0x11);
	sc431hai_write_register(ViPipe, 0x481f, 0x04);
	sc431hai_write_register(ViPipe, 0x4821, 0x09);
	sc431hai_write_register(ViPipe, 0x4823, 0x04);
	sc431hai_write_register(ViPipe, 0x4825, 0x04);
	sc431hai_write_register(ViPipe, 0x4827, 0x04);
	sc431hai_write_register(ViPipe, 0x4829, 0x07);
	sc431hai_write_register(ViPipe, 0x5780, 0x76);
	sc431hai_write_register(ViPipe, 0x5784, 0x0a);
	sc431hai_write_register(ViPipe, 0x5785, 0x04);
	sc431hai_write_register(ViPipe, 0x5787, 0x0a);
	sc431hai_write_register(ViPipe, 0x5788, 0x0a);
	sc431hai_write_register(ViPipe, 0x5789, 0x08);
	sc431hai_write_register(ViPipe, 0x578a, 0x0a);
	sc431hai_write_register(ViPipe, 0x578b, 0x0a);
	sc431hai_write_register(ViPipe, 0x578c, 0x08);
	sc431hai_write_register(ViPipe, 0x578d, 0x40);
	sc431hai_write_register(ViPipe, 0x5790, 0x08);
	sc431hai_write_register(ViPipe, 0x5791, 0x04);
	sc431hai_write_register(ViPipe, 0x5792, 0x04);
	sc431hai_write_register(ViPipe, 0x5793, 0x08);
	sc431hai_write_register(ViPipe, 0x5794, 0x04);
	sc431hai_write_register(ViPipe, 0x5795, 0x04);
	sc431hai_write_register(ViPipe, 0x57ac, 0x00);
	sc431hai_write_register(ViPipe, 0x57ad, 0x00);
	sc431hai_write_register(ViPipe, 0x36e9, 0x44);
	sc431hai_write_register(ViPipe, 0x37f9, 0x44);

	sc431hai_default_reg_init(ViPipe);

	sc431hai_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC431HAI 1440p 30fps 10bit LINE Init OK!===\n", ViPipe);
}

static void sc431hai_linear_1440p20_1l_init(VI_PIPE ViPipe)
{
	sc431hai_write_register(ViPipe, 0x0100, 0x00);
	sc431hai_write_register(ViPipe, 0x36e9, 0x80);
	sc431hai_write_register(ViPipe, 0x37f9, 0x80);
	sc431hai_write_register(ViPipe, 0x3018, 0x1a);
	sc431hai_write_register(ViPipe, 0x3019, 0x0e);
	sc431hai_write_register(ViPipe, 0x301f, 0x34);
	sc431hai_write_register(ViPipe, 0x3058, 0x21);
	sc431hai_write_register(ViPipe, 0x3059, 0x53);
	sc431hai_write_register(ViPipe, 0x305a, 0x40);
	sc431hai_write_register(ViPipe, 0x320c, 0x05);
	sc431hai_write_register(ViPipe, 0x320d, 0xa0);
	sc431hai_write_register(ViPipe, 0x3250, 0x00);
	sc431hai_write_register(ViPipe, 0x3301, 0x0c);
	sc431hai_write_register(ViPipe, 0x3304, 0x50);
	sc431hai_write_register(ViPipe, 0x3305, 0x00);
	sc431hai_write_register(ViPipe, 0x3306, 0x50);
	sc431hai_write_register(ViPipe, 0x3307, 0x04);
	sc431hai_write_register(ViPipe, 0x3308, 0x0a);
	sc431hai_write_register(ViPipe, 0x3309, 0x60);
	sc431hai_write_register(ViPipe, 0x330b, 0xc8);
	sc431hai_write_register(ViPipe, 0x330d, 0x08);
	sc431hai_write_register(ViPipe, 0x330e, 0x38);
	sc431hai_write_register(ViPipe, 0x331e, 0x41);
	sc431hai_write_register(ViPipe, 0x331f, 0x51);
	sc431hai_write_register(ViPipe, 0x3333, 0x10);
	sc431hai_write_register(ViPipe, 0x3334, 0x40);
	sc431hai_write_register(ViPipe, 0x3364, 0x5e);
	sc431hai_write_register(ViPipe, 0x338e, 0xe7);
	sc431hai_write_register(ViPipe, 0x338f, 0x80);
	sc431hai_write_register(ViPipe, 0x3390, 0x08);
	sc431hai_write_register(ViPipe, 0x3391, 0x18);
	sc431hai_write_register(ViPipe, 0x3392, 0xb8);
	sc431hai_write_register(ViPipe, 0x3393, 0x12);
	sc431hai_write_register(ViPipe, 0x3394, 0x14);
	sc431hai_write_register(ViPipe, 0x3395, 0x10);
	sc431hai_write_register(ViPipe, 0x3396, 0x88);
	sc431hai_write_register(ViPipe, 0x3397, 0x98);
	sc431hai_write_register(ViPipe, 0x3398, 0xb8);
	sc431hai_write_register(ViPipe, 0x3399, 0x10);
	sc431hai_write_register(ViPipe, 0x339a, 0x16);
	sc431hai_write_register(ViPipe, 0x339b, 0x1c);
	sc431hai_write_register(ViPipe, 0x339c, 0x40);
	sc431hai_write_register(ViPipe, 0x33ac, 0x0a);
	sc431hai_write_register(ViPipe, 0x33ad, 0x10);
	sc431hai_write_register(ViPipe, 0x33ae, 0x4f);
	sc431hai_write_register(ViPipe, 0x33af, 0x5e);
	sc431hai_write_register(ViPipe, 0x33b1, 0x80);
	sc431hai_write_register(ViPipe, 0x33b2, 0x50);
	sc431hai_write_register(ViPipe, 0x33b3, 0x10);
	sc431hai_write_register(ViPipe, 0x33f8, 0x00);
	sc431hai_write_register(ViPipe, 0x33f9, 0x50);
	sc431hai_write_register(ViPipe, 0x33fa, 0x00);
	sc431hai_write_register(ViPipe, 0x33fb, 0x50);
	sc431hai_write_register(ViPipe, 0x33fc, 0x48);
	sc431hai_write_register(ViPipe, 0x33fd, 0x78);
	sc431hai_write_register(ViPipe, 0x349f, 0x03);
	sc431hai_write_register(ViPipe, 0x34a6, 0x40);
	sc431hai_write_register(ViPipe, 0x34a7, 0x58);
	sc431hai_write_register(ViPipe, 0x34a8, 0x10);
	sc431hai_write_register(ViPipe, 0x34a9, 0x10);
	sc431hai_write_register(ViPipe, 0x34f8, 0x78);
	sc431hai_write_register(ViPipe, 0x34f9, 0x10);
	sc431hai_write_register(ViPipe, 0x3633, 0x44);
	sc431hai_write_register(ViPipe, 0x363b, 0x8f);
	sc431hai_write_register(ViPipe, 0x363c, 0x02);
	sc431hai_write_register(ViPipe, 0x3641, 0x08);
	sc431hai_write_register(ViPipe, 0x3654, 0x20);
	sc431hai_write_register(ViPipe, 0x3674, 0xc2);
	sc431hai_write_register(ViPipe, 0x3675, 0xb4);
	sc431hai_write_register(ViPipe, 0x3676, 0x88);
	sc431hai_write_register(ViPipe, 0x367c, 0x88);
	sc431hai_write_register(ViPipe, 0x367d, 0xb8);
	sc431hai_write_register(ViPipe, 0x3690, 0x34);
	sc431hai_write_register(ViPipe, 0x3691, 0x44);
	sc431hai_write_register(ViPipe, 0x3692, 0x54);
	sc431hai_write_register(ViPipe, 0x3693, 0x88);
	sc431hai_write_register(ViPipe, 0x3694, 0x98);
	sc431hai_write_register(ViPipe, 0x3696, 0x80);
	sc431hai_write_register(ViPipe, 0x3697, 0x83);
	sc431hai_write_register(ViPipe, 0x3698, 0x81);
	sc431hai_write_register(ViPipe, 0x3699, 0x81);
	sc431hai_write_register(ViPipe, 0x369a, 0x84);
	sc431hai_write_register(ViPipe, 0x369b, 0x82);
	sc431hai_write_register(ViPipe, 0x36a2, 0x80);
	sc431hai_write_register(ViPipe, 0x36a3, 0x88);
	sc431hai_write_register(ViPipe, 0x36a4, 0xf8);
	sc431hai_write_register(ViPipe, 0x36a5, 0xb8);
	sc431hai_write_register(ViPipe, 0x36a6, 0x98);
	sc431hai_write_register(ViPipe, 0x36d0, 0x15);
	sc431hai_write_register(ViPipe, 0x36ea, 0x10);
	sc431hai_write_register(ViPipe, 0x36eb, 0x0d);
	sc431hai_write_register(ViPipe, 0x36ec, 0x45);
	sc431hai_write_register(ViPipe, 0x36ed, 0x08);
	sc431hai_write_register(ViPipe, 0x370f, 0x01);
	sc431hai_write_register(ViPipe, 0x3722, 0x03);
	sc431hai_write_register(ViPipe, 0x3724, 0x92);
	sc431hai_write_register(ViPipe, 0x3727, 0x14);
	sc431hai_write_register(ViPipe, 0x37b0, 0x17);
	sc431hai_write_register(ViPipe, 0x37b1, 0x9b);
	sc431hai_write_register(ViPipe, 0x37b2, 0x9b);
	sc431hai_write_register(ViPipe, 0x37b3, 0x88);
	sc431hai_write_register(ViPipe, 0x37b4, 0xb8);
	sc431hai_write_register(ViPipe, 0x37fa, 0x10);
	sc431hai_write_register(ViPipe, 0x37fb, 0x54);
	sc431hai_write_register(ViPipe, 0x37fc, 0x21);
	sc431hai_write_register(ViPipe, 0x37fd, 0x0c);
	sc431hai_write_register(ViPipe, 0x391f, 0x41);
	sc431hai_write_register(ViPipe, 0x3926, 0xe0);
	sc431hai_write_register(ViPipe, 0x3933, 0x80);
	sc431hai_write_register(ViPipe, 0x3934, 0xf8);
	sc431hai_write_register(ViPipe, 0x3935, 0x00);
	sc431hai_write_register(ViPipe, 0x3936, 0x45);
	sc431hai_write_register(ViPipe, 0x3937, 0x66);
	sc431hai_write_register(ViPipe, 0x3938, 0x66);
	sc431hai_write_register(ViPipe, 0x3939, 0x00);
	sc431hai_write_register(ViPipe, 0x393a, 0x03);
	sc431hai_write_register(ViPipe, 0x393b, 0x00);
	sc431hai_write_register(ViPipe, 0x393c, 0x00);
	sc431hai_write_register(ViPipe, 0x393d, 0x02);
	sc431hai_write_register(ViPipe, 0x393e, 0x80);
	sc431hai_write_register(ViPipe, 0x3e00, 0x00);
	sc431hai_write_register(ViPipe, 0x3e01, 0xba);
	sc431hai_write_register(ViPipe, 0x3e02, 0xd0);
	sc431hai_write_register(ViPipe, 0x3e16, 0x00);
	sc431hai_write_register(ViPipe, 0x3e17, 0xc5);
	sc431hai_write_register(ViPipe, 0x3e18, 0x00);
	sc431hai_write_register(ViPipe, 0x3e19, 0xc5);
	sc431hai_write_register(ViPipe, 0x4509, 0x20);
	sc431hai_write_register(ViPipe, 0x450d, 0x0b);
	sc431hai_write_register(ViPipe, 0x4819, 0x0b);
	sc431hai_write_register(ViPipe, 0x481b, 0x06);
	sc431hai_write_register(ViPipe, 0x481d, 0x17);
	sc431hai_write_register(ViPipe, 0x481f, 0x05);
	sc431hai_write_register(ViPipe, 0x4821, 0x0b);
	sc431hai_write_register(ViPipe, 0x4823, 0x06);
	sc431hai_write_register(ViPipe, 0x4825, 0x05);
	sc431hai_write_register(ViPipe, 0x4827, 0x05);
	sc431hai_write_register(ViPipe, 0x4829, 0x09);
	sc431hai_write_register(ViPipe, 0x5780, 0x76);
	sc431hai_write_register(ViPipe, 0x5784, 0x0a);
	sc431hai_write_register(ViPipe, 0x5785, 0x04);
	sc431hai_write_register(ViPipe, 0x5787, 0x0a);
	sc431hai_write_register(ViPipe, 0x5788, 0x0a);
	sc431hai_write_register(ViPipe, 0x5789, 0x08);
	sc431hai_write_register(ViPipe, 0x578a, 0x0a);
	sc431hai_write_register(ViPipe, 0x578b, 0x0a);
	sc431hai_write_register(ViPipe, 0x578c, 0x08);
	sc431hai_write_register(ViPipe, 0x578d, 0x40);
	sc431hai_write_register(ViPipe, 0x5790, 0x08);
	sc431hai_write_register(ViPipe, 0x5791, 0x04);
	sc431hai_write_register(ViPipe, 0x5792, 0x04);
	sc431hai_write_register(ViPipe, 0x5793, 0x08);
	sc431hai_write_register(ViPipe, 0x5794, 0x04);
	sc431hai_write_register(ViPipe, 0x5795, 0x04);
	sc431hai_write_register(ViPipe, 0x57ac, 0x00);
	sc431hai_write_register(ViPipe, 0x57ad, 0x00);
	sc431hai_write_register(ViPipe, 0x36e9, 0x04);
	sc431hai_write_register(ViPipe, 0x37f9, 0x04);
	sc431hai_default_reg_init(ViPipe);

	sc431hai_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC431HAI 1l 1440p 20fps 10bit LINE Init OK!===\n", ViPipe);
}
