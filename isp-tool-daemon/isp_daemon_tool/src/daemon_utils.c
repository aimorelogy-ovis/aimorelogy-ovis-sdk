
#include "daemon_utils.h"
#include "cvi_sys.h"
#include "cvi_comm_vi.h"
#include "cvi_sensor.h"

int dump_video_frame(FILE *fp, void *pframe)
{
	VIDEO_FRAME_INFO_S *frame = (VIDEO_FRAME_INFO_S *)pframe;

	if (fp == NULL || frame == NULL) {
		return -1;
	}

	for (int i = 0; i < 3; i++) {
		if (frame->stVFrame.u32Length[i] == 0) {
			continue;
		}
		void *vaddr = CVI_SYS_MmapCache(frame->stVFrame.u64PhyAddr[i],
					   frame->stVFrame.u32Length[i]);
		if (vaddr == NULL) {
			clog_e("CVI_SYS_Mmap:0x%x failed\n",
			       frame->stVFrame.u64PhyAddr[i]);
			return -1;
		}
		CVI_SYS_IonInvalidateCache(frame->stVFrame.u64PhyAddr[i], vaddr,
					   frame->stVFrame.u32Length[i]);
		fwrite(vaddr, 1, frame->stVFrame.u32Length[i], fp);
		fflush(fp);
		CVI_SYS_Munmap(vaddr, frame->stVFrame.u32Length[i]);
	}

	return 0;
}

SENSOR_CFG_S *get_sensor_cfg(char *ini_path)
{
	static SENSOR_CFG_S sensor_cfg;
	static int is_initialized;
	CVI_S32 s32Ret;

	if (!is_initialized) {
		memset(&sensor_cfg, 0, sizeof(SENSOR_CFG_S));

		if (ini_path == NULL) {
			clog_e("First call must provide valid ini path\n");
			return NULL;
		}

		s32Ret = CVI_SNS_SetIniPath(ini_path);
		if (s32Ret == CVI_FAILURE) {
			clog_e("[ERROR] set ini path: %s fail\n", ini_path);
			return NULL;
		}

		s32Ret = CVI_SNS_ParseIni(&sensor_cfg);
		if (s32Ret == CVI_FAILURE) {
			clog_e("[ERROR] Parse fail\n");
			return NULL;
		}

		s32Ret = CVI_SNS_GetConfigInfo(&sensor_cfg);
		if (s32Ret == CVI_FAILURE) {
			clog_e("[ERROR] get sns cfg failed\n");
			return NULL;
		}

		s32Ret = CVI_SNS_SetSnsDrvCfg(&sensor_cfg);
		if (s32Ret == CVI_FAILURE) {
			clog_e("[ERROR] set sns_drv failed\n");
			return NULL;
		}

		// Handle multiple sensors sharing the same reset pin
		// Keep the first one, mark subsequent ones as -1 to skip reset
		SNS_INI_CFG_S *stSnsIniCfg = &sensor_cfg.sns_ini_cfg;

		for (int i = 0; i < stSnsIniCfg->devNum && i < VI_MAX_DEV_NUM; i++) {
			if (stSnsIniCfg->s32RstPort[i] == -1 || stSnsIniCfg->s32RstPin[i] == -1) {
				continue;
			}
			// Check if subsequent sensors use the same reset pin
			for (int j = i + 1; j < stSnsIniCfg->devNum && j < VI_MAX_DEV_NUM; j++) {
				if (stSnsIniCfg->s32RstPort[j] == stSnsIniCfg->s32RstPort[i] &&
					stSnsIniCfg->s32RstPin[j] == stSnsIniCfg->s32RstPin[i]) {
					clog_i("sensor_%d and sensor_%d use same reset pin (port=%d, pin=%d), "
						   "disable sensor_%d reset\n",
						   i, j, stSnsIniCfg->s32RstPort[i], stSnsIniCfg->s32RstPin[i], j);
					stSnsIniCfg->s32RstPort[j] = -1;
					stSnsIniCfg->s32RstPin[j] = -1;
				}
			}
		}

		is_initialized = 1;
	}

	return &sensor_cfg;
}
