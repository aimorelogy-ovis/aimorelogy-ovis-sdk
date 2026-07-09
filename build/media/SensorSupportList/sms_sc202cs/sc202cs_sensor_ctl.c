#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc202cs_cmos_ex.h"
#include "sensor_i2c.h"

static void sc202cs_linear_600P30_init(VI_PIPE ViPipe);
static void sc202cs_linear_slave_320P30_init(VI_PIPE ViPipe);
static void sc202cs_linear_master_320P30_init(VI_PIPE ViPipe);

const CVI_U8 sc202cs_i2c_addr = 0x36;        /* I2C Address of SC202CS */
const CVI_U32 sc202cs_addr_byte = 2;
const CVI_U32 sc202cs_data_byte = 1;

#define SC202CS_CHIP_ID_HI_ADDR		0x3107
#define SC202CS_CHIP_ID_LO_ADDR		0x3108
#define SC202CS_CHIP_ID				0xeb52

int sc202cs_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC202CS_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC202CS_AddrInfo[ViPipe].s8I2cAddr);
}

int sc202cs_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC202CS_BusInfo[ViPipe].s8I2cDev);
}

int sc202cs_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC202CS_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC202CS_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc202cs_addr_byte, sc202cs_data_byte);
}

int sc202cs_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC202CS_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC202CS_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc202cs_addr_byte, (CVI_U32)data, sc202cs_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc202cs_standby(VI_PIPE ViPipe)
{
    CVI_U8 val = sc202cs_read_register(ViPipe, 0x0100);
    val &= ~0x01;
	sc202cs_write_register(ViPipe, 0x0100, val);
}

void sc202cs_restart(VI_PIPE ViPipe)
{
    CVI_U8 val = sc202cs_read_register(ViPipe, 0x0103);
    val |= 0x01;
	sc202cs_write_register(ViPipe, 0x0103, val);
	delay_ms(20);
    val &= ~0x01;
	sc202cs_write_register(ViPipe, 0x0103, val);
}

void sc202cs_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = sc202cs_read_register(ViPipe, 0x3221) & ~0x66;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x6;
		break;
	case ISP_SNS_FLIP:
		val |= 0x60;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x66;
		break;
	default:
		return;
	}

	sc202cs_write_register(ViPipe, 0x3221, val);
}

int sc202cs_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc202cs_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc202cs_read_register(ViPipe, SC202CS_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc202cs_read_register(ViPipe, SC202CS_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC202CS_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc202cs_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;
	for (i = 0; i < g_pastSC202CS[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc202cs_write_register(ViPipe,
				g_pastSC202CS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC202CS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void sc202cs_init(VI_PIPE ViPipe)
{
    CVI_U8 u8ImgMode = g_pastSC202CS[ViPipe]->u8ImgMode;

	sc202cs_i2c_init(ViPipe);

	//linear mode only
    if (u8ImgMode == SC202CS_MODE_800X600P30) {
        sc202cs_linear_600P30_init(ViPipe);
    }else if (u8ImgMode == SC202CS_MODE_512X320P30_1L_MASTER_10BIT) {
		sc202cs_linear_master_320P30_init(ViPipe);
	}else if(u8ImgMode == SC202CS_MODE_512X320P30_1L_SLAVE_10BIT){
		sc202cs_linear_slave_320P30_init(ViPipe);
	}
	else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support this mode, please check!\n");
		return;
	}
	g_pastSC202CS[ViPipe]->bInit = CVI_TRUE;
}

/* 600P30 */
static void sc202cs_linear_600P30_init(VI_PIPE ViPipe)
{
    sc202cs_write_register(ViPipe, 0x0103, 0x01);
    sc202cs_write_register(ViPipe, 0x0100, 0x00);
    sc202cs_write_register(ViPipe, 0x36e9, 0x80);
    sc202cs_write_register(ViPipe, 0x36eb, 0x14);
    sc202cs_write_register(ViPipe, 0x36e9, 0x24);
    sc202cs_write_register(ViPipe, 0x301f, 0x0a);
    sc202cs_write_register(ViPipe, 0x303f, 0x82);
    sc202cs_write_register(ViPipe, 0x3208, 0x03);
    sc202cs_write_register(ViPipe, 0x3209, 0x20);
    sc202cs_write_register(ViPipe, 0x320a, 0x02);
    sc202cs_write_register(ViPipe, 0x320b, 0x58);
    sc202cs_write_register(ViPipe, 0x3211, 0x02);
    sc202cs_write_register(ViPipe, 0x3213, 0x02);
    sc202cs_write_register(ViPipe, 0x3215, 0x13);
    sc202cs_write_register(ViPipe, 0x3220, 0x17);
    sc202cs_write_register(ViPipe, 0x3250, 0x40);
    sc202cs_write_register(ViPipe, 0x3301, 0xff);
    sc202cs_write_register(ViPipe, 0x3304, 0x68);
    sc202cs_write_register(ViPipe, 0x3306, 0x40);
    sc202cs_write_register(ViPipe, 0x3308, 0x08);
    sc202cs_write_register(ViPipe, 0x3309, 0xa8);
    sc202cs_write_register(ViPipe, 0x330b, 0xb0);
    sc202cs_write_register(ViPipe, 0x330c, 0x18);
    sc202cs_write_register(ViPipe, 0x330d, 0xff);
    sc202cs_write_register(ViPipe, 0x330e, 0x20);
    sc202cs_write_register(ViPipe, 0x331e, 0x59);
    sc202cs_write_register(ViPipe, 0x331f, 0x99);
    sc202cs_write_register(ViPipe, 0x3333, 0x10);
    sc202cs_write_register(ViPipe, 0x335e, 0x06);
    sc202cs_write_register(ViPipe, 0x335f, 0x08);
    sc202cs_write_register(ViPipe, 0x3364, 0x1f);
    sc202cs_write_register(ViPipe, 0x336c, 0xcc);
    sc202cs_write_register(ViPipe, 0x337c, 0x02);
    sc202cs_write_register(ViPipe, 0x337d, 0x0a);
    sc202cs_write_register(ViPipe, 0x337e, 0x80);
    sc202cs_write_register(ViPipe, 0x338f, 0xa0);
    sc202cs_write_register(ViPipe, 0x3390, 0x01);
    sc202cs_write_register(ViPipe, 0x3391, 0x03);
    sc202cs_write_register(ViPipe, 0x3392, 0x1f);
    sc202cs_write_register(ViPipe, 0x3393, 0xff);
    sc202cs_write_register(ViPipe, 0x3394, 0xff);
    sc202cs_write_register(ViPipe, 0x3395, 0xff);
    sc202cs_write_register(ViPipe, 0x33a2, 0x04);
    sc202cs_write_register(ViPipe, 0x33ad, 0x0c);
    sc202cs_write_register(ViPipe, 0x33b1, 0x20);
    sc202cs_write_register(ViPipe, 0x33b3, 0x38);
    sc202cs_write_register(ViPipe, 0x33f9, 0x40);
    sc202cs_write_register(ViPipe, 0x33fb, 0x48);
    sc202cs_write_register(ViPipe, 0x33fc, 0x0f);
    sc202cs_write_register(ViPipe, 0x33fd, 0x1f);
    sc202cs_write_register(ViPipe, 0x349f, 0x03);
    sc202cs_write_register(ViPipe, 0x34a6, 0x03);
    sc202cs_write_register(ViPipe, 0x34a7, 0x1f);
    sc202cs_write_register(ViPipe, 0x34a8, 0x38);
    sc202cs_write_register(ViPipe, 0x34a9, 0x30);
    sc202cs_write_register(ViPipe, 0x34ab, 0xb0);
    sc202cs_write_register(ViPipe, 0x34ad, 0xb0);
    sc202cs_write_register(ViPipe, 0x34f8, 0x1f);
    sc202cs_write_register(ViPipe, 0x34f9, 0x20);
    sc202cs_write_register(ViPipe, 0x3630, 0xa0);
    sc202cs_write_register(ViPipe, 0x3631, 0x92);
    sc202cs_write_register(ViPipe, 0x3632, 0x64);
    sc202cs_write_register(ViPipe, 0x3633, 0x43);
    sc202cs_write_register(ViPipe, 0x3637, 0x49);
    sc202cs_write_register(ViPipe, 0x363a, 0x85);
    sc202cs_write_register(ViPipe, 0x363c, 0x0f);
    sc202cs_write_register(ViPipe, 0x3650, 0x31);
    sc202cs_write_register(ViPipe, 0x3670, 0x0d);
    sc202cs_write_register(ViPipe, 0x3674, 0xc0);
    sc202cs_write_register(ViPipe, 0x3675, 0xa0);
    sc202cs_write_register(ViPipe, 0x3676, 0xa0);
    sc202cs_write_register(ViPipe, 0x3677, 0x92);
    sc202cs_write_register(ViPipe, 0x3678, 0x96);
    sc202cs_write_register(ViPipe, 0x3679, 0x9a);
    sc202cs_write_register(ViPipe, 0x367c, 0x03);
    sc202cs_write_register(ViPipe, 0x367d, 0x0f);
    sc202cs_write_register(ViPipe, 0x367e, 0x01);
    sc202cs_write_register(ViPipe, 0x367f, 0x0f);
    sc202cs_write_register(ViPipe, 0x3698, 0x83);
    sc202cs_write_register(ViPipe, 0x3699, 0x86);
    sc202cs_write_register(ViPipe, 0x369a, 0x8c);
    sc202cs_write_register(ViPipe, 0x369b, 0x94);
    sc202cs_write_register(ViPipe, 0x36a2, 0x01);
    sc202cs_write_register(ViPipe, 0x36a3, 0x03);
    sc202cs_write_register(ViPipe, 0x36a4, 0x07);
    sc202cs_write_register(ViPipe, 0x36ae, 0x0f);
    sc202cs_write_register(ViPipe, 0x36af, 0x1f);
    sc202cs_write_register(ViPipe, 0x36bd, 0x22);
    sc202cs_write_register(ViPipe, 0x36be, 0x22);
    sc202cs_write_register(ViPipe, 0x36bf, 0x22);
    sc202cs_write_register(ViPipe, 0x36d0, 0x01);
    sc202cs_write_register(ViPipe, 0x370f, 0x02);
    sc202cs_write_register(ViPipe, 0x3721, 0x6c);
    sc202cs_write_register(ViPipe, 0x3722, 0x8d);
    sc202cs_write_register(ViPipe, 0x3725, 0xc5);
    sc202cs_write_register(ViPipe, 0x3727, 0x14);
    sc202cs_write_register(ViPipe, 0x3728, 0x04);
    sc202cs_write_register(ViPipe, 0x37b7, 0x04);
    sc202cs_write_register(ViPipe, 0x37b8, 0x04);
    sc202cs_write_register(ViPipe, 0x37b9, 0x06);
    sc202cs_write_register(ViPipe, 0x37bd, 0x07);
    sc202cs_write_register(ViPipe, 0x37be, 0x0f);
    sc202cs_write_register(ViPipe, 0x3901, 0x02);
    sc202cs_write_register(ViPipe, 0x3903, 0x40);
    sc202cs_write_register(ViPipe, 0x3905, 0x8d);
    sc202cs_write_register(ViPipe, 0x3907, 0x00);
    sc202cs_write_register(ViPipe, 0x3908, 0x41);
    sc202cs_write_register(ViPipe, 0x391f, 0x41);
    sc202cs_write_register(ViPipe, 0x3933, 0x80);
    sc202cs_write_register(ViPipe, 0x3934, 0x02);
    sc202cs_write_register(ViPipe, 0x3937, 0x6f);
    sc202cs_write_register(ViPipe, 0x393a, 0x01);
    sc202cs_write_register(ViPipe, 0x393d, 0x01);
    sc202cs_write_register(ViPipe, 0x393e, 0xc0);
    sc202cs_write_register(ViPipe, 0x39dd, 0x41);
    sc202cs_write_register(ViPipe, 0x3e00, 0x00);
    sc202cs_write_register(ViPipe, 0x3e01, 0x4d);
    sc202cs_write_register(ViPipe, 0x3e02, 0xc0);
    sc202cs_write_register(ViPipe, 0x3e09, 0x00);
    sc202cs_write_register(ViPipe, 0x4509, 0x28);
    sc202cs_write_register(ViPipe, 0x450d, 0x61);
    sc202cs_write_register(ViPipe, 0x4800, 0x44);
    sc202cs_write_register(ViPipe, 0x4819, 0x05);
    sc202cs_write_register(ViPipe, 0x481b, 0x03);
    sc202cs_write_register(ViPipe, 0x481d, 0x0a);
    sc202cs_write_register(ViPipe, 0x481f, 0x02);
    sc202cs_write_register(ViPipe, 0x4821, 0x08);
    sc202cs_write_register(ViPipe, 0x4823, 0x03);
    sc202cs_write_register(ViPipe, 0x4825, 0x02);
    sc202cs_write_register(ViPipe, 0x4827, 0x03);
    sc202cs_write_register(ViPipe, 0x4829, 0x04);
    sc202cs_write_register(ViPipe, 0x5000, 0x46);
    sc202cs_write_register(ViPipe, 0x5900, 0xf1);
    sc202cs_write_register(ViPipe, 0x5901, 0x04);
    sc202cs_write_register(ViPipe, 0x0100, 0x01);

    sc202cs_default_reg_init(ViPipe);
    delay_ms(50);
    sc202cs_write_register(ViPipe, 0x0100, 0x01);

    printf("ViPipe:%d, ===SC202CS 600P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}


/* 320P30Master */
static void sc202cs_linear_master_320P30_init(VI_PIPE ViPipe)
{
    sc202cs_write_register(ViPipe, 0x0103, 0x01);
    sc202cs_write_register(ViPipe, 0x0100, 0x00);
    sc202cs_write_register(ViPipe, 0x36e9, 0x80);
    sc202cs_write_register(ViPipe, 0x36eb, 0x14);
    sc202cs_write_register(ViPipe, 0x36e9, 0x24);
    sc202cs_write_register(ViPipe, 0x301f, 0x0a);
    sc202cs_write_register(ViPipe, 0x303f, 0x82);
    sc202cs_write_register(ViPipe, 0x3208, 0x03);
    sc202cs_write_register(ViPipe, 0x3209, 0x20);
    sc202cs_write_register(ViPipe, 0x320a, 0x02);
    sc202cs_write_register(ViPipe, 0x320b, 0x58);
    sc202cs_write_register(ViPipe, 0x3211, 0x02);
    sc202cs_write_register(ViPipe, 0x3213, 0x02);
    sc202cs_write_register(ViPipe, 0x3215, 0x13);
    sc202cs_write_register(ViPipe, 0x3220, 0x17);
    sc202cs_write_register(ViPipe, 0x3250, 0x40);
    sc202cs_write_register(ViPipe, 0x3301, 0xff);
    sc202cs_write_register(ViPipe, 0x3304, 0x68);
    sc202cs_write_register(ViPipe, 0x3306, 0x40);
    sc202cs_write_register(ViPipe, 0x3308, 0x08);
    sc202cs_write_register(ViPipe, 0x3309, 0xa8);
    sc202cs_write_register(ViPipe, 0x330b, 0xb0);
    sc202cs_write_register(ViPipe, 0x330c, 0x18);
    sc202cs_write_register(ViPipe, 0x330d, 0xff);
    sc202cs_write_register(ViPipe, 0x330e, 0x20);
    sc202cs_write_register(ViPipe, 0x331e, 0x59);
    sc202cs_write_register(ViPipe, 0x331f, 0x99);
    sc202cs_write_register(ViPipe, 0x3333, 0x10);
    sc202cs_write_register(ViPipe, 0x335e, 0x06);
    sc202cs_write_register(ViPipe, 0x335f, 0x08);
    sc202cs_write_register(ViPipe, 0x3364, 0x1f);
    sc202cs_write_register(ViPipe, 0x336c, 0xcc);
    sc202cs_write_register(ViPipe, 0x337c, 0x02);
    sc202cs_write_register(ViPipe, 0x337d, 0x0a);
    sc202cs_write_register(ViPipe, 0x337e, 0x80);
    sc202cs_write_register(ViPipe, 0x338f, 0xa0);
    sc202cs_write_register(ViPipe, 0x3390, 0x01);
    sc202cs_write_register(ViPipe, 0x3391, 0x03);
    sc202cs_write_register(ViPipe, 0x3392, 0x1f);
    sc202cs_write_register(ViPipe, 0x3393, 0xff);
    sc202cs_write_register(ViPipe, 0x3394, 0xff);
    sc202cs_write_register(ViPipe, 0x3395, 0xff);
    sc202cs_write_register(ViPipe, 0x33a2, 0x04);
    sc202cs_write_register(ViPipe, 0x33ad, 0x0c);
    sc202cs_write_register(ViPipe, 0x33b1, 0x20);
    sc202cs_write_register(ViPipe, 0x33b3, 0x38);
    sc202cs_write_register(ViPipe, 0x33f9, 0x40);
    sc202cs_write_register(ViPipe, 0x33fb, 0x48);
    sc202cs_write_register(ViPipe, 0x33fc, 0x0f);
    sc202cs_write_register(ViPipe, 0x33fd, 0x1f);
    sc202cs_write_register(ViPipe, 0x349f, 0x03);
    sc202cs_write_register(ViPipe, 0x34a6, 0x03);
    sc202cs_write_register(ViPipe, 0x34a7, 0x1f);
    sc202cs_write_register(ViPipe, 0x34a8, 0x38);
    sc202cs_write_register(ViPipe, 0x34a9, 0x30);
    sc202cs_write_register(ViPipe, 0x34ab, 0xb0);
    sc202cs_write_register(ViPipe, 0x34ad, 0xb0);
    sc202cs_write_register(ViPipe, 0x34f8, 0x1f);
    sc202cs_write_register(ViPipe, 0x34f9, 0x20);
    sc202cs_write_register(ViPipe, 0x3630, 0xa0);
    sc202cs_write_register(ViPipe, 0x3631, 0x92);
    sc202cs_write_register(ViPipe, 0x3632, 0x64);
    sc202cs_write_register(ViPipe, 0x3633, 0x43);
    sc202cs_write_register(ViPipe, 0x3637, 0x49);
    sc202cs_write_register(ViPipe, 0x363a, 0x85);
    sc202cs_write_register(ViPipe, 0x363c, 0x0f);
    sc202cs_write_register(ViPipe, 0x3650, 0x31);
    sc202cs_write_register(ViPipe, 0x3670, 0x0d);
    sc202cs_write_register(ViPipe, 0x3674, 0xc0);
    sc202cs_write_register(ViPipe, 0x3675, 0xa0);
    sc202cs_write_register(ViPipe, 0x3676, 0xa0);
    sc202cs_write_register(ViPipe, 0x3677, 0x92);
    sc202cs_write_register(ViPipe, 0x3678, 0x96);
    sc202cs_write_register(ViPipe, 0x3679, 0x9a);
    sc202cs_write_register(ViPipe, 0x367c, 0x03);
    sc202cs_write_register(ViPipe, 0x367d, 0x0f);
    sc202cs_write_register(ViPipe, 0x367e, 0x01);
    sc202cs_write_register(ViPipe, 0x367f, 0x0f);
    sc202cs_write_register(ViPipe, 0x3698, 0x83);
    sc202cs_write_register(ViPipe, 0x3699, 0x86);
    sc202cs_write_register(ViPipe, 0x369a, 0x8c);
    sc202cs_write_register(ViPipe, 0x369b, 0x94);
    sc202cs_write_register(ViPipe, 0x36a2, 0x01);
    sc202cs_write_register(ViPipe, 0x36a3, 0x03);
    sc202cs_write_register(ViPipe, 0x36a4, 0x07);
    sc202cs_write_register(ViPipe, 0x36ae, 0x0f);
    sc202cs_write_register(ViPipe, 0x36af, 0x1f);
    sc202cs_write_register(ViPipe, 0x36bd, 0x22);
    sc202cs_write_register(ViPipe, 0x36be, 0x22);
    sc202cs_write_register(ViPipe, 0x36bf, 0x22);
    sc202cs_write_register(ViPipe, 0x36d0, 0x01);
    sc202cs_write_register(ViPipe, 0x370f, 0x02);
    sc202cs_write_register(ViPipe, 0x3721, 0x6c);
    sc202cs_write_register(ViPipe, 0x3722, 0x8d);
    sc202cs_write_register(ViPipe, 0x3725, 0xc5);
    sc202cs_write_register(ViPipe, 0x3727, 0x14);
    sc202cs_write_register(ViPipe, 0x3728, 0x04);
    sc202cs_write_register(ViPipe, 0x37b7, 0x04);
    sc202cs_write_register(ViPipe, 0x37b8, 0x04);
    sc202cs_write_register(ViPipe, 0x37b9, 0x06);
    sc202cs_write_register(ViPipe, 0x37bd, 0x07);
    sc202cs_write_register(ViPipe, 0x37be, 0x0f);
    sc202cs_write_register(ViPipe, 0x3901, 0x02);
    sc202cs_write_register(ViPipe, 0x3903, 0x40);
    sc202cs_write_register(ViPipe, 0x3905, 0x8d);
    sc202cs_write_register(ViPipe, 0x3907, 0x00);
    sc202cs_write_register(ViPipe, 0x3908, 0x41);
    sc202cs_write_register(ViPipe, 0x391f, 0x41);
    sc202cs_write_register(ViPipe, 0x3933, 0x80);
    sc202cs_write_register(ViPipe, 0x3934, 0x02);
    sc202cs_write_register(ViPipe, 0x3937, 0x6f);
    sc202cs_write_register(ViPipe, 0x393a, 0x01);
    sc202cs_write_register(ViPipe, 0x393d, 0x01);
    sc202cs_write_register(ViPipe, 0x393e, 0xc0);
    sc202cs_write_register(ViPipe, 0x39dd, 0x41);
    sc202cs_write_register(ViPipe, 0x3e00, 0x00);
    sc202cs_write_register(ViPipe, 0x3e01, 0x4d);
    sc202cs_write_register(ViPipe, 0x3e02, 0xc0);
    sc202cs_write_register(ViPipe, 0x3e09, 0x00);
    sc202cs_write_register(ViPipe, 0x4509, 0x28);
    sc202cs_write_register(ViPipe, 0x450d, 0x61);
    sc202cs_write_register(ViPipe, 0x4800, 0x44);
    sc202cs_write_register(ViPipe, 0x4819, 0x05);
    sc202cs_write_register(ViPipe, 0x481b, 0x03);
    sc202cs_write_register(ViPipe, 0x481d, 0x0a);
    sc202cs_write_register(ViPipe, 0x481f, 0x02);
    sc202cs_write_register(ViPipe, 0x4821, 0x08);
    sc202cs_write_register(ViPipe, 0x4823, 0x03);
    sc202cs_write_register(ViPipe, 0x4825, 0x02);
    sc202cs_write_register(ViPipe, 0x4827, 0x03);
    sc202cs_write_register(ViPipe, 0x4829, 0x04);
    sc202cs_write_register(ViPipe, 0x5000, 0x46);
    sc202cs_write_register(ViPipe, 0x5900, 0xf1);
    sc202cs_write_register(ViPipe, 0x5901, 0x04);
    //master
    sc202cs_write_register(ViPipe, 0x300a, 0x40);//bit[6]:fsync output
    sc202cs_write_register(ViPipe, 0x3032, 0x80);//bit[7]:output fsync

    sc202cs_write_register(ViPipe, 0x0100, 0x01);

    sc202cs_default_reg_init(ViPipe);
    delay_ms(50);
    sc202cs_write_register(ViPipe, 0x0100, 0x01);
    printf("ViPipe:%d, ===SC202CS 320P 30fps 10bit LINEAR master Init OK!===\n", ViPipe);
}


/* 320P30Slave */
static void sc202cs_linear_slave_320P30_init(VI_PIPE ViPipe)
{
    sc202cs_write_register(ViPipe, 0x0103, 0x01);
    sc202cs_write_register(ViPipe, 0x0100, 0x00);
    sc202cs_write_register(ViPipe, 0x36e9, 0x80);
    sc202cs_write_register(ViPipe, 0x36eb, 0x14);
    sc202cs_write_register(ViPipe, 0x36e9, 0x24);
    sc202cs_write_register(ViPipe, 0x301f, 0x0a);
    sc202cs_write_register(ViPipe, 0x303f, 0x82);
    sc202cs_write_register(ViPipe, 0x3208, 0x03);
    sc202cs_write_register(ViPipe, 0x3209, 0x20);
    sc202cs_write_register(ViPipe, 0x320a, 0x02);
    sc202cs_write_register(ViPipe, 0x320b, 0x58);
    sc202cs_write_register(ViPipe, 0x3211, 0x02);
    sc202cs_write_register(ViPipe, 0x3213, 0x02);
    sc202cs_write_register(ViPipe, 0x3215, 0x13);
    sc202cs_write_register(ViPipe, 0x3220, 0x17);
    sc202cs_write_register(ViPipe, 0x3250, 0x40);
    sc202cs_write_register(ViPipe, 0x3301, 0xff);
    sc202cs_write_register(ViPipe, 0x3304, 0x68);
    sc202cs_write_register(ViPipe, 0x3306, 0x40);
    sc202cs_write_register(ViPipe, 0x3308, 0x08);
    sc202cs_write_register(ViPipe, 0x3309, 0xa8);
    sc202cs_write_register(ViPipe, 0x330b, 0xb0);
    sc202cs_write_register(ViPipe, 0x330c, 0x18);
    sc202cs_write_register(ViPipe, 0x330d, 0xff);
    sc202cs_write_register(ViPipe, 0x330e, 0x20);
    sc202cs_write_register(ViPipe, 0x331e, 0x59);
    sc202cs_write_register(ViPipe, 0x331f, 0x99);
    sc202cs_write_register(ViPipe, 0x3333, 0x10);
    sc202cs_write_register(ViPipe, 0x335e, 0x06);
    sc202cs_write_register(ViPipe, 0x335f, 0x08);
    sc202cs_write_register(ViPipe, 0x3364, 0x1f);
    sc202cs_write_register(ViPipe, 0x336c, 0xcc);
    sc202cs_write_register(ViPipe, 0x337c, 0x02);
    sc202cs_write_register(ViPipe, 0x337d, 0x0a);
    sc202cs_write_register(ViPipe, 0x337e, 0x80);
    sc202cs_write_register(ViPipe, 0x338f, 0xa0);
    sc202cs_write_register(ViPipe, 0x3390, 0x01);
    sc202cs_write_register(ViPipe, 0x3391, 0x03);
    sc202cs_write_register(ViPipe, 0x3392, 0x1f);
    sc202cs_write_register(ViPipe, 0x3393, 0xff);
    sc202cs_write_register(ViPipe, 0x3394, 0xff);
    sc202cs_write_register(ViPipe, 0x3395, 0xff);
    sc202cs_write_register(ViPipe, 0x33a2, 0x04);
    sc202cs_write_register(ViPipe, 0x33ad, 0x0c);
    sc202cs_write_register(ViPipe, 0x33b1, 0x20);
    sc202cs_write_register(ViPipe, 0x33b3, 0x38);
    sc202cs_write_register(ViPipe, 0x33f9, 0x40);
    sc202cs_write_register(ViPipe, 0x33fb, 0x48);
    sc202cs_write_register(ViPipe, 0x33fc, 0x0f);
    sc202cs_write_register(ViPipe, 0x33fd, 0x1f);
    sc202cs_write_register(ViPipe, 0x349f, 0x03);
    sc202cs_write_register(ViPipe, 0x34a6, 0x03);
    sc202cs_write_register(ViPipe, 0x34a7, 0x1f);
    sc202cs_write_register(ViPipe, 0x34a8, 0x38);
    sc202cs_write_register(ViPipe, 0x34a9, 0x30);
    sc202cs_write_register(ViPipe, 0x34ab, 0xb0);
    sc202cs_write_register(ViPipe, 0x34ad, 0xb0);
    sc202cs_write_register(ViPipe, 0x34f8, 0x1f);
    sc202cs_write_register(ViPipe, 0x34f9, 0x20);
    sc202cs_write_register(ViPipe, 0x3630, 0xa0);
    sc202cs_write_register(ViPipe, 0x3631, 0x92);
    sc202cs_write_register(ViPipe, 0x3632, 0x64);
    sc202cs_write_register(ViPipe, 0x3633, 0x43);
    sc202cs_write_register(ViPipe, 0x3637, 0x49);
    sc202cs_write_register(ViPipe, 0x363a, 0x85);
    sc202cs_write_register(ViPipe, 0x363c, 0x0f);
    sc202cs_write_register(ViPipe, 0x3650, 0x31);
    sc202cs_write_register(ViPipe, 0x3670, 0x0d);
    sc202cs_write_register(ViPipe, 0x3674, 0xc0);
    sc202cs_write_register(ViPipe, 0x3675, 0xa0);
    sc202cs_write_register(ViPipe, 0x3676, 0xa0);
    sc202cs_write_register(ViPipe, 0x3677, 0x92);
    sc202cs_write_register(ViPipe, 0x3678, 0x96);
    sc202cs_write_register(ViPipe, 0x3679, 0x9a);
    sc202cs_write_register(ViPipe, 0x367c, 0x03);
    sc202cs_write_register(ViPipe, 0x367d, 0x0f);
    sc202cs_write_register(ViPipe, 0x367e, 0x01);
    sc202cs_write_register(ViPipe, 0x367f, 0x0f);
    sc202cs_write_register(ViPipe, 0x3698, 0x83);
    sc202cs_write_register(ViPipe, 0x3699, 0x86);
    sc202cs_write_register(ViPipe, 0x369a, 0x8c);
    sc202cs_write_register(ViPipe, 0x369b, 0x94);
    sc202cs_write_register(ViPipe, 0x36a2, 0x01);
    sc202cs_write_register(ViPipe, 0x36a3, 0x03);
    sc202cs_write_register(ViPipe, 0x36a4, 0x07);
    sc202cs_write_register(ViPipe, 0x36ae, 0x0f);
    sc202cs_write_register(ViPipe, 0x36af, 0x1f);
    sc202cs_write_register(ViPipe, 0x36bd, 0x22);
    sc202cs_write_register(ViPipe, 0x36be, 0x22);
    sc202cs_write_register(ViPipe, 0x36bf, 0x22);
    sc202cs_write_register(ViPipe, 0x36d0, 0x01);
    sc202cs_write_register(ViPipe, 0x370f, 0x02);
    sc202cs_write_register(ViPipe, 0x3721, 0x6c);
    sc202cs_write_register(ViPipe, 0x3722, 0x8d);
    sc202cs_write_register(ViPipe, 0x3725, 0xc5);
    sc202cs_write_register(ViPipe, 0x3727, 0x14);
    sc202cs_write_register(ViPipe, 0x3728, 0x04);
    sc202cs_write_register(ViPipe, 0x37b7, 0x04);
    sc202cs_write_register(ViPipe, 0x37b8, 0x04);
    sc202cs_write_register(ViPipe, 0x37b9, 0x06);
    sc202cs_write_register(ViPipe, 0x37bd, 0x07);
    sc202cs_write_register(ViPipe, 0x37be, 0x0f);
    sc202cs_write_register(ViPipe, 0x3901, 0x02);
    sc202cs_write_register(ViPipe, 0x3903, 0x40);
    sc202cs_write_register(ViPipe, 0x3905, 0x8d);
    sc202cs_write_register(ViPipe, 0x3907, 0x00);
    sc202cs_write_register(ViPipe, 0x3908, 0x41);
    sc202cs_write_register(ViPipe, 0x391f, 0x41);
    sc202cs_write_register(ViPipe, 0x3933, 0x80);
    sc202cs_write_register(ViPipe, 0x3934, 0x02);
    sc202cs_write_register(ViPipe, 0x3937, 0x6f);
    sc202cs_write_register(ViPipe, 0x393a, 0x01);
    sc202cs_write_register(ViPipe, 0x393d, 0x01);
    sc202cs_write_register(ViPipe, 0x393e, 0xc0);
    sc202cs_write_register(ViPipe, 0x39dd, 0x41);
    sc202cs_write_register(ViPipe, 0x3e00, 0x00);
    sc202cs_write_register(ViPipe, 0x3e01, 0x4d);
    sc202cs_write_register(ViPipe, 0x3e02, 0xc0);
    sc202cs_write_register(ViPipe, 0x3e09, 0x00);
    sc202cs_write_register(ViPipe, 0x4509, 0x28);
    sc202cs_write_register(ViPipe, 0x450d, 0x61);
    sc202cs_write_register(ViPipe, 0x4800, 0x44);
    sc202cs_write_register(ViPipe, 0x4819, 0x05);
    sc202cs_write_register(ViPipe, 0x481b, 0x03);
    sc202cs_write_register(ViPipe, 0x481d, 0x0a);
    sc202cs_write_register(ViPipe, 0x481f, 0x02);
    sc202cs_write_register(ViPipe, 0x4821, 0x08);
    sc202cs_write_register(ViPipe, 0x4823, 0x03);
    sc202cs_write_register(ViPipe, 0x4825, 0x02);
    sc202cs_write_register(ViPipe, 0x4827, 0x03);
    sc202cs_write_register(ViPipe, 0x4829, 0x04);
    sc202cs_write_register(ViPipe, 0x5000, 0x46);
    sc202cs_write_register(ViPipe, 0x5900, 0xf1);
    sc202cs_write_register(ViPipe, 0x5901, 0x04);
    //slave
    sc202cs_write_register(ViPipe, 0x3222, 0x02);//bit[1]:slave en
    sc202cs_write_register(ViPipe, 0x3230, 0x00);
    sc202cs_write_register(ViPipe, 0x3231, 0x04); //RB rows = 0x4
    sc202cs_write_register(ViPipe, 0x322e, 0x04); //vts - RB rows
    sc202cs_write_register(ViPipe, 0x322f, 0xde);

    sc202cs_write_register(ViPipe, 0x0100, 0x01);

    sc202cs_default_reg_init(ViPipe);
    delay_ms(50);
    sc202cs_write_register(ViPipe, 0x0100, 0x01);
    printf("ViPipe:%d, ===SC202CS 320P 30fps 10bit LINEAR slave Init OK!===\n", ViPipe);
}