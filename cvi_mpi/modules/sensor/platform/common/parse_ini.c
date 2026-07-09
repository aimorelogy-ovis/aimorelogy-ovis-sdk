#include "ini.h"
#include "platform_sensor.h"

#define INI_FILE_PATH	"/mnt/data/sensor_cfg.ini"
#define INI_DEF_PATH	"/mnt/system/usr/bin/sensor_cfg.ini"
#define SNSCFGPATH_SIZE 100

static CVI_CHAR g_snsCfgPath[SNSCFGPATH_SIZE];

static SNS_INI_CFG_S sns_ini_cfg = {
	.devNum    = 1,
	.enSnsMode    = 0xff,
	.enSnsType[0] = CVI_SNS_TYPE_BUTT,
	.s32BusId[0]  = 3,
	.s32SnsI2cAddr[0] = -1,
	.s32SnsI2cAddr[1] = -1,
	.MipiDev[0]   = 0xFF,
	.MipiDev[1]   = 0xFF,
	.MipiDev[2]   = 0xFF,
	.u8Hsettle[0] = 0,
	.bHsettlen[0] = 0,
	.s32RstPort[0] = 0,
	.s32RstPin[0]  = 0,
	.s32RstPol[0]  = 0,
};

/*=== Source section parser handler begin === */
static void parse_source_devnum(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	int devno = atoi(value);

	(CVI_VOID) param0;
	(CVI_VOID) param1;
	(CVI_VOID) param2;

	SNS_DBG_PRT("devNum =  %s\n", value);

	if (devno >= 1 && devno <= VI_MAX_DEV_NUM)
		cfg->devNum = devno;
	else
		cfg->devNum = 1;
}

static void parse_source_enmode(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	int devmode = atoi(value);

	(CVI_VOID) param0;
	(CVI_VOID) param1;
	(CVI_VOID) param2;

	SNS_DBG_PRT("devmode =  %s\n", value);

	if (devmode >= 0 && devmode <= 6)
		cfg->enSnsMode = devmode;
	else
		cfg->enSnsMode = 0;
}
/* === Source section parser handler end === */

/* === Sensor section parser handler begin === */
static int parse_lane_id(CVI_S16 *LaneId, const char *value)
{
	char buf[8];
	int offset = 0, idx = 0, k;

	for (k = 0; k < MIPI_LANE_NUM * 6; k++) {
		/* find next ',' */
		if (value[k] == ',' || value[k] == '\0') {
			if (k == offset) {
				SNS_DBG_PRT("lane_id parse error, is the format correct?\n");
				return -1;
			}
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &value[offset], k - offset);
			buf[k-offset] = '\0';
			LaneId[idx++] = atoi(buf);
			offset = k + 1;
		}

		if (value[k] == '\0' || idx == MIPI_LANE_NUM + 1)
			break;
	}

	if (k == 60) {
		SNS_DBG_PRT("lane_id parse error, is the format correct?\n");
		return -1;
	}

	return CVI_FAILURE;
}

static int parse_func_id(CVI_S16 *FuncId, const char *value)
{
	char buf[8];
	int offset = 0, idx = 0, k;

	for (k = 0; k < TTL_PIN_FUNC_NUM * 6; k++) {
		/* find next ',' */
		if (value[k] == ',' || value[k] == '\0') {
			if (k == offset) {
				SNS_DBG_PRT("func_id parse error, is the format correct?\n");
				return -1;
			}
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &value[offset], k - offset);
			buf[k-offset] = '\0';
			FuncId[idx++] = atoi(buf);
			offset = k + 1;
		}

		if (value[k] == '\0' || idx == TTL_PIN_FUNC_NUM + 1)
			break;
	}

	if (k == 60) {
		SNS_DBG_PRT("func_id parse error, is the format correct?\n");
		return -1;
	}

	return CVI_FAILURE;
}

static int parse_pn_swap(CVI_S8 *PNSwap, const char *value)
{
	char buf[8];
	int offset = 0, idx = 0, k;

	for (k = 0; k < 30; k++) {
		/* find next ',' */
		if (value[k] == ',' || value[k] == '\0') {
			if (k == offset) {
				SNS_DBG_PRT("lane_id parse error, is the format correct?\n");
				return -1;
			}
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &value[offset], k - offset);
			buf[k-offset] = '\0';
			PNSwap[idx++] = atoi(buf);
			offset = k + 1;
		}

		if (value[k] == '\0' || idx == 5)
			break;
	}

	if (k == 30) {
		SNS_DBG_PRT("lane_id parse error, is the format correct?\n");
		return -1;
	}

	return CVI_FAILURE;
}

static int parse_switch_gpio(CVI_S32 *gpio, const char *value)
{
	char buf[8];
	int offset = 0, idx = 0, k;

	for (k = 0; k < SWITCH_GPIO_NUM * 6; k++) {
		/* find next ',' */
		if (value[k] == ',' || value[k] == '\0') {
			if (k == offset) {
				SNS_DBG_PRT("gpio parse error, is the format correct?\n");
				return -1;
			}
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &value[offset], k - offset);
			buf[k-offset] = '\0';
			gpio[idx++] = atoi(buf);
			offset = k + 1;
		}

		if (value[k] == '\0' || idx == MIPI_LANE_NUM + 1)
			break;
	}

	if (k == 60) {
		SNS_DBG_PRT("gpio parse error, is the format correct?\n");
		return -1;
	}

	return CVI_FAILURE;
}

static void parse_sensor_name(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
#define NAME_SIZE 20
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("sensor_%d =  %s\n", index, value);

	char *endptr;
	unsigned long num = strtoul(value, &endptr, 0);

	if (num == CVI_SNS_TYPE_BUTT) {
		SNS_DBG_PRT("Sensor name parse error, will set IMX327\n");
		cfg->enSnsType[index] = SONY_IMX327_MIPI_2M_30FPS_12BIT;
		return;
	}
	if (endptr == value) {
		SNS_DBG_PRT("Have no any numbers\n");
		return;
	}
	if (*endptr!= '\0') {
		SNS_DBG_PRT("Non-numeric characters encountered in conversion :%s\n", endptr);
		return;
	}
	cfg->enSnsType[index] = num;
}

static void parse_sensor_busid(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("bus_id =  %s\n", value);
	cfg->s32BusId[index] = atoi(value);
}

static void parse_sensor_i2caddr(SNS_INI_CFG_S *cfg, const char *value,
					CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("sns_i2c_addr =  %s\n", value);
	cfg->s32SnsI2cAddr[index] = atoi(value);
}

static void parse_sensor_mipidev(SNS_INI_CFG_S *cfg, const char *value,
					CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("mipi_dev =  %s\n", value);
	cfg->MipiDev[index] = atoi(value);
}

static void parse_sensor_laneid(SNS_INI_CFG_S *cfg, const char *value,
					CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("Lane_id =  %s\n", value);
	parse_lane_id(cfg->as16LaneId[index], value);
}

static void parse_sensor_funcid(SNS_INI_CFG_S *cfg, const char *value,
					CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("Func_id =  %s\n", value);
	parse_func_id(cfg->as16FuncId[index], value);
}

static void parse_sensor_pnswap(SNS_INI_CFG_S *cfg, const char *value,
					CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("pn_swap =  %s\n", value);
	parse_pn_swap(cfg->as8PNSwap[index], value);
}

static void parse_sensor_hwsync(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("hw_sync =  %s\n", value);
	cfg->u8HwSync[index] = atoi(value);
}

static void parse_sensor_mclken(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("mclk_en =  %s\n", value);
	cfg->stMclkAttr[index].bMclkEn = atoi(value);
}

static void parse_sensor_mclk(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("mclk =  %s\n", value);
	cfg->stMclkAttr[index].u8Mclk = atoi(value);
}

static void parse_sensor_hsettlen(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("hs_settle enable =  %s\n", value);
	cfg->bHsettlen[index] = atoi(value);
}

static void parse_sensor_hsettle(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("hs_settle =  %s\n", value);
	cfg->u8Hsettle[index] = atoi(value);
}

static void parse_sensor_orien(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("orien =  %s\n", value);
	cfg->u8Orien[index] = atoi(value);
}

static void parse_sensor_rstport(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	if (atoi(value) == 0) {
		SNS_DBG_PRT("RST port A\n");
	} else if (atoi(value) == 1) {
		SNS_DBG_PRT("RST port B\n");
	} else if (atoi(value) == 2) {
		SNS_DBG_PRT("RST port C\n");
	} else if (atoi(value) == 3) {
		SNS_DBG_PRT("RST port D\n");
	}
	cfg->s32RstPort[index] = atoi(value);
}

static void parse_sensor_rstpin(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("RST pin =  %s\n", value);
	cfg->s32RstPin[index] = atoi(value);
}

static void parse_sensor_rstpol(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("RST pol =  %s\n", value);
	cfg->s32RstPol[index] = atoi(value);
}

static void parse_sensor_switchport(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("switch port =  %s\n", value);
	parse_switch_gpio(cfg->s32SwitchPort[index], value);
}

static void parse_sensor_switchpin(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("switch pin =  %s\n", value);
	parse_switch_gpio(cfg->s32SwitchPin[index], value);
}

static void parse_sensor_switchpol(SNS_INI_CFG_S *cfg, const char *value,
				CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("switch pol =  %s\n", value);
	parse_switch_gpio(cfg->s32SwitchPol[index], value);
}

static void parse_sensor_muxdev(SNS_INI_CFG_S *cfg, const char *value,
	CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("muxdev =  %s\n", value);
	cfg->u8MuxDev[index] = atoi(value);
}

static void parse_sensor_attachdev(SNS_INI_CFG_S *cfg, const char *value,
	CVI_U32 param0, CVI_U32 param1, CVI_U32 param2)
{
	CVI_U32 index = param0;

	(CVI_VOID) param1;
	(CVI_VOID) param2;
	SNS_DBG_PRT("attach_dev =  %s\n", value);
	cfg->u8AttachDev[index] = atoi(value);
}

/* === Sensor section parser handler end === */
typedef CVI_VOID(*parser)(SNS_INI_CFG_S *cfg, const char *value,
		CVI_U32 param0, CVI_U32 param1, CVI_U32 param2);

typedef struct _INI_HDLR_S {
	const char name[16];
	CVI_U32 param0;
	CVI_U32 param1;
	CVI_U32 param2;
	parser pfnJob;
} INI_HDLR_S;

typedef enum _INI_SOURCE_NAME_E {
	INI_SOURCE_DEVNUM = 0,
	INI_SOURCE_ENMODE,
	INI_SOURCE_NUM,
} INI_SOURCE_NAME_E;

typedef enum _INI_SENSOR_NAME_E {
	INI_SENSOR_NAME = 0,
	INI_SENSOR_BUSID,
	INI_SENSOR_I2CADDR,
	INI_SENSOR_MIPIDEV,
	INI_SENSOR_LANEID,
	INI_SENSOR_FUNCID,
	INI_SENSOR_PNSWAP,
	INI_SENSOR_HWSYNC,
	INI_SENSOR_MCLKEN,
	INI_SENSOR_MCLK,
	INI_SENSOR_SETTLEEN,
	INI_SENSOR_SETTLE,
	INI_SENSOR_ORIEN,
	INI_SENSOR_RSTPORT,
	INI_SENSOR_RSTPIN,
	INI_SENSOR_ACTIVE,
	INI_SENSOR_MUXDEV,
	INI_SENSOR_ATTACHDEV,
	INI_SENSOR_SWITCHPORT,
	INI_SENSOR_SWITCHPIN,
	INI_SENSOR_SWITCHPOL,
	INI_SENSOR_NUM,
} INI_SENSOR_NAME_E;

INI_HDLR_S stSectionSource[INI_SOURCE_NUM] = {
	[INI_SOURCE_DEVNUM] = {"dev_num", 0, 0, 0, parse_source_devnum},
	[INI_SOURCE_ENMODE] = {"en_mode", 0, 0, 0, parse_source_enmode},
};

INI_HDLR_S stSectionSensor[INI_SENSOR_NUM] = {
	[INI_SENSOR_NAME] = {"name", 0, 0, 0, parse_sensor_name},
	[INI_SENSOR_BUSID] = {"bus_id", 0, 0, 0, parse_sensor_busid},
	[INI_SENSOR_I2CADDR] = {"sns_i2c_addr", 0, 0, 0, parse_sensor_i2caddr},
	[INI_SENSOR_MIPIDEV] = {"mipi_dev", 0, 0, 0, parse_sensor_mipidev},
	[INI_SENSOR_LANEID] = {"lane_id", 0, 0, 0, parse_sensor_laneid},
	[INI_SENSOR_FUNCID] = {"func_id", 0, 0, 0, parse_sensor_funcid},
	[INI_SENSOR_PNSWAP] = {"pn_swap", 0, 0, 0, parse_sensor_pnswap},
	[INI_SENSOR_HWSYNC] = {"hw_sync", 0, 0, 0, parse_sensor_hwsync},
	[INI_SENSOR_MCLKEN] = {"mclk_en", 0, 0, 0, parse_sensor_mclken},
	[INI_SENSOR_MCLK] = {"mclk", 0, 0, 0, parse_sensor_mclk},
	[INI_SENSOR_SETTLEEN] = {"hs_settle_en", 0, 0, 0, parse_sensor_hsettlen},
	[INI_SENSOR_SETTLE] = {"hs_settle", 0, 0, 0, parse_sensor_hsettle},
	[INI_SENSOR_ORIEN] = {"orien", 0, 0, 0, parse_sensor_orien},
	[INI_SENSOR_RSTPORT] = {"port", 0, 0, 0, parse_sensor_rstport},
	[INI_SENSOR_RSTPIN] = {"pin", 0, 0, 0, parse_sensor_rstpin},
	[INI_SENSOR_ACTIVE] = {"pol", 0, 0, 0, parse_sensor_rstpol},
	[INI_SENSOR_MUXDEV] = {"mux_dev", 1, 0, 0, parse_sensor_muxdev},
	[INI_SENSOR_ATTACHDEV] = {"attach_dev", 1, 0, 0, parse_sensor_attachdev},
	[INI_SENSOR_SWITCHPORT] = {"switch_port", 1, 0, 0, parse_sensor_switchport},
	[INI_SENSOR_SWITCHPIN] = {"switch_gpio", 1, 0, 0, parse_sensor_switchpin},
	[INI_SENSOR_SWITCHPOL] = {"switch_pol", 1, 0, 0, parse_sensor_switchpol},
};

static int parse_handler(void *user, const char *section, const char *name, const char *value)
{
	SNS_INI_CFG_S *cfg = (SNS_INI_CFG_S *)user;
	const INI_HDLR_S *hdler;
	int i, size, index = 0;

	if (strcmp(section, "source") == 0) {
		hdler = stSectionSource;
		size = INI_SOURCE_NUM;
	} else if (strcmp(section, "sensor") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 0;
	} else if (strcmp(section, "sensor2") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 1;
	} else if (strcmp(section, "sensor3") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 2;
	} else if (strcmp(section, "sensor4") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 3;
	} else if (strcmp(section, "sensor5") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 4;
	} else if (strcmp(section, "sensor6") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 5;
	} else if (strcmp(section, "sensor7") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 6;
	} else if (strcmp(section, "sensor8") == 0) {
		hdler = stSectionSensor;
		size = INI_SENSOR_NUM;
		index = 7;
	} else {
		/* unknown section/name */
		return CVI_SUCCESS;
	}

	if (hdler == stSectionSensor) {
		for (i = 0; i < size; i++) {
			stSectionSensor[i].param0 = index;
		}
	}

	for (i = 0; i < size; i++) {
		if (strcmp(name, hdler[i].name) == 0) {
			hdler[i].pfnJob(cfg, value, hdler[i].param0,
					hdler[i].param1, hdler[i].param2);
			break;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sns_setinipath(const CVI_CHAR *iniPath)
{
	int ret;

	if (iniPath == NULL) {
		SNS_DBG_PRT("%s: null ptr\n", __func__);
		ret = CVI_FAILURE;
	} else if (strlen(iniPath) >= SNSCFGPATH_SIZE) {
		SNS_DBG_PRT("%s: SNSCFGPATH_SIZE is too small\n", __func__);
		ret = CVI_FAILURE;
	} else {
		strncpy(g_snsCfgPath, iniPath, SNSCFGPATH_SIZE);
		ret = CVI_SUCCESS;
	}

	return ret;
}

CVI_S32 platform_sns_parseini(SENSOR_CFG_S *sensor_cfg)
{
	int ret = 0;
#define INI_FILE_PATH	"/mnt/data/sensor_cfg.ini"
#define INI_DEF_PATH	"/mnt/system/usr/bin/sensor_cfg.ini"

	if (sensor_cfg == NULL) {
		SNS_DBG_PRT("%s: null ptr\n", __func__);
		ret = CVI_FAILURE;
	}
	SNS_INI_CFG_S *pstIniCfg = &sensor_cfg->sns_ini_cfg;

	memcpy(pstIniCfg, &sns_ini_cfg, sizeof(*pstIniCfg));
	if (g_snsCfgPath[0] != 0) {
		SNS_DBG_PRT("Parse %s\n", g_snsCfgPath);
		ret = ini_parse(g_snsCfgPath, parse_handler, pstIniCfg);
		if (ret >= 0) {
			memcpy(&sns_ini_cfg, pstIniCfg, sizeof(*pstIniCfg));
			return CVI_SUCCESS;
		}
		if (ret != -1) {
			SNS_DBG_PRT("Parse %s incomplete, use default cfg\n", INI_FILE_PATH);
			return CVI_FAILURE;
		}

		SNS_DBG_PRT("%s Not Found\n", g_snsCfgPath);
	}
	SNS_DBG_PRT("Parse %s\n", INI_FILE_PATH);
	ret = ini_parse(INI_FILE_PATH, parse_handler, pstIniCfg);
	if (ret >= 0) {
		memcpy(&sns_ini_cfg, pstIniCfg, sizeof(*pstIniCfg));
		return CVI_SUCCESS;
	}
	if (ret != -1) {
		SNS_DBG_PRT("Parse %s incomplete, use default cfg\n", INI_FILE_PATH);
		return CVI_FAILURE;
	}
	SNS_DBG_PRT("%s Not Found\n", INI_FILE_PATH);
	SNS_DBG_PRT("Parse %s\n", INI_DEF_PATH);

	ret = ini_parse(INI_DEF_PATH, parse_handler, pstIniCfg);
	if (ret < 0) {
		if (ret == -1) {
			SNS_DBG_PRT("%s not exist, use default cfg\n", INI_DEF_PATH);
		} else {
			SNS_DBG_PRT("Parse %s incomplete, use default cfg\n", INI_DEF_PATH);
		}

		return CVI_FAILURE;
	}
	memcpy(&sns_ini_cfg, pstIniCfg, sizeof(*pstIniCfg));

	return CVI_SUCCESS;
}