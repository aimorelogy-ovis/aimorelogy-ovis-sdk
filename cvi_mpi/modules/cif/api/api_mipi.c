#include "platform_mipi.h"

CVI_S32 CVI_MIPI_SetMipiReset(CVI_S32 devno, CVI_U32 reset)
{
	return platform_mipi_SetMipiReset(devno, reset);
}

CVI_S32 CVI_MIPI_SetSensorClock(CVI_S32 devno, CVI_U32 enable)
{
	return platform_mipi_SetSensorClock(devno, enable);
}

CVI_S32 CVI_MIPI_SetSensorReset(CVI_S32 devno, CVI_U32 reset_port,
				CVI_U32 reset_pin, CVI_U32 reset_pol, CVI_U32 reset_enable)
{
	return platform_mipi_SetSensorReset(devno, reset_port, reset_pin, reset_pol, reset_enable);
}

CVI_S32 CVI_MIPI_SetMipiAttr(CVI_S32 ViPipe, const CVI_VOID *devAttr)
{
	return platform_mipi_SetMipiAttr(ViPipe, devAttr);
}

CVI_S32 CVI_MIPI_SetClkEdge(CVI_S32 devno, CVI_U32 is_up)
{
	return platform_mipi_SetClkEdge(devno, is_up);
}

CVI_S32 CVI_MIPI_SetSnsMclk(struct mclk_pll_s *mclk)
{
	return platform_mipi_SetSnsMclk(mclk);
}