#include "vi_ut_comm.h"

CVI_S32 vi_ut_smooth_rawdump(VI_UT_CTX *pUtCtx, CVI_BOOL isSave2File)
{
	CVI_S32 retVal = CVI_SUCCESS;
	VI_PIPE viPipe = pUtCtx->smoothInfo.u32Dev;
	CVI_U8 blkCnt = pUtCtx->smoothInfo.u32BlkCnt;
	CVI_U8 totalFrameCnt = pUtCtx->smoothInfo.u32TotalFrameCnt;
	VB_POOL poolId;
	VB_POOL_CONFIG_S cfg;
	VI_SMOOTH_RAW_DUMP_INFO_S stDumpInfo;
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DEV_ATTR_S stDevAttr;
	VI_PIPE_ATTR_S stPipeAttr;
	VI_CHN_ATTR_S stChnAttr;
	CVI_U64 phyAddr, *phyAddrList = CVI_NULL;
	VB_BLK vbBlk;
	CVI_S32 frmNum = 1;
	CVI_U32 devFrmWidth, devFrmHeight, frmWidth, frmHeight;
	CVI_U32 cropX = 0, cropY = 0, cropWidth = 0, cropHeight = 0;
	CVI_CHAR imgName[128] = {0, };
	struct timeval timeVal;

	CVI_VI_GetDevAttr((VI_DEV)viPipe, &stDevAttr);
	CVI_VI_GetChnAttr(0, (VI_CHN)viPipe, &stChnAttr);
	CVI_VI_GetPipeAttr(viPipe, &stPipeAttr);
	stPipeAttr.enCompressMode = pUtCtx->isDpcmOn ? COMPRESS_MODE_TILE : COMPRESS_MODE_NONE;
	CVI_VI_SetPipeAttr(viPipe, &stPipeAttr);

	devFrmWidth = stDevAttr.stSize.u32Width;
	devFrmHeight = stDevAttr.stSize.u32Height;

	frmWidth = devFrmWidth;
	frmHeight = devFrmHeight;

	frmNum = (stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ? 2 : 1;
	cfg.u32BlkCnt = frmNum * blkCnt;
	cfg.u32BlkSize = VI_GetRawBufferSize(frmWidth, frmHeight,
					PIXEL_FORMAT_RGB_BAYER_12BPP,
					stPipeAttr.enCompressMode,
					16,
					(stChnAttr.stSize.u32Width > 2304) ? CVI_TRUE : CVI_FALSE);

	UT_PRT("Create VB pool frm(%d) blkcnt(%d) cnt(%d) blksize(0x%x)\n",
			frmNum, blkCnt, cfg.u32BlkCnt, cfg.u32BlkSize);

	poolId = CVI_VB_CreatePool(&cfg);
	if (poolId == VB_INVALID_POOLID) {
		UT_PRT("create vb pool failed\n");
		retVal = CVI_FAILURE;
		return retVal;
	}

	phyAddrList = malloc(sizeof(*phyAddrList) * cfg.u32BlkCnt);
	if (phyAddrList == CVI_NULL) {
		UT_PRT("malloc phyAddrList failed\n");
		CVI_VB_DestroyPool(poolId);
		retVal = CVI_FAILURE;
		return retVal;
	}

	for (CVI_U32 i = 0; i < cfg.u32BlkCnt; i++) {
		vbBlk = CVI_VB_GetBlock(poolId, cfg.u32BlkSize);
		if (vbBlk == VB_INVALID_HANDLE) {
			UT_PRT("get VB blk failed\n");
			for (CVI_U32 j = 0; j < i; j++) {
				if (phyAddrList[j]) {
					vbBlk = CVI_VB_PhysAddr2Handle(phyAddrList[j]);
					if (vbBlk != VB_INVALID_HANDLE) {
						CVI_VB_ReleaseBlock(vbBlk);
					}
				}
			}
			free(phyAddrList);
			CVI_VB_DestroyPool(poolId);
			retVal = CVI_FAILURE;
			return retVal;
		}
		phyAddr = CVI_VB_Handle2PhysAddr(vbBlk);
		*(phyAddrList + i) = phyAddr;
		UT_PRT("i=%d, vbBlk=%#"PRIx64", addr(%#"PRIx64"), phyAddr(%#"PRIx64")\n",
					i, (intmax_t)vbBlk, phyAddr, *(phyAddrList + i));
	}

	memset(&stDumpInfo, 0, sizeof(stDumpInfo));
	stDumpInfo.ViPipe = viPipe;
	stDumpInfo.u8BlkCnt = blkCnt;
	stDumpInfo.phy_addr_list = phyAddrList;
	// set rawdump crop info in stDumpInfo
	stDumpInfo.stCropRect.s32X = cropX;
	stDumpInfo.stCropRect.s32Y = cropY;
	stDumpInfo.stCropRect.u32Width = cropWidth;
	stDumpInfo.stCropRect.u32Height = cropHeight;

	retVal = CVI_VI_StartSmoothRawDump(&stDumpInfo);
	if (retVal != CVI_SUCCESS) {
		UT_PRT("start failed\n");
		retVal = CVI_FAILURE;
		return retVal;
	}

	for (int i = 0; i < totalFrameCnt; i++) {
		memset(stVideoFrame, 0, sizeof(stVideoFrame));
		retVal = CVI_VI_GetSmoothRawDump(viPipe, stVideoFrame, 5000);
		if (retVal != CVI_SUCCESS) {
			UT_PRT("[%d] get frame failed\n", i);
			continue;
		}

		UT_PRT("[%d] get roi frame addr(%#"PRIx64") length(%d), number(%d)\n", i,
				stVideoFrame[0].stVFrame.u64PhyAddr[0],
				stVideoFrame[0].stVFrame.u32Length[0],
				stVideoFrame[0].stVFrame.u32TimeRef);

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0) {
			UT_PRT("[%d] get roi frame addr(%#"PRIx64") length(%d) number(%d)\n", i,
					stVideoFrame[1].stVFrame.u64PhyAddr[0],
					stVideoFrame[1].stVFrame.u32Length[0],
					stVideoFrame[1].stVFrame.u32TimeRef);
		}

		if (isSave2File) {
			for (int j = 0; j < frmNum; j++) {
				gettimeofday(&timeVal, NULL);
				snprintf(imgName, sizeof(imgName),
						"./vi_%d_%s_%d_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
						viPipe, (j == 0) ? "LE" : "SE",
						stVideoFrame[j].stVFrame.enBayerFormat,
						stVideoFrame[j].stVFrame.u32Width,
						stVideoFrame[j].stVFrame.u32Height,
						stVideoFrame[j].stVFrame.s16OffsetLeft,
						stVideoFrame[j].stVFrame.s16OffsetTop,
						(long int)timeVal.tv_sec, (long int)timeVal.tv_usec);

				retVal = FrameFullSaveToFile(imgName, &stVideoFrame[j]);
			}
		}

		retVal = CVI_VI_PutSmoothRawDump(viPipe, stVideoFrame);
		if (retVal != CVI_SUCCESS) {
			UT_PRT("[%d] release frame failed\n", i);
			continue;
		}
	}

	retVal = CVI_VI_StopSmoothRawDump(&stDumpInfo);
	if (retVal != CVI_SUCCESS) {
		UT_PRT("stop failed\n");
		retVal = CVI_FAILURE;
		return retVal;
	}

	for (CVI_U32 i = 0; i < cfg.u32BlkCnt; i++) {
		phyAddr = *(phyAddrList + i);
		vbBlk = CVI_VB_PhysAddr2Handle(phyAddr);
		if (vbBlk != VB_INVALID_HANDLE) {
			CVI_VB_ReleaseBlock(vbBlk);
		}
	}

	if (phyAddrList != CVI_NULL) {
		free(phyAddrList);
		phyAddrList = CVI_NULL;
	}

	retVal = CVI_VB_DestroyPool(poolId);

	return retVal;
}

CVI_S32 vi_ut_get_pipe_frame(VI_UT_CTX *pUtCtx, CVI_U8 pipe, CVI_BOOL isSave2File)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	VI_PIPE_ATTR_S pipe_attr;

	struct timeval tv1;
	int frm_num = 1, j = 0;

	memset(stVideoFrame, 0, sizeof(stVideoFrame));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	// Config pipe_attr.enCompressMode
	CVI_VI_GetPipeAttr(pipe, &pipe_attr);
	pipe_attr.enCompressMode = pUtCtx->isDpcmOn ? COMPRESS_MODE_TILE : COMPRESS_MODE_NONE;
	CVI_VI_SetPipeAttr(pipe, &pipe_attr);

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;

	CVI_VI_SetPipeDumpAttr(pipe, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;

	CVI_VI_GetPipeDumpAttr(pipe, &attr);

	UT_PRT("Enable(%d), DumpType(%d):\n", attr.bEnable, attr.enDumpType);

	frm_num = 1;

	s32Ret = CVI_VI_GetPipeFrame(pipe, stVideoFrame, 6000);
	if (s32Ret == CVI_SUCCESS) {
		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
			frm_num = 2;

		gettimeofday(&tv1, NULL);

		for (j = 0; j < frm_num; j++) {
			size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
			unsigned char *ptr = calloc(1, image_size);
			FILE *output;
			char img_name[128] = {0,}, order_id[8] = {0,};

			if (attr.enDumpType == VI_DUMP_TYPE_RAW && isSave2File) {
				stVideoFrame[j].stVFrame.pu8VirAddr[0]
					= CVI_SYS_Mmap(stVideoFrame[j].stVFrame.u64PhyAddr[0]
						, stVideoFrame[j].stVFrame.u32Length[0]);
				UT_PRT("paddr(%#"PRIx64") vaddr(%p)\n",
							stVideoFrame[j].stVFrame.u64PhyAddr[0],
							stVideoFrame[j].stVFrame.pu8VirAddr[0]);

				memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
					stVideoFrame[j].stVFrame.u32Length[0]);
				CVI_SYS_Munmap((void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
						stVideoFrame[j].stVFrame.u32Length[0]);

				switch (stVideoFrame[j].stVFrame.enBayerFormat) {
				default:
				case BAYER_FORMAT_BG:
					snprintf(order_id, sizeof(order_id), "BG");
					break;
				case BAYER_FORMAT_GB:
					snprintf(order_id, sizeof(order_id), "GB");
					break;
				case BAYER_FORMAT_GR:
					snprintf(order_id, sizeof(order_id), "GR");
					break;
				case BAYER_FORMAT_RG:
					snprintf(order_id, sizeof(order_id), "RG");
					break;
				}

				snprintf(img_name, sizeof(img_name),
						"./vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
						pipe, (j == 0) ? "LE" : "SE", order_id,
						stVideoFrame[j].stVFrame.u32Width,
						stVideoFrame[j].stVFrame.u32Height,
						stVideoFrame[j].stVFrame.s16OffsetLeft,
						stVideoFrame[j].stVFrame.s16OffsetTop,
						(long int)tv1.tv_sec, (long int)tv1.tv_usec);

				UT_PRT("dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);
			}

			UT_PRT("Get frame OK paddr(0x%#"PRIx64")\n", stVideoFrame[j].stVFrame.u64PhyAddr[0]);
		}

		CVI_VI_ReleasePipeFrame(pipe, stVideoFrame);
	}

	return s32Ret;
}

CVI_S32 vi_ut_get_chn_frame(CVI_U8 pipe, CVI_U8 chn, CVI_BOOL isSave2File)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));

	s32Ret = CVI_VI_GetChnFrame(pipe, chn, &stVideoFrame, 6000);
	if (s32Ret == CVI_SUCCESS) {
		CVI_CHAR img_name[128] = {0, };
		struct timeval tv;

		gettimeofday(&tv, NULL);
		snprintf(img_name, sizeof(img_name), "ut_pipe%d_chn%d_%u_%u_%ld_%ld.yuv",
				pipe, chn, stVideoFrame.stVFrame.u32Width, stVideoFrame.stVFrame.u32Height,
				(long int)tv.tv_sec, (long int)tv.tv_usec);

		if (isSave2File)
			FrameFullSaveToFile(img_name, &stVideoFrame);

		if (CVI_VI_ReleaseChnFrame(pipe, chn, &stVideoFrame) != 0)
			UT_PRT("CVI_VI_ReleaseChnFrame NG\n");

		UT_PRT("Get frame OK paddr(0x%#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]);

		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vi_ut_get_vpss_chn_frame(CVI_U8 chn)
{
	VIDEO_FRAME_INFO_S stVideoFrame = {0};

	if (CVI_VPSS_GetChnFrame(0, chn, &stVideoFrame, 6000) == 0) {
		FILE *output;
		size_t image_size = stVideoFrame.stVFrame.u32Length[0]
				  + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		CVI_CHAR img_name[128] = {0, };
		struct timeval tv;

		gettimeofday(&tv, NULL);
		snprintf(img_name, sizeof(img_name), "ut_chn%d_%u_%u_%ld_%ld.yuv",
				chn, stVideoFrame.stVFrame.u32Width, stVideoFrame.stVFrame.u32Height,
				(long int)tv.tv_sec, (long int)tv.tv_usec);

		UT_PRT("name: %s\n", img_name);
		UT_PRT("width: %d, height: %d, total_buf_length: %zu\n",
			   stVideoFrame.stVFrame.u32Width,
			   stVideoFrame.stVFrame.u32Height, image_size);

		output = fopen(img_name, "wb");
		if (output == NULL) {
			CVI_VPSS_ReleaseChnFrame(0, chn, &stVideoFrame);
			UT_PRT("fopen fail\n");
			return CVI_FAILURE;
		}

		u32LumaSize = stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		u32ChromaSize = stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
		plane_offset = 0;
		for (int i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				UT_PRT("plane(%d): paddr(0x%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				fwrite((void *)stVideoFrame.stVFrame.pu8VirAddr[i],
					(i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
			}
		}
		CVI_SYS_Munmap(vir_addr, image_size);

		if (CVI_VPSS_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			UT_PRT("CVI_VPSS_ReleaseChnFrame NG\n");

		fclose(output);
		return CVI_SUCCESS;
	}
	UT_PRT("CVI_VPSS_GetChnFrame NG\n");

	return CVI_FAILURE;
}

CVI_S32 vi_ut_auto_test_dump(VI_UT_CTX *pUtCtx, CVI_BOOL isDumpRaw, CVI_BOOL isDumpYuv, CVI_BOOL isSave2File)
{
	CVI_S32 s32Ret;
	CVI_S32 i = 0, j = 0;
	VI_PIPE viPipe;
	VI_CONFIG_S *pViConfig = &pUtCtx->viConfig;
	VI_INFO_S *pstViInfo = NULL;

	if (!pUtCtx->isAutoTest) {
		return 0;
	}

	for (i = 0; i < pViConfig->s32ViNum; i++) {
		pstViInfo = &pViConfig->astViInfo[i];
		for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
			if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
				viPipe = pstViInfo->stPipeInfo.aPipe[j];
				if (isDumpRaw) {
					s32Ret = vi_ut_get_pipe_frame(pUtCtx, viPipe, isSave2File);
					if (s32Ret != CVI_SUCCESS) {
						UT_PRT("vi_ut_get_pipe_frame failed. s32Ret: 0x%x !\n", s32Ret);
						return s32Ret;
					}

				}

				if (isDumpYuv) {
					s32Ret = vi_ut_get_chn_frame(viPipe, 0, isSave2File);
					if (s32Ret != CVI_SUCCESS) {
						UT_PRT("vi_ut_get_chn_frame failed. s32Ret: 0x%x !\n", s32Ret);
						return s32Ret;
					}
				}
			}
		}
	}

	UT_PRT("vi_ut_get_pipe_frame and vi_ut_get_chn_frame success\n");

	return s32Ret;
}