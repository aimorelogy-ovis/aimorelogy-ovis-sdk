
#include <sys/prctl.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "osd"

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_region.h"
#include "osd_font_mod.h"

#define CVI_MEDIA_MAX_INFO_OSD_LEN (80) // minimum chn width (1920) / font width (24)
#define OSD_INFO_START_X (0)
#define OSD_INFO_START_Y (0)
#define OSD_INFO_FONT_WIDTH (24)
#define OSD_INFO_FONT_HEIGHT (30)
#define OSD_NUM (2)

#define IsASCII(a) (((a) >= 0x00 && (a) <= 0x7F) ? 1 : 0)
#define BYTE_BITS (8)
/* Number of bytes occupied by each Chinese character */
#define NOASCII_CHARACTER_BYTES (2)
#define OSD_LIB_FONT_W (OSD_INFO_FONT_WIDTH)
#define OSD_LIB_FONT_H (OSD_INFO_FONT_HEIGHT)

typedef struct {
	uint32_t font_width;
	uint32_t font_height;
	MMF_CHN_S chn;
	uint32_t rgn_hdl[OSD_NUM];
} osd_ctx_t;

static uint32_t g_rgn_hdl;

static int init(struct module_t *thiz)
{
	int ret = 0;

	clog_i("pipe_id: %d\n", thiz->pipe_id);

	osd_ctx_t *ctx = (osd_ctx_t *)calloc(1, sizeof(osd_ctx_t));

	if (!ctx) {
		clog_e("calloc failed\n");
		return -1;
	}

	thiz->module_ctx = ctx;

	ctx->font_width = OSD_INFO_FONT_WIDTH;
	ctx->font_height = OSD_INFO_FONT_HEIGHT;

	ctx->chn.enModId = CVI_ID_VPSS;
	ctx->chn.s32DevId = thiz->pipe_id;
	ctx->chn.s32ChnId = 0;

	RGN_ATTR_S rgn_attr;
	RGN_CHN_ATTR_S rgn_chn_attr;

	memset(&rgn_attr, 0, sizeof(RGN_ATTR_S));
	memset(&rgn_chn_attr, 0, sizeof(RGN_CHN_ATTR_S));

	rgn_attr.enType = OVERLAY_RGN;
	rgn_attr.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	rgn_attr.unAttr.stOverlay.u32BgColor = 0x8000;
	rgn_attr.unAttr.stOverlay.stSize.u32Width =
		CVI_MEDIA_MAX_INFO_OSD_LEN * OSD_INFO_FONT_WIDTH;
	rgn_attr.unAttr.stOverlay.stSize.u32Height = OSD_INFO_FONT_HEIGHT;
	rgn_attr.unAttr.stOverlay.u32CanvasNum = 2;

	rgn_chn_attr.bShow = CVI_TRUE;
	rgn_chn_attr.enType = OVERLAY_RGN;
	rgn_chn_attr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	rgn_chn_attr.unChnAttr.stOverlayChn.stPoint.s32X = OSD_INFO_START_X;
	rgn_chn_attr.unChnAttr.stOverlayChn.stPoint.s32Y = OSD_INFO_START_Y;
	rgn_chn_attr.unChnAttr.stOverlayChn.u32Layer = 0;

	for (int i = 0; i < OSD_NUM; i++) {
		ctx->rgn_hdl[i] = g_rgn_hdl++;
		rgn_chn_attr.unChnAttr.stOverlayChn.stPoint.s32Y +=
			(i * OSD_INFO_FONT_HEIGHT);

		ret = CVI_RGN_Create(ctx->rgn_hdl[i], &rgn_attr);
		if (ret != CVI_SUCCESS) {
			clog_e("CVI_RGN_Create failed with %#x\n", ret);
			return ret;
		}

		ret = CVI_RGN_AttachToChn(ctx->rgn_hdl[i], &ctx->chn,
					  &rgn_chn_attr);
		if (ret != CVI_SUCCESS) {
			clog_e("CVI_RGN_AttachToChn failed with %#x\n", ret);
			return ret;
		}
	}

	return 0;
}

static int deinit(struct module_t *thiz)
{
	int ret = 0;

	osd_ctx_t *ctx = (osd_ctx_t *)thiz->module_ctx;

	for (int i = 0; i < OSD_NUM; i++) {
		ret = CVI_RGN_DetachFromChn(ctx->rgn_hdl[i], &ctx->chn);
		if (ret != CVI_SUCCESS) {
			clog_e("CVI_RGN_DetachFromChn failed with %#x\n", ret);
			return ret;
		}

		ret = CVI_RGN_Destroy(ctx->rgn_hdl[i]);
		if (ret != CVI_SUCCESS) {
			clog_e("CVI_RGN_Destroy failed with %#x\n", ret);
			return ret;
		}
	}

	if (thiz->module_cfg) {
		free(thiz->module_cfg);
		thiz->module_cfg = NULL;
	}

	if (thiz->module_ctx) {
		free(thiz->module_ctx);
		thiz->module_ctx = NULL;
	}
	return 0;
}

static int osd_get_non_asc_num(char *string, int len)
{
	int i;
	int n = 0;

	for (i = 0; i < len; i++) {
		if (string[i] == '\0') {
			break;
		}
		if (!IsASCII(string[i])) {
			i++;
			n++;
		}
	}

	return n;
}

static int osd_get_font_mod(char *Character, uint8_t **FontMod, int *FontModLen)
{
	/* Get Font Mod in GB2312 Fontlib*/
	uint32_t offset = 0;
	uint32_t areacode = 0;
	uint32_t bitcode = 0;

	if (IsASCII(Character[0])) {
		areacode = 3;
		bitcode = (uint32_t)((uint8_t)Character[0] - 0x20);
	} else {
		areacode = (uint32_t)((uint8_t)Character[0] - 0xA0);
		bitcode = (uint32_t)((uint8_t)Character[1] - 0xA0);
	}
	offset = (94 * (areacode - 1) + (bitcode - 1)) *
		 (OSD_LIB_FONT_W * OSD_LIB_FONT_H / 8);
	*FontMod = (uint8_t *)g_fontLib + offset;
	*FontModLen = OSD_LIB_FONT_W * OSD_LIB_FONT_H / 8;
	return CVI_SUCCESS;
}

static int osd_update_text_bitmap(RGN_HANDLE RgnHdl, char *inStr)
{
	int s32Ret;
	uint32_t u32CanvasWidth, u32CanvasHeight, u32BgColor, u32Color;
	SIZE_S stFontSize;
	int s32StrLen = strnlen(inStr, CVI_MEDIA_MAX_INFO_OSD_LEN);
	int NonASCNum = osd_get_non_asc_num(inStr, s32StrLen);

	SIZE_S stMaxOsdSize = { 0 };

	stMaxOsdSize.u32Width =
		CVI_MEDIA_MAX_INFO_OSD_LEN * OSD_INFO_FONT_WIDTH;
	stMaxOsdSize.u32Height = OSD_INFO_FONT_HEIGHT;
	u32CanvasWidth =
		OSD_INFO_FONT_WIDTH *
		(s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
	u32CanvasHeight = OSD_INFO_FONT_HEIGHT;
	stFontSize.u32Width = OSD_INFO_FONT_WIDTH;
	stFontSize.u32Height = OSD_INFO_FONT_HEIGHT;
	u32BgColor = 0x8000;
	u32Color = 0xFFFF;

	RGN_CANVAS_INFO_S stCanvasInfo;

	s32Ret = CVI_RGN_GetCanvasInfo(RgnHdl, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_GetCanvasInfo fail,RgnHdl[%d] Error Code: [0x%08X]\n",
		       RgnHdl, s32Ret);
		return s32Ret;
	}

	/*   Generate Bitmap   */
#if (defined CHIP_ARCH_MARS) || (defined CHIP_ARCH_PHOBOS)
	uint16_t *puBmData = (uint16_t *)CVI_SYS_Mmap(
		stCanvasInfo.u64PhyAddr,
		stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);
#else
	uint16_t *puBmData = (uint16_t *)stCanvasInfo.pu8VirtAddr;
#endif

	uint32_t u32BmRow, u32BmCol; /*   Bitmap Row/Col Index   */

	for (u32BmRow = 0; u32BmRow < u32CanvasHeight; ++u32BmRow) {
		int NonASCShow = 0;

		for (u32BmCol = 0; u32BmCol < u32CanvasWidth; ++u32BmCol) {
			/*   Bitmap Data Offset for the point   */
			int s32BmDataIdx =
				u32BmRow * stCanvasInfo.u32Stride / 2 +
				u32BmCol;
			/*   Character Index in Text String   */
			int s32CharIdx = u32BmCol / stFontSize.u32Width;
			int s32StringIdx =
				s32CharIdx +
				NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
			if (NonASCNum > 0 && s32CharIdx > 0) {
				NonASCShow = osd_get_non_asc_num(inStr,
								 s32StringIdx);
				s32StringIdx =
					s32CharIdx +
					NonASCShow *
						(NOASCII_CHARACTER_BYTES - 1);
			}
			/*   Point Row/Col Index in Character   */
			int s32CharCol = (u32BmCol -
					  (stFontSize.u32Width * s32CharIdx)) *
					 OSD_LIB_FONT_W / stFontSize.u32Width;
			int s32CharRow = u32BmRow * OSD_LIB_FONT_H /
					 stFontSize.u32Height;
			int s32HexOffset =
				s32CharRow * OSD_LIB_FONT_W / BYTE_BITS +
				s32CharCol / BYTE_BITS;
			int s32BitOffset = s32CharCol % BYTE_BITS;
			uint8_t *FontMod = NULL;
			int FontModLen = 0;

			if (CVI_SUCCESS ==
			    osd_get_font_mod(&inStr[s32StringIdx], &FontMod,
					     &FontModLen)) {
				if (FontMod != NULL &&
				    s32HexOffset < FontModLen) {
					uint8_t temp = FontMod[s32HexOffset];

					if ((temp >>
					     ((BYTE_BITS - 1) - s32BitOffset)) &
					    0x1) {
						puBmData[s32BmDataIdx] =
							(uint16_t)u32Color;
					} else {
						puBmData[s32BmDataIdx] =
							(uint16_t)u32BgColor;
					}
					continue;
				}
			}
			printf("GetFontMod Fail\n");
#if (defined CHIP_ARCH_MARS) || (defined CHIP_ARCH_PHOBOS)
			CVI_SYS_Munmap((void *)puBmData,
				       stCanvasInfo.u32Stride *
					       stCanvasInfo.stSize.u32Height);
#endif
			return CVI_FAILURE;
		}

		for (u32BmCol = u32CanvasWidth;
		     u32BmCol < stMaxOsdSize.u32Width; ++u32BmCol) {
			int s32BmDataIdx =
				u32BmRow * stCanvasInfo.u32Stride / 2 +
				u32BmCol;
			puBmData[s32BmDataIdx] = (uint16_t)u32BgColor;
		}
	}

	for (u32BmRow = u32CanvasHeight; u32BmRow < stMaxOsdSize.u32Height;
	     ++u32BmRow) {
		for (u32BmCol = 0; u32BmCol < stMaxOsdSize.u32Width;
		     ++u32BmCol) {
			int s32BmDataIdx =
				u32BmRow * stCanvasInfo.u32Stride / 2 +
				u32BmCol;
			puBmData[s32BmDataIdx] = (uint16_t)u32BgColor;
		}
	}
	stCanvasInfo.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stCanvasInfo.stSize.u32Width = u32CanvasWidth;
	stCanvasInfo.stSize.u32Height = u32CanvasHeight;

	s32Ret = CVI_RGN_UpdateCanvas(RgnHdl);

	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_UpdateCanvas fail,RgnHdl[%d] Error Code: [0x%08X]\n",
		       RgnHdl, s32Ret);
#if (defined CHIP_ARCH_MARS) || (defined CHIP_ARCH_PHOBOS)
		CVI_SYS_Munmap((void *)puBmData,
			       stCanvasInfo.u32Stride *
				       stCanvasInfo.stSize.u32Height);
#endif
		return s32Ret;
	}
#if (defined CHIP_ARCH_MARS) || (defined CHIP_ARCH_PHOBOS)
	CVI_SYS_Munmap((void *)puBmData,
		       stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);
#endif
	return s32Ret;
}

static void osd_get_isp_info(VI_PIPE viPipe, char *str, uint32_t strLen,
			     int line)
{
	ISP_EXP_INFO_S expInfo;
	ISP_WB_INFO_S wbInfo;

	memset(&expInfo, 0, sizeof(ISP_EXP_INFO_S));
	memset(&wbInfo, 0, sizeof(ISP_WB_INFO_S));
	CVI_ISP_QueryExposureInfo(viPipe, &expInfo);
	CVI_ISP_QueryWBInfo(viPipe, &wbInfo);
	if (line == 0) {
		snprintf(
			str, strLen,
			"ExpT:%u(%u) ISO:%u(%u) Lv:%.1f AG:%u DG:%u IG:%u Lum:%d(%d)",
			expInfo.u32ExpTime, expInfo.u32ShortExpTime,
			expInfo.u32ISO, expInfo.u32ISOSF, expInfo.fLightValue,
			expInfo.u32AGain, expInfo.u32DGain, expInfo.u32ISPDGain,
			expInfo.u8AveLum, expInfo.s16HistError);
	} else if (line == 1) {
		struct tm *ptime;
		struct timeval tv;
		char str_time[24];

		gettimeofday(&tv, NULL);
		ptime = localtime(&tv.tv_sec);
		strftime(str_time, sizeof(str_time), "%m-%d %I:%M:%S",
			 ptime);

		snprintf(
			str, strLen,
			"%s:%03u Fps:%u ExpIsMax:%d #AWB RG:%d BG:%d CT:%d",
			str_time, (uint32_t)(tv.tv_usec / 1000),
			expInfo.u32Fps, expInfo.bExposureIsMAX,
			wbInfo.u16Rgain, wbInfo.u16Bgain, wbInfo.u16ColorTemp);
	}
}

static void *worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;
	osd_ctx_t *ctx = (osd_ctx_t *)thiz->module_ctx;
	char isp_info_str[CVI_MEDIA_MAX_INFO_OSD_LEN] = { 0 };

	clog_i("run, pipe_id: %d\n", thiz->pipe_id);
	prctl(PR_SET_NAME, "osd", 0, 0, 0);

	while (thiz->thread_run) {
		ret = CVI_ISP_GetVDTimeOut(thiz->pipe_id, ISP_VD_BE_END,
					   DAEMON_TIMEOUT_MS);
		if (ret != 0) {
			clog_w("get vd timeout, go on wait...\n");
			continue;
		}
		for (int i = 0; i < OSD_NUM; i++) {
			memset(isp_info_str, 0, CVI_MEDIA_MAX_INFO_OSD_LEN);
			osd_get_isp_info(thiz->pipe_id, isp_info_str,
					 CVI_MEDIA_MAX_INFO_OSD_LEN, i);
			osd_update_text_bitmap(ctx->rgn_hdl[i], isp_info_str);
		}
	}

	return NULL;
}

static int start(struct module_t *thiz)
{
	clog_i("pipe_id: %d\n", thiz->pipe_id);
	thiz->thread_run = 1;
	pthread_create(&thiz->thread_id, NULL, worker, thiz);
	return 0;
}

static int stop(struct module_t *thiz)
{
	clog_i("pipe_id: %d\n", thiz->pipe_id);
	thiz->thread_run = 0;
	pthread_join(thiz->thread_id, NULL);
	return 0;
}

static int get(struct module_t *thiz, void **data)
{
	int ret = 0;
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);

	ret = src_module->fun.get(src_module, data);
	if (ret != 0) {
		return ret;
	}
	return 0;
}

static int put(struct module_t *thiz, void *data)
{
	int ret = 0;
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);

	ret = src_module->fun.put(src_module, data);
	if (ret != 0) {
		return ret;
	}
	return 0;
}

struct module_fun_t osd_fun = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = get,
	.put = put,
};
