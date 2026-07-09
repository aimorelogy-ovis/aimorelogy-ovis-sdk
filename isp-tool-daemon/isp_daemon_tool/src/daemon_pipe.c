
#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "pipe"

#include "daemon_module.h"
#include "daemon_pipe.h"
#include "daemon_utils.h"
#include "cvi_comm_vi.h"
#include "cvi_buffer.h"
#include "cvi_vb.h"
#include "cvi_isp.h"

extern int g_main_loop_run;
static struct module_pipe_t *pdaemon_pipe;

static int create_normal_boot_pipe(int pipe_id, struct module_pipe_t *daemon_pipe, daemon_pipe_cfg_t *p_cfg)
{
	struct module_t module;
	SENSOR_CFG_S *sensor_cfg = get_sensor_cfg(NULL);

	module_video_src_cfg_t *video_src_cfg =
		(module_video_src_cfg_t *)calloc(1, sizeof(module_video_src_cfg_t));

	video_src_cfg->enable_teaisp_bnr = p_cfg->video_pipe_cfg.enable_teaisp_bnr;
	video_src_cfg->enable_patgen = p_cfg->video_pipe_cfg.enable_patgen;
	video_src_cfg->enable_raw_replay = p_cfg->raw_replay_enable;
	video_src_cfg->enable_mirror = 0;
	video_src_cfg->enable_flip = 0;
	video_src_cfg->enable_isp_stitch = p_cfg->enable_stitch;
	video_src_cfg->stitch_attr = p_cfg->video_pipe_cfg.stitch_attr;
	if (p_cfg->enable_dump_boot_video) {
		video_src_cfg->bypass_src_frame_num = 0;
	} else {
		video_src_cfg->bypass_src_frame_num = -1; // do nothing
	}
	video_src_cfg->vi_vpss_mode = p_cfg->vi_vpss_mode;
	video_src_cfg->compress_mode = p_cfg->video_pipe_cfg.compress_mode;
	snprintf(video_src_cfg->cvi_bin_path, MAX_PATH_LEN, "%s", p_cfg->cvi_bin_path);
	snprintf(video_src_cfg->bnr_model_list, MAX_PATH_LEN, "%s",
		p_cfg->video_pipe_cfg.bnr_model_list);
	video_src_cfg->raw_replay_cfg = p_cfg->raw_replay_cfg;

	memset(&module, 0, sizeof(struct module_t));
	module.name = "video_src";
	module.pipe_id = pipe_id;
	module.module_cfg = video_src_cfg;
	module.fun = video_src_fun;
	module_pipe_add(daemon_pipe, &module);

	module_vpss_cfg_t *vpss_cfg =
		(module_vpss_cfg_t *)calloc(1, sizeof(module_vpss_cfg_t));

	if (p_cfg->vi_vpss_mode == VI_ONLINE_VPSS_ONLINE ||
	    p_cfg->vi_vpss_mode == VI_OFFLINE_VPSS_ONLINE) {
		vpss_cfg->grp_id = pipe_id;
		vpss_cfg->enable_vi_vpss_online = 1;
	} else {
		vpss_cfg->grp_id = -1; // auto assign
		vpss_cfg->enable_vi_vpss_online = 0;
	}

	vpss_cfg->chn_id = 0;
	vpss_cfg->chn_num = 1;
	vpss_cfg->enable_bypass_src_frame = 0;

	int img_width = sensor_cfg->sns_cfg.u32ImageWigth[pipe_id];
	int img_height = sensor_cfg->sns_cfg.u32ImageHeight[pipe_id];

	if (p_cfg->raw_replay_enable) {
		img_width = p_cfg->raw_replay_cfg.width;
		img_height = p_cfg->raw_replay_cfg.height;
	} else {
		img_width = sensor_cfg->sns_cfg.u32ImageWigth[pipe_id];
		img_height = sensor_cfg->sns_cfg.u32ImageHeight[pipe_id];
	}

	vpss_cfg->st_vpss_grp_attr.u32MaxW = img_width;
	vpss_cfg->st_vpss_grp_attr.u32MaxH = img_height;
	vpss_cfg->st_vpss_grp_attr.enPixelFormat = VI_PIXEL_FORMAT;
	vpss_cfg->st_vpss_grp_attr.stFrameRate.s32SrcFrameRate = -1;
	vpss_cfg->st_vpss_grp_attr.stFrameRate.s32DstFrameRate = -1;
	vpss_cfg->st_vpss_grp_attr.u8VpssDev = 1; // ?

	if (vpss_cfg->enable_vi_vpss_online) {
		vpss_cfg->chn_vb_cnt[0] = 0; // use VI buffer, common vb
	} else {
		vpss_cfg->chn_vb_cnt[0] = 2;
	}
	vpss_cfg->st_vpss_chn_attr[0].u32Width = img_width;
	vpss_cfg->st_vpss_chn_attr[0].u32Height = img_height;
	vpss_cfg->st_vpss_chn_attr[0].enVideoFormat = VIDEO_FORMAT_LINEAR;
	vpss_cfg->st_vpss_chn_attr[0].enPixelFormat = VI_PIXEL_FORMAT;
	vpss_cfg->st_vpss_chn_attr[0].u32Depth = 1;
	vpss_cfg->st_vpss_chn_attr[0].stFrameRate.s32SrcFrameRate = -1;
	vpss_cfg->st_vpss_chn_attr[0].stFrameRate.s32DstFrameRate = -1;
	vpss_cfg->st_vpss_chn_attr[0].bMirror = CVI_FALSE;
	vpss_cfg->st_vpss_chn_attr[0].bFlip = CVI_FALSE;
	vpss_cfg->st_vpss_chn_attr[0].stAspectRatio.enMode = ASPECT_RATIO_NONE;
	vpss_cfg->st_vpss_chn_attr[0].stNormalize.bEnable = CVI_FALSE;

	memset(&module, 0, sizeof(struct module_t));
	module.name = "vpss";
	module.pipe_id = pipe_id;
	module.module_cfg = vpss_cfg;
	module.fun = vpss_fun;
	module_pipe_add(daemon_pipe, &module);

	//memset(&module, 0, sizeof(struct module_t));
	//module.name = "empty_module";
	//module.pipe_id = pipe_id;
	//module.fun = empty_module_fun;
	//module_pipe_add(daemon_pipe, &module);
	//return 0;

#ifdef ENABLE_TEAISP_PQ
	if (p_cfg->video_pipe_cfg.enable_teaisp_pq) {
		module_teaisp_pq_cfg_t *teaisp_pq_cfg =
			(module_teaisp_pq_cfg_t *)calloc(1, sizeof(module_teaisp_pq_cfg_t));

		snprintf(teaisp_pq_cfg->model_path, MAX_PATH_LEN, "%s",
			p_cfg->teaisp_pq_model_path);

		memset(&module, 0, sizeof(struct module_t));
		module.name = "teaisp_pq";
		module.pipe_id = pipe_id;
		module.module_cfg = teaisp_pq_cfg;
		module.fun = teaisp_pq_fun;
		module_pipe_add(daemon_pipe, &module);
	}
#endif
#ifdef ENABLE_FACE_AE
	if (p_cfg->video_pipe_cfg.enable_face_ae) {
		module_face_ae_cfg_t *face_ae_cfg =
			(module_face_ae_cfg_t *)calloc(1, sizeof(module_face_ae_cfg_t));

		snprintf(face_ae_cfg->model_path, MAX_PATH_LEN, "%s",
			p_cfg->teaisp_faceae_model_path);
		memset(&module, 0, sizeof(struct module_t));
		module.name = "face_ae";
		module.pipe_id = pipe_id;
		module.module_cfg = face_ae_cfg;
		module.fun = face_ae_fun;
		module_pipe_add(daemon_pipe, &module);
	}
#endif

	if (p_cfg->video_pipe_cfg.enable_isp_info_osd) {
		memset(&module, 0, sizeof(struct module_t));
		module.name = "osd";
		module.pipe_id = pipe_id;
		module.fun = osd_fun;
		module_pipe_add(daemon_pipe, &module);
	}

	module_venc_cfg_t *venc_cfg =
		(module_venc_cfg_t *)calloc(1, sizeof(module_venc_cfg_t));

	venc_cfg->width = img_width;
	venc_cfg->height = img_height;
	venc_cfg->pixel_format = VI_PIXEL_FORMAT;
	snprintf(venc_cfg->codec, MAX_CODEC_LEN, "%s", p_cfg->video_pipe_cfg.codec);
	venc_cfg->vc_cfg = p_cfg->video_pipe_cfg.st_vc_cfg;
	venc_cfg->enable_dump_boot_video = p_cfg->enable_dump_boot_video;

	memset(&module, 0, sizeof(struct module_t));
	module.name = "venc";
	module.pipe_id = pipe_id;
	module.fun = venc_fun;
	module.module_cfg = venc_cfg;
	module_pipe_add(daemon_pipe, &module);

	module_sei_osd_cfg_t *sei_osd_cfg =
		(module_sei_osd_cfg_t *)calloc(1, sizeof(module_sei_osd_cfg_t));

	snprintf(sei_osd_cfg->codec, MAX_CODEC_LEN, "%s", p_cfg->video_pipe_cfg.codec);

	memset(&module, 0, sizeof(struct module_t));
	module.name = "sei_osd";
	module.pipe_id = pipe_id;
	module.fun = sei_osd_fun;
	module.module_cfg = sei_osd_cfg;
	module_pipe_add(daemon_pipe, &module);

	memset(&module, 0, sizeof(struct module_t));
	module.name = "rtsp";
	module.pipe_id = pipe_id;
	if (p_cfg->rtsp_server_select == 3) {
		module_rtsp3_cfg_t *rtsp3_cfg =
			(module_rtsp3_cfg_t *)calloc(1, sizeof(module_rtsp3_cfg_t));

		rtsp3_cfg->rtsp_port = p_cfg->rtsp_port;
		rtsp3_cfg->width = img_width;
		rtsp3_cfg->height = img_height;
		snprintf(rtsp3_cfg->codec, MAX_CODEC_LEN, "%s", p_cfg->video_pipe_cfg.codec);

		module.module_cfg = rtsp3_cfg;
		module.fun = cvi_rtsp3_fun;
	} else {
		module_rtsp_cfg_t *rtsp_cfg =
			(module_rtsp_cfg_t *)calloc(1, sizeof(module_rtsp_cfg_t));

		rtsp_cfg->rtsp_port = p_cfg->rtsp_port;
		rtsp_cfg->max_buf_size = p_cfg->rtsp_max_buf_size;
		rtsp_cfg->bitrate = p_cfg->video_pipe_cfg.st_vc_cfg.st_rc_attr.BitRate;
		snprintf(rtsp_cfg->codec, MAX_CODEC_LEN, "%s", p_cfg->video_pipe_cfg.codec);
		module.module_cfg = rtsp_cfg;
		module.fun = cvi_rtsp_fun;
	}

	module_pipe_add(daemon_pipe, &module);
	return 0;
}

static int create_fastboot_pipe(int pipe_id, struct module_pipe_t *daemon_pipe, daemon_pipe_cfg_t *p_cfg)
{
	struct module_t module;
	SENSOR_CFG_S *sensor_cfg = get_sensor_cfg(NULL);

	memset(&module, 0, sizeof(struct module_t));

	module_venc_fastboot_cfg_t *venc_fastboot_cfg =
		(module_venc_fastboot_cfg_t *)calloc(1, sizeof(module_venc_fastboot_cfg_t));

	venc_fastboot_cfg->dev_num = sensor_cfg->sns_ini_cfg.devNum;
	snprintf(venc_fastboot_cfg->codec, MAX_CODEC_LEN, "%s", p_cfg->video_pipe_cfg.codec);

	module.name = "venc_fastboot";
	module.pipe_id = pipe_id;
	module.module_cfg = venc_fastboot_cfg;
	module.fun = venc_fun_fastboot;
	module_pipe_add(daemon_pipe, &module);

	module_sei_osd_cfg_t *sei_osd_cfg =
		(module_sei_osd_cfg_t *)calloc(1, sizeof(module_sei_osd_cfg_t));

	snprintf(sei_osd_cfg->codec, MAX_CODEC_LEN, "%s", p_cfg->video_pipe_cfg.codec);

	memset(&module, 0, sizeof(struct module_t));
	module.name = "sei_osd";
	module.pipe_id = pipe_id;
	module.fun = sei_osd_fun;
	module.module_cfg = sei_osd_cfg;
	module_pipe_add(daemon_pipe, &module);

	module.name = "rtsp";
	module.pipe_id = pipe_id;
	if (p_cfg->rtsp_server_select == 3) {
		module_rtsp3_cfg_t *rtsp3_cfg =
			(module_rtsp3_cfg_t *)calloc(1, sizeof(module_rtsp3_cfg_t));

		rtsp3_cfg->rtsp_port = p_cfg->rtsp_port;
		rtsp3_cfg->width = sensor_cfg->sns_cfg.u32ImageWigth[pipe_id];
		rtsp3_cfg->height = sensor_cfg->sns_cfg.u32ImageHeight[pipe_id];
		snprintf(rtsp3_cfg->codec, MAX_CODEC_LEN, "%s", p_cfg->video_pipe_cfg.codec);

		module.module_cfg = rtsp3_cfg;
		module.fun = cvi_rtsp3_fun;
	} else {
		module_rtsp_cfg_t *rtsp_cfg =
			(module_rtsp_cfg_t *)calloc(1, sizeof(module_rtsp_cfg_t));

		rtsp_cfg->rtsp_port = p_cfg->rtsp_port;
		rtsp_cfg->max_buf_size = p_cfg->rtsp_max_buf_size;
		rtsp_cfg->bitrate = p_cfg->video_pipe_cfg.st_vc_cfg.st_rc_attr.BitRate;
		snprintf(rtsp_cfg->codec, MAX_CODEC_LEN, "%s", p_cfg->video_pipe_cfg.codec);
		module.module_cfg = rtsp_cfg;
		module.fun = cvi_rtsp_fun;
	}

	module_pipe_add(daemon_pipe, &module);
	return 0;
}

static int init_comm_vb_pool(daemon_pipe_cfg_t *p_cfg)
{
	int ret = 0;
	SENSOR_CFG_S *sensor_cfg = get_sensor_cfg(NULL);
	SNS_INI_CFG_S *stSnsIniCfg = &sensor_cfg->sns_ini_cfg;
	SNS_CFG_S *stSnsCfg = &sensor_cfg->sns_cfg;
	CVI_U32 u32BlkSize;
	CVI_U32 u32BlkRotSize;
	VB_CONFIG_S stVbConf;
	SIZE_S stSize;
	CVI_U32 Vb_cnt;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 0;

	int sns_num = (p_cfg->raw_replay_enable) ? 1 : stSnsIniCfg->devNum;

	for (int i = 0; i < sns_num; i++) {
		Vb_cnt = 0;
		bool createNewPool = true;
		PIXEL_FORMAT_E enPixelFormat = stSnsCfg->bBypassIsp[i] ?
			PIXEL_FORMAT_YUYV : VI_PIXEL_FORMAT;

		if (stSnsCfg->enFormatMode[i] == (SNS_DATA_TYPE_E) VI_DATA_TYPE_YUV) {
			if (stSnsCfg->enChnMode[i] ==
				(SNS_CHN_MODE_E) VI_WORK_MODE_2Multiplex) {
				Vb_cnt = 6;
			} else if (stSnsCfg->enChnMode[i] ==
				(SNS_CHN_MODE_E) VI_WORK_MODE_3Multiplex) {
				Vb_cnt = 9;
			} else if (stSnsCfg->enChnMode[i] ==
				(SNS_CHN_MODE_E) VI_WORK_MODE_4Multiplex) {
				Vb_cnt = 12;
			} else {
				Vb_cnt = 3;
			}
		} else {
			Vb_cnt = 3;
		}

		if (p_cfg->video_pipe_cfg.buf_blk_cnt > (int)Vb_cnt) {
			Vb_cnt = p_cfg->video_pipe_cfg.buf_blk_cnt;
		}

		if (p_cfg->raw_replay_enable) {
			stSize.u32Width = p_cfg->raw_replay_cfg.width;
			stSize.u32Height = p_cfg->raw_replay_cfg.height;
		} else {
			stSize.u32Width = stSnsCfg->u32ImageWigth[i];
			stSize.u32Height = stSnsCfg->u32ImageHeight[i];
		}

		u32BlkSize = COMMON_GetPicBufferSize(
			stSize.u32Width, stSize.u32Height, enPixelFormat,
			DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(
			stSize.u32Height, stSize.u32Width, enPixelFormat,
			DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;

		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (stVbConf.astCommPool[j].u32BlkSize == u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt += Vb_cnt;
				createNewPool = false;
				break;
			}
		}

		if (createNewPool) {
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize =
				u32BlkSize;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt =
				Vb_cnt;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt]
				.enRemapMode = VB_REMAP_MODE_CACHED;
			stVbConf.u32MaxPoolCnt++;
			clog_i("[INFO]  createNewPool VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						stVbConf.u32MaxPoolCnt, stSize.u32Width, stSize.u32Height,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkCnt,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkSize);
		} else {
			clog_i("[INFO] set VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						stVbConf.u32MaxPoolCnt, stSize.u32Width, stSize.u32Height,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkCnt,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkSize);
		}
	}

	ret = CVI_VB_SetConfig(&stVbConf);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_VB_SetConf failed!\n");
		return ret;
	}

	ret = CVI_VB_Init();
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_VB_Init failed!\n");
		return ret;
	}

	return ret;
}

static int deinit_comm_vb_pool(void)
{
	return CVI_VB_Exit();
}

int create_pipe(daemon_pipe_cfg_t *p_cfg)
{
	int ret = 0;
	SENSOR_CFG_S *sensor_cfg = NULL;
	int dev_num = 0;
	struct module_t module;

	memset(&module, 0, sizeof(struct module_t));

	// parse sensor cfg ini
	sensor_cfg = get_sensor_cfg(p_cfg->sns_cfg_ini);
	if (sensor_cfg == NULL) {
		clog_e("get sensor cfg fail!\n");
		return -1;
	}

	if (p_cfg->dev_num > sensor_cfg->sns_ini_cfg.devNum) {
		p_cfg->dev_num = sensor_cfg->sns_ini_cfg.devNum;
	}

	dev_num = p_cfg->dev_num;

	if (p_cfg->raw_replay_enable) {
		dev_num = 1;
		p_cfg->dev_num = 1;
	}

	if (p_cfg->is_fastboot_mode == 0) {
		ret = init_comm_vb_pool(p_cfg);
		if (ret != 0) {
			clog_e("init vb pool fail!\n");
			return ret;
		}
	}

	pdaemon_pipe = (struct module_pipe_t *)calloc(dev_num, sizeof(struct module_pipe_t));
	for (int i = 0; i < dev_num; i++) {
		clog_i("create pipe: %d, %d\n", i, p_cfg[i].video_pipe_cfg.chn);
		pdaemon_pipe[i].pipe_cfg = &p_cfg[i];

		if (p_cfg->is_fastboot_mode == 0) {
			create_normal_boot_pipe(i, &pdaemon_pipe[i], &p_cfg[i]);
		} else {
			create_fastboot_pipe(i, &pdaemon_pipe[i], &p_cfg[i]);
		}

		ret = module_pipe_init(&pdaemon_pipe[i]);

		if (ret != 0) {
			clog_e("craete pipe: %d fail!\n", i);
			return ret;
		}
		//sleep(1); // wait for frame ready
		//module_pipe_start(&pdaemon_pipe[i]);
	}

#define __WAIT_FRAME_MAX_CONT (30) // the bypass frame num by isp
	int wait_frame_cont = 0;

	do {
		if (g_main_loop_run == 0 ||
			p_cfg[0].enable_dump_boot_video) {
			break;
		}
		ret = CVI_ISP_GetVDTimeOut(0, ISP_VD_BE_END,
					   DAEMON_TIMEOUT_MS * 2);
		if (ret == 0) {
			wait_frame_cont++;
		} else {
			clog_w("wait vi working timeout, go on wait...\n");
		}
	} while (wait_frame_cont < __WAIT_FRAME_MAX_CONT);

	if (wait_frame_cont >= __WAIT_FRAME_MAX_CONT) {
		clog_i("wait vi working done!\n"); // for auto test, don't modify
	}

	for (int i = 0; i < dev_num; i++) {
		module_pipe_start(&pdaemon_pipe[i]);
	}

	return 0;
}

int destroy_pipe(daemon_pipe_cfg_t *p_cfg)
{
	for (int i = 0; i < p_cfg->dev_num; i++) {
		clog_i("destroy pipe: %d\n", i);
		module_pipe_stop(&pdaemon_pipe[i]);
		module_pipe_deinit(&pdaemon_pipe[i]);
		module_pipe_destroy(&pdaemon_pipe[i]);
	}

	free(pdaemon_pipe);
	pdaemon_pipe = NULL;

	if (p_cfg->is_fastboot_mode == 0) {
		deinit_comm_vb_pool();
	}

	return 0;
}
