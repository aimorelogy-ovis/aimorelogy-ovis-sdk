#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "mis40h1_cmos_ex.h"
#include "sensor_i2c.h"
static void mis40h1_linear_1440p25_2l_init(VI_PIPE ViPipe);

CVI_U8 mis40h1_i2c_addr = 0x30;        /* I2C Address of MIS40H1 */
const CVI_U32 mis40h1_addr_byte = 2;
const CVI_U32 mis40h1_data_byte = 1;

int mis40h1_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunMIS40H1_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunMIS40H1_AddrInfo[ViPipe].s8I2cAddr);
}

int mis40h1_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunMIS40H1_BusInfo[ViPipe].s8I2cDev);
}

int mis40h1_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunMIS40H1_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunMIS40H1_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							mis40h1_addr_byte, mis40h1_data_byte);
}

int mis40h1_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunMIS40H1_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunMIS40H1_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							mis40h1_addr_byte, (CVI_U32)data, mis40h1_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void mis40h1_standby(VI_PIPE ViPipe)
{
	mis40h1_write_register(ViPipe, 0x3006, 0x02);
}

void mis40h1_restart(VI_PIPE ViPipe)
{
	mis40h1_write_register(ViPipe, 0x3006, 0x01);
}

void mis40h1_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastMIS40H1[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		mis40h1_write_register(ViPipe,
				g_pastMIS40H1[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastMIS40H1[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define MIS40H1_CHIP_ID_HI_ADDR		0x3000
#define MIS40H1_CHIP_ID_LO_ADDR		0x3001
#define MIS40H1_CHIP_ID			0x40d1

int mis40h1_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	if (mis40h1_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	delay_ms(5);

	nVal = mis40h1_read_register(ViPipe, MIS40H1_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = mis40h1_read_register(ViPipe, MIS40H1_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != MIS40H1_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}
#define MIS40H1_DCG0_ADDR	0x2003
#define MIS40H1_DCG1_ADDR	0x2004

void mis40h1_dcg_init(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 dcg_value;

	if (mis40h1_i2c_init(ViPipe) != CVI_SUCCESS)
		return

	delay_ms(5);
	nVal = mis40h1_read_register(ViPipe, MIS40H1_DCG0_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read mis40h1 dcg value error.\n");
		return;
	}
	dcg_value = (nVal & 0xFF);
	nVal = mis40h1_read_register(ViPipe, MIS40H1_DCG1_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read mis40h1 dcg value error.\n");
		return;
	}
	dcg_value = dcg_value + ((nVal & 0xFC) >> 2);

	g_au16MIS40H1_DcgValue[ViPipe] = dcg_value;
	CVI_TRACE_SNS(CVI_DBG_ERR, "vipipe[%d], Mis40h1 default dcg value = %d\n", ViPipe, dcg_value);

}

void mis40h1_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode;

	mis40h1_i2c_init(ViPipe);
	mis40h1_dcg_init(ViPipe);

	u8ImgMode = g_pastMIS40H1[ViPipe]->u8ImgMode;

	if (u8ImgMode == MIS40H1_MODE_1440P25_2L) {
		mis40h1_linear_1440p25_2l_init(ViPipe);
	}

	g_pastMIS40H1[ViPipe]->bInit = CVI_TRUE;
}

void mis40h1_exit(VI_PIPE ViPipe)
{
	mis40h1_i2c_exit(ViPipe);
}

/* 1440p20 */
static void mis40h1_linear_1440p25_2l_init(VI_PIPE ViPipe)
{

	mis40h1_write_register(ViPipe, 0x3006, 0x01);
	delay_ms(100);
	mis40h1_write_register(ViPipe, 0x3006, 0x02);
	mis40h1_write_register(ViPipe, 0x3304, 0xb0);
	mis40h1_write_register(ViPipe, 0x3305, 0xca);
	mis40h1_write_register(ViPipe, 0x3023, 0x2b);
	mis40h1_write_register(ViPipe, 0x3111, 0x80);//size
	mis40h1_write_register(ViPipe, 0x3110, 0x0c);
	mis40h1_write_register(ViPipe, 0x310e, 0x07);
	mis40h1_write_register(ViPipe, 0x310f, 0xbc); //vts
	mis40h1_write_register(ViPipe, 0x3113, 0x2C);
	mis40h1_write_register(ViPipe, 0x3112, 0x00);
	mis40h1_write_register(ViPipe, 0x3115, 0xCB);
	mis40h1_write_register(ViPipe, 0x3114, 0x05);
	mis40h1_write_register(ViPipe, 0x3117, 0x44);
	mis40h1_write_register(ViPipe, 0x3116, 0x00);
	mis40h1_write_register(ViPipe, 0x3119, 0x43);
	mis40h1_write_register(ViPipe, 0x3118, 0x0a);
	mis40h1_write_register(ViPipe, 0x3120, 0x78);
	mis40h1_write_register(ViPipe, 0x3123, 0x12);
	mis40h1_write_register(ViPipe, 0x3306, 0x25);
	mis40h1_write_register(ViPipe, 0x3605, 0xc5);
	mis40h1_write_register(ViPipe, 0x360d, 0xc5);
	mis40h1_write_register(ViPipe, 0x3615, 0x58);
	mis40h1_write_register(ViPipe, 0x361d, 0xc5);
	mis40h1_write_register(ViPipe, 0x3621, 0xc5);
	mis40h1_write_register(ViPipe, 0x3629, 0xc5);
	mis40h1_write_register(ViPipe, 0x362d, 0xb9);
	mis40h1_write_register(ViPipe, 0x363d, 0xF7);
	mis40h1_write_register(ViPipe, 0x363f, 0x7d);
	mis40h1_write_register(ViPipe, 0x3641, 0xB0);
	mis40h1_write_register(ViPipe, 0x365d, 0x08);
	mis40h1_write_register(ViPipe, 0x365f, 0xb9);
	mis40h1_write_register(ViPipe, 0x3661, 0xa2);
	mis40h1_write_register(ViPipe, 0x367f, 0xad);
	mis40h1_write_register(ViPipe, 0x3681, 0xa0);
	mis40h1_write_register(ViPipe, 0x368b, 0xad);
	mis40h1_write_register(ViPipe, 0x368d, 0xc5);
	mis40h1_write_register(ViPipe, 0x3692, 0x03);
	mis40h1_write_register(ViPipe, 0x3693, 0x39);
	mis40h1_write_register(ViPipe, 0x3695, 0xbf);
	mis40h1_write_register(ViPipe, 0x36b9, 0xb9);
	mis40h1_write_register(ViPipe, 0x3703, 0xbf);
	mis40h1_write_register(ViPipe, 0x3705, 0xc5);
	mis40h1_write_register(ViPipe, 0x3713, 0xdd);
	mis40h1_write_register(ViPipe, 0x371e, 0x66);
	mis40h1_write_register(ViPipe, 0x3720, 0x95);
	mis40h1_write_register(ViPipe, 0x3724, 0x72);
	mis40h1_write_register(ViPipe, 0x3728, 0x89);
	mis40h1_write_register(ViPipe, 0x3730, 0x42);
	mis40h1_write_register(ViPipe, 0x3740, 0x89);
	mis40h1_write_register(ViPipe, 0x375c, 0xA8);
	mis40h1_write_register(ViPipe, 0x3905, 0x0e);
	mis40h1_write_register(ViPipe, 0x3c03, 0x03);
	mis40h1_write_register(ViPipe, 0x3c39, 0x03);
	mis40h1_write_register(ViPipe, 0x4102, 0x13);
	mis40h1_write_register(ViPipe, 0x410e, 0x02);
	mis40h1_write_register(ViPipe, 0x4303, 0x13);
	mis40h1_write_register(ViPipe, 0x432e, 0x00);
	mis40h1_write_register(ViPipe, 0x432f, 0x80);
	mis40h1_write_register(ViPipe, 0x4330, 0x00);
	mis40h1_write_register(ViPipe, 0x4331, 0x80);
	mis40h1_write_register(ViPipe, 0x4332, 0x00);
	mis40h1_write_register(ViPipe, 0x4333, 0x80);
	mis40h1_write_register(ViPipe, 0x4334, 0x00);
	mis40h1_write_register(ViPipe, 0x4335, 0x80);
	mis40h1_write_register(ViPipe, 0x4402, 0x7f);
	mis40h1_write_register(ViPipe, 0x390c, 0x1b);//  BLC亮暗切换闪烁，DBLC 不过IIR filter直通  20250114
	mis40h1_write_register(ViPipe, 0x390d, 0x1b);
	mis40h1_write_register(ViPipe, 0x390e, 0x1b);
	mis40h1_write_register(ViPipe, 0x390f, 0x1b);
	mis40h1_write_register(ViPipe, 0x3910, 0x1b);
	mis40h1_write_register(ViPipe, 0x3911, 0x1b);
	mis40h1_write_register(ViPipe, 0x3a0e, 0x0f);
	mis40h1_write_register(ViPipe, 0x4102, 0x13);
	mis40h1_write_register(ViPipe, 0x410e, 0x02);
	mis40h1_write_register(ViPipe, 0x410f, 0x39);
	mis40h1_write_register(ViPipe, 0x4303, 0x13);
	mis40h1_write_register(ViPipe, 0x4304, 0x39);
	mis40h1_write_register(ViPipe, 0x4309, 0x02);
	mis40h1_write_register(ViPipe, 0x433c, 0x02);
	mis40h1_write_register(ViPipe, 0x436f, 0x02);
	mis40h1_write_register(ViPipe, 0x367C, 0x01);
	mis40h1_write_register(ViPipe, 0x367D, 0xE8);//高亮竖线
	mis40h1_write_register(ViPipe, 0x3a1e, 0x7f);//优化干扰横纹20250109 hql
	mis40h1_write_register(ViPipe, 0x3a1f, 0x00);
	mis40h1_write_register(ViPipe, 0x3a20, 0xff);
	mis40h1_write_register(ViPipe, 0x3915, 0x00);
	mis40h1_write_register(ViPipe, 0x3913, 0x1c); //小米手电 高光中心短横线 eclp电压  20250123  hql
	mis40h1_write_register(ViPipe, 0x3914, 0x1c);
	mis40h1_write_register(ViPipe, 0x3759, 0x12);  //优化shading 20250909 hql
	mis40h1_write_register(ViPipe, 0x375c, 0x70);
	mis40h1_write_register(ViPipe, 0x3a05, 0x63);
	mis40h1_write_register(ViPipe, 0x3715, 0x78);
	mis40h1_write_register(ViPipe, 0x363b, 0xc0);
	mis40h1_write_register(ViPipe, 0x363d, 0xfc);
	mis40h1_write_register(ViPipe, 0x3641, 0xb5);
	mis40h1_write_register(ViPipe, 0x3902, 0x1a);
	mis40h1_write_register(ViPipe, 0x3502, 0x01);  //关闭TS OTP矫正，调整TS时序移动到VS区间  20251017  hql
	mis40h1_write_register(ViPipe, 0x350a, 0x01);
	mis40h1_write_register(ViPipe, 0x350b, 0xd0);
	mis40h1_write_register(ViPipe, 0x350c, 0x2b);
	mis40h1_write_register(ViPipe, 0x3a0b, 0xaa); //宇视sensor头板 横纹 20251024
	mis40h1_write_register(ViPipe, 0x3a22, 0x00);
	mis40h1_write_register(ViPipe, 0x3a21, 0x05);
	mis40h1_write_register(ViPipe, 0x3a08, 0xe6); //优化曝光不满横纹 20251223 hql
	mis40h1_write_register(ViPipe, 0x3a09, 0x67);  //曝光未满存在分层_tx_shading 20251024
	mis40h1_write_register(ViPipe, 0x3615, 0x30);
	mis40h1_write_register(ViPipe, 0x3031, 0x0c);//生效
	mis40h1_write_register(ViPipe, 0x3008, 0x01);
	mis40h1_write_register(ViPipe, 0x3006, 0x00);
	mis40h1_write_register(ViPipe, 0x3c1a, 0x01);
	delay_ms(100);
	mis40h1_write_register(ViPipe, 0x300c, 0x81);
	mis40h1_default_reg_init(ViPipe);


	printf("ViPipe:%d,===MIS40H1 1440P 25fps 1lane 10bit LINE Init OK!===\n", ViPipe);
}