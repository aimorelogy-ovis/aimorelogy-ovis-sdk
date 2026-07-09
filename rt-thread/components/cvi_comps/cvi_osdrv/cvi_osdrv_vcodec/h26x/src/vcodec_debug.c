#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
// #include "aos/cli.h"
#include "vdi.h"
#include "vdi_osal.h"
#include "vpuapifunc.h"
#include "cvitest_internal.h"


#ifndef UNUSED
#define UNUSED(x) //((void)(x))
#endif

#define cvi_printf printf

//static int bVcodecCliReg;
//static pthread_mutex_t vcodecCliMutex = PTHREAD_MUTEX_INITIALIZER;

int showVpuReg(int id)
{
	int coreIdx =  0;
	int idx = 0;

	// if (argc != 2) {
	// 	cvi_printf("usage:%s num\n", argv[0]);
	// 	return -1;
	// }

	coreIdx = id;
	cvi_printf("show coreid = %d regs from hw:\n", coreIdx);
	cvi_printf("0x%04X  %X\n", idx, VpuReadReg(coreIdx, idx + 0));
	cvi_printf("0x%04X  %X\n", idx + 0x4, VpuReadReg(coreIdx, idx + 0x4));
	cvi_printf("0x%04X  %X\n", idx + 0x8, VpuReadReg(coreIdx, idx + 0x8));
	cvi_printf("0x%04X  %X\n", idx + 0xC, VpuReadReg(coreIdx, idx + 0xC));
	cvi_printf("0x%04X  %X\n", idx + 0x010, VpuReadReg(coreIdx, idx  + 0x010));
	cvi_printf("0x%04X  %X\n", idx + 0x014, VpuReadReg(coreIdx, idx  + 0x014));
	cvi_printf("0x%04X  %X\n", idx + 0x018, VpuReadReg(coreIdx, idx  + 0x018));
	cvi_printf("0x%04X  %X\n", idx + 0x024, VpuReadReg(coreIdx, idx  + 0x024));
	cvi_printf("0x%04X  %X\n", idx + 0x034, VpuReadReg(coreIdx, idx  + 0x034));


	for (idx = 0x100; idx <= 0x200; idx += 0x10) {
		cvi_printf("0x%04X  %011X  %011X  %011X\t  %011X\n", idx,
			   VpuReadReg(coreIdx, idx), VpuReadReg(coreIdx, idx + 0x4),
			   VpuReadReg(coreIdx, idx + 0x8),
			   VpuReadReg(coreIdx, idx + 0xC));
	}


	return 0;
}


int vcodecModMaskDebug(int32_t argc, char *argv[])
{
	int dbg_mask_tmp  = 0;

	if (argc != 2) {
		cvi_printf("usage:%s mask_value\n", argv[0]);
		cvi_printf("current dbg_mask=%#x\n", dbg_mask);
		return -1;
	}

	dbg_mask_tmp = (int)atoi(argv[1]);
	dbg_mask = dbg_mask_tmp;
	cvi_printf("set suc,current dbg_mask=%#x\n", dbg_mask);

	return 0;
}


void showCodecInstPoolInfo(CodecInst *pCodecInst)
{
	if (pCodecInst && pCodecInst->inUse) {
		cvi_printf("================start=================\n");
		cvi_printf("inUse:%d\n", pCodecInst->inUse);
		cvi_printf("instIndex:%d\n", pCodecInst->instIndex);
		cvi_printf("coreIdx:%d\n", pCodecInst->coreIdx);
		cvi_printf("codecMode:%d\n", pCodecInst->codecMode);
		cvi_printf("codecModeAux:%d\n", pCodecInst->codecModeAux);
		cvi_printf("productId:%d\n", pCodecInst->productId);
#ifdef ENABLE_CNM_DEBUG_MSG
		cvi_printf("loggingEnable:%d\n", pCodecInst->loggingEnable);
#endif
		cvi_printf("isDecoder:%s\n", pCodecInst->isDecoder ? "decode" : "encode");
		cvi_printf("state:%d\n", pCodecInst->state);

		if (!pCodecInst->isDecoder) { //enc
			EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;

			cvi_printf("encInfo:\n");
			cvi_printf(" bitRate:%d\n", pEncInfo->openParam.bitRate);
			cvi_printf(" bitstreamBufferSize:%d\n", pEncInfo->openParam.bitstreamBufferSize);
			cvi_printf(" picWidth:%d\n", pEncInfo->openParam.picWidth);
			cvi_printf(" picHeight:%d\n", pEncInfo->openParam.picHeight);
			cvi_printf(" frameSkipDisable:%d\n", pEncInfo->openParam.frameSkipDisable);
			cvi_printf(" ringBufferEnable:%d\n", pEncInfo->openParam.ringBufferEnable);
			cvi_printf(" frameRateInfo:%d\n", pEncInfo->openParam.frameRateInfo);
			cvi_printf(" gopSize:%d\n", pEncInfo->openParam.gopSize);
			cvi_printf(" initialDelay:%d\n", pEncInfo->openParam.initialDelay);
			cvi_printf(" changePos:%d\n", pEncInfo->openParam.changePos);

			cvi_printf(" minFrameBufferCount:%d\n", pEncInfo->initialInfo.minFrameBufferCount);
			cvi_printf(" minSrcFrameCount:%d\n", pEncInfo->initialInfo.minSrcFrameCount);

			cvi_printf(" streamBufSize:%d\n", pEncInfo->streamBufSize);
			cvi_printf(" linear2TiledEnable:%d\n", pEncInfo->linear2TiledEnable);
			cvi_printf(" linear2TiledMode:%d\n", pEncInfo->linear2TiledMode);
			cvi_printf(" mapType:%d\n", pEncInfo->mapType);
			cvi_printf(" stride:%d\n", pEncInfo->frameBufPool[0].stride);
			cvi_printf(" myIndex:%d\n", pEncInfo->frameBufPool[0].myIndex);
			cvi_printf(" width:%d\n", pEncInfo->frameBufPool[0].width);
			cvi_printf(" height:%d\n", pEncInfo->frameBufPool[0].height);
			cvi_printf(" size:%d\n", pEncInfo->frameBufPool[0].size);
			cvi_printf(" format:%d\n", pEncInfo->frameBufPool[0].format);
			cvi_printf(" vbFrame_size:%d\n", pEncInfo->vbFrame.size);
			cvi_printf(" vbPPU_size:%d\n", pEncInfo->vbPPU.size);
			cvi_printf(" streamBufSize:%d\n", pEncInfo->frameAllocExt);
			cvi_printf(" ppuAllocExt:%d\n", pEncInfo->ppuAllocExt);
			cvi_printf(" numFrameBuffers:%d\n", pEncInfo->numFrameBuffers);
			cvi_printf(" ActivePPSIdx:%d\n", pEncInfo->ActivePPSIdx);
			cvi_printf(" frameIdx:%d\n", pEncInfo->frameIdx);

			if (pCodecInst->coreIdx) {
				int ActivePPSIdx = pEncInfo->ActivePPSIdx;
				EncAvcParam *pAvcParam = &pEncInfo->openParam.EncStdParam.avcParam;
				AvcPpsParam *ActivePPS = &pAvcParam->ppsParam[ActivePPSIdx];

				cvi_printf(" entropyCodingMode:%d\n", ActivePPS->entropyCodingMode);
				cvi_printf(" transform8x8Mode:%d\n", ActivePPS->transform8x8Mode);
				cvi_printf(" profile:%d\n", pAvcParam->profile);
				cvi_printf(" fieldFlag:%d\n", pAvcParam->fieldFlag);
				cvi_printf(" chromaFormat400:%d\n", pAvcParam->chromaFormat400);
			}

			cvi_printf(" numFrameBuffers:%d\n", pEncInfo->numFrameBuffers);
			cvi_printf(" stride:%d\n", pEncInfo->stride);
			cvi_printf(" frameBufferHeight:%d\n", pEncInfo->frameBufferHeight);
			cvi_printf(" rotationEnable:%d\n", pEncInfo->rotationEnable);
			cvi_printf(" mirrorEnable:%d\n", pEncInfo->mirrorEnable);
			cvi_printf(" mirrorDirection:%d\n", pEncInfo->mirrorDirection);
			cvi_printf(" rotationAngle:%d\n", pEncInfo->rotationAngle);
			cvi_printf(" initialInfoObtained:%d\n", pEncInfo->initialInfoObtained);
			cvi_printf(" ringBufferEnable:%d\n", pEncInfo->ringBufferEnable);
			cvi_printf(" mirrorEnable:%d\n", pEncInfo->mirrorEnable);
			cvi_printf(" encoded_frames_in_gop:%d\n", pEncInfo->encoded_frames_in_gop);

			cvi_printf(" lineBufIntEn:%d\n", pEncInfo->lineBufIntEn);
			cvi_printf(" bEsBufQueueEn:%d\n", pEncInfo->bEsBufQueueEn);
			cvi_printf(" vbWork_size:%d\n", pEncInfo->vbWork.size);
			cvi_printf(" vbScratch_size:%d\n", pEncInfo->vbScratch.size);
			cvi_printf(" vbTemp_size:%d\n", pEncInfo->vbTemp.size);
			cvi_printf(" vbMV_size:%d\n", pEncInfo->vbMV.size);
			cvi_printf(" vbFbcYTbl_size:%d\n", pEncInfo->vbFbcYTbl.size);
			cvi_printf(" vbFbcCTbl_size:%d\n", pEncInfo->vbFbcCTbl.size);
			cvi_printf(" vbSubSamBuf_size:%d\n", pEncInfo->vbSubSamBuf.size);
			cvi_printf(" errorReasonCode:%d\n", pEncInfo->errorReasonCode);
			cvi_printf(" curPTS:%lu\n", pEncInfo->curPTS);
			cvi_printf(" instanceQueueCount:%d\n", pEncInfo->instanceQueueCount);
#ifdef SUPPORT_980_ROI_RC_LIB
			cvi_printf(" gop_size:%d\n", pEncInfo->gop_size);
			cvi_printf(" max_latency_pictures:%d\n", pEncInfo->max_latency_pictures);
#endif
			cvi_printf(" force_as_long_term_ref:%d\n", pEncInfo->force_as_long_term_ref);
			cvi_printf(" pic_ctu_avg_qp:%d\n", pEncInfo->pic_ctu_avg_qp);
		} else {  //dec
			DecInfo *pDecInfo = &pCodecInst->CodecInfo->decInfo;

			cvi_printf("decInfo:\n");
			cvi_printf(" bitstreamBufferSize:%d\n",
				pDecInfo->openParam.bitstreamBufferSize);
			cvi_printf(" streamBufSize:%d\n", pDecInfo->streamBufSize);
			cvi_printf(" numFbsForDecoding:%d\n", pDecInfo->numFbsForDecoding);
			cvi_printf(" wtlEnable:%d\n", pDecInfo->wtlEnable);
			cvi_printf(" picWidth:%d\n", pDecInfo->initialInfo.picWidth);
			cvi_printf(" picHeight:%d\n", pDecInfo->initialInfo.picHeight);
			cvi_printf(" wtlEnable:%d\n", pDecInfo->wtlEnable);
			cvi_printf(" frameBufPool_size:%d\n", pDecInfo->frameBufPool[0].size);
			cvi_printf(" frameBufPool_width:%d\n",
				pDecInfo->frameBufPool[0].width);
			cvi_printf(" frameBufPool_height:%d\n",
				pDecInfo->frameBufPool[0].height);
			cvi_printf(" frameBufPool_format:%d\n",
				pDecInfo->frameBufPool[0].format);
		}

		cvi_printf("rcInfo begin:\n");
		cvi_printf(" rcEnable:%d\n", pCodecInst->rcInfo.rcEnable);
		cvi_printf(" targetBitrate:%d\n", pCodecInst->rcInfo.targetBitrate);
		cvi_printf(" picAvgBit:%d\n", pCodecInst->rcInfo.picAvgBit);
		cvi_printf(" maximumBitrate:%d\n", pCodecInst->rcInfo.maximumBitrate);
		cvi_printf(" bitrateBuffer:%d\n", pCodecInst->rcInfo.bitrateBuffer);
		cvi_printf(" frameSkipBufThr:%d\n", pCodecInst->rcInfo.frameSkipBufThr);
		cvi_printf(" frameSkipCnt:%d\n", pCodecInst->rcInfo.frameSkipCnt);
		cvi_printf(" contiSkipNum:%d\n", pCodecInst->rcInfo.contiSkipNum);
		cvi_printf(" convergStateBufThr:%d\n", pCodecInst->rcInfo.convergStateBufThr);
		cvi_printf(" ConvergenceState:%d\n", pCodecInst->rcInfo.ConvergenceState);
		cvi_printf(" codec:%d\n", pCodecInst->rcInfo.codec);
		cvi_printf(" rcMode:%d\n", pCodecInst->rcInfo.rcMode);
		cvi_printf(" picIMinQp:%d\n", pCodecInst->rcInfo.picIMinQp);
		cvi_printf(" picIMaxQp:%d\n", pCodecInst->rcInfo.picIMaxQp);
		cvi_printf(" picPMinQp:%d\n", pCodecInst->rcInfo.picPMinQp);
		cvi_printf(" picPMaxQp:%d\n", pCodecInst->rcInfo.picPMaxQp);

		cvi_printf(" picMinMaxQpClipRange:%d\n", pCodecInst->rcInfo.picMinMaxQpClipRange);
		cvi_printf(" lastPicQp:%d\n", pCodecInst->rcInfo.lastPicQp);
		cvi_printf(" rcState:%d\n", pCodecInst->rcInfo.rcState);
		cvi_printf(" framerate:%d\n", pCodecInst->rcInfo.framerate);
		cvi_printf(" maxIPicBit:%d\n", pCodecInst->rcInfo.maxIPicBit);
		cvi_printf("cviApiMode:%d\n", pCodecInst->cviApiMode);
		cvi_printf("u64StartTime:%ld\n", pCodecInst->u64StartTime);
		cvi_printf("u64EndTime:%ld\n", pCodecInst->u64EndTime);
		cvi_printf("================end=================\n");
		cvi_printf("================memory info=================\n");
		uint32_t total_size = 0;
		vpu_buffer_t vbu_comm_bf;

		vdi_get_common_memory(pCodecInst->coreIdx, &vbu_comm_bf);
		cvi_printf("dts common_size:%d\n", vbu_comm_bf.size);
		if (!pCodecInst->isDecoder) {
			cvi_printf("ion bitstreamBufferSize:%d\n",
				pCodecInst->CodecInfo->encInfo.openParam.bitstreamBufferSize);

			cvi_printf("ion work_buf_size:%d\n", pCodecInst->CodecInfo->encInfo.vbWork.size);
				cvi_printf("TestEncConfig_size:%ld\n", sizeof(TestEncConfig));
			cvi_printf("ion reconFBsize:%d\n", pCodecInst->CodecInfo->encInfo.frameBufPool[0].size);
			cvi_printf("numFrameBuffers:%d\n", pCodecInst->CodecInfo->encInfo.numFrameBuffers);
			cvi_printf("ion vbScratch_size:%d\n", pCodecInst->CodecInfo->encInfo.vbScratch.size);
			cvi_printf("sram vbTemp_size:%d\n", pCodecInst->CodecInfo->encInfo.vbTemp.size);
			cvi_printf("ion vbMV_size:%d\n", pCodecInst->CodecInfo->encInfo.vbMV.size);
			cvi_printf("ion vbFbcYTbl_size:%d\n", pCodecInst->CodecInfo->encInfo.vbFbcYTbl.size);
			cvi_printf("ion vbFbcCTbl_size:%d\n", pCodecInst->CodecInfo->encInfo.vbFbcCTbl.size);
			cvi_printf("ion vbSubSamBuf_size:%d\n", pCodecInst->CodecInfo->encInfo.vbSubSamBuf.size);
			total_size = pCodecInst->CodecInfo->encInfo.openParam.bitstreamBufferSize
					 + pCodecInst->CodecInfo->encInfo.vbWork.size
					 + sizeof(TestEncConfig)
					 + pCodecInst->CodecInfo->encInfo.numFrameBuffers
					* pCodecInst->CodecInfo->encInfo.frameBufPool[0].size
					 + pCodecInst->CodecInfo->encInfo.vbScratch.size
					// + pCodecInst->CodecInfo->encInfo.vbTemp.size
					 + pCodecInst->CodecInfo->encInfo.vbFbcYTbl.size
					 + pCodecInst->CodecInfo->encInfo.vbFbcCTbl.size
					 + pCodecInst->CodecInfo->encInfo.vbSubSamBuf.size;
			cvi_printf("currnet enc instance's total_size:%d (%d KB)\n",
					total_size, total_size / 1000);
		} else {
			DecInfo *pDecInfo = &pCodecInst->CodecInfo->decInfo;

			cvi_printf("ion bitstreamBufferSize:%d\n", pDecInfo->openParam.bitstreamBufferSize);
			cvi_printf("work_buf_size:%d\n", pDecInfo->vbWork.size);
			cvi_printf("DecInfo_size:%ld\n", sizeof(DecInfo));
			cvi_printf("FrameBuffer_size:%d\n",
					pDecInfo->initialInfo.picWidth
				   * pDecInfo->initialInfo.picHeight
				   * pDecInfo->numFbsForDecoding);


			total_size = pDecInfo->openParam.bitstreamBufferSize
					 + pDecInfo->vbWork.size + sizeof(DecInfo)
					 + pDecInfo->initialInfo.picWidth
					 * pDecInfo->initialInfo.picHeight
					 * pDecInfo->numFbsForDecoding;
			cvi_printf("currnet dec instance's total_size:%d (%d KB)\n",
				total_size, total_size / 1000);
		}
	}

}


int vcodecH26xInstaccePoolInfo(int codec)
{
	int coreIdx  = 0;

	// if (argc != 2) {
	// 	cvi_printf("usage:%s idx\n", argv[0]);
	// 	cvi_printf("0: 265,1: 264");
	// 	return -1;
	// }

	coreIdx = codec;

	if (coreIdx > 1 || coreIdx < 0) {
		cvi_printf("0: 265,1: 264");
		return -1;
	}

	vdi_show_vdi_info(coreIdx);

	int i;
	vpu_instance_pool_t *vip;
	CodecInst *pCodecInst;

	vip = (vpu_instance_pool_t *)vdi_get_instance_pool(coreIdx);

	if (!vip) {
		cvi_printf("no resource in vdi instance pool\n");
		return -1;
	}

	cvi_printf("show inst pool info:\n");

	for (i = 0; i < MAX_NUM_INSTANCE; i++) {
		pCodecInst = (CodecInst *)vip->codecInstPool[i];

		showCodecInstPoolInfo(pCodecInst);
	}

	return 0;
}


uint32_t getVcodecMemoryInfo(int coreIdx)
{
	int i;
	vpu_instance_pool_t *vip;
	CodecInst *pCodecInst;
	uint32_t total_size = 0;
	uint32_t all_total_size = 0;
	vpu_buffer_t vbu_comm_bf;
	int bAddStreamSize = 0;

	vip = (vpu_instance_pool_t *)vdi_get_instance_pool(coreIdx);

	if (!vip) {
		cvi_printf("no resource in vdi instance pool\n");
		return -1;
	}

	cvi_printf("key infomation:\n");
	cvi_printf("is_single_es_buf:%d\n", vdi_get_is_single_es_buf(coreIdx));

	vdi_get_common_memory(coreIdx, &vbu_comm_bf);
	cvi_printf("dts common_size:%d\n", vbu_comm_bf.size);
	for (i = 0; i < MAX_NUM_INSTANCE; i++) {
		pCodecInst = (CodecInst *)vip->codecInstPool[i];

		if (pCodecInst && pCodecInst->inUse) {
			cvi_printf("================%s=================\n",
				pCodecInst->isDecoder ? "decode" : "encode");

			if (!pCodecInst->isDecoder) {
				EncInfo *pEncInfo = &pCodecInst->CodecInfo->encInfo;

				cvi_printf("ion bitstreamBufferSize:%d\n",
					pEncInfo->openParam.bitstreamBufferSize);

				cvi_printf("ion work_buf_size:%d\n", pEncInfo->vbWork.size);
				cvi_printf("mem stTestEncoder_size:%ld\n", sizeof(stTestEncoder));
				cvi_printf("ion reconFBsize:%d\n", pEncInfo->frameBufPool[0].size);
				cvi_printf("numFrameBuffers:%d\n", pEncInfo->numFrameBuffers);
				cvi_printf("ion vbScratch_size:%d\n", pEncInfo->vbScratch.size);
				cvi_printf("sram vbTemp_size:%d\n", pEncInfo->vbTemp.size);
				cvi_printf("ion vbMV_size:%d\n", pEncInfo->vbMV.size);
				cvi_printf("ion vbFbcYTbl_size:%d\n", pEncInfo->vbFbcYTbl.size);
				cvi_printf("ion vbFbcCTbl_size:%d\n", pEncInfo->vbFbcCTbl.size);
				cvi_printf("ion vbSubSamBuf_size:%d\n", pEncInfo->vbSubSamBuf.size);
				total_size = pEncInfo->vbWork.size
						 + sizeof(stTestEncoder)
						 + pEncInfo->numFrameBuffers
						 * pEncInfo->frameBufPool[0].size
						 + pEncInfo->vbScratch.size
						 + pEncInfo->vbMV.size
						 + pEncInfo->vbFbcYTbl.size
						 + pEncInfo->vbFbcCTbl.size
						 + pEncInfo->vbSubSamBuf.size;
				if (vdi_get_is_single_es_buf(coreIdx)) {
					if (!bAddStreamSize) {
						total_size += pEncInfo->openParam.bitstreamBufferSize;
						bAddStreamSize = 1;
					}
				} else {
					total_size += pEncInfo->openParam.bitstreamBufferSize;
				}

				cvi_printf("currnet enc instance's total_size:%d (%d KB)\n",
						total_size, total_size / 1024);
			} else {
				DecInfo *pDecInfo = &pCodecInst->CodecInfo->decInfo;

				cvi_printf("bitstreamBufferSize:%d\n", pDecInfo->openParam.bitstreamBufferSize);
				cvi_printf("common_size:%d\n", vbu_comm_bf.size);
				cvi_printf("work_buf_size:%d\n", pDecInfo->vbWork.size);
				cvi_printf("DecInfo_size:%ld\n", sizeof(DecInfo));
				cvi_printf("FrameBuffer_size:%d\n",
						pDecInfo->initialInfo.picWidth
					   * pDecInfo->initialInfo.picHeight
					   * pDecInfo->numFbsForDecoding);


				total_size = pDecInfo->openParam.bitstreamBufferSize
					     + pDecInfo->vbWork.size + sizeof(DecInfo)
					     + pDecInfo->initialInfo.picWidth
					     * pDecInfo->initialInfo.picHeight
					     * pDecInfo->numFbsForDecoding;
				cvi_printf("currnet dec instance's total_size:%d (%d KB)\n",
					total_size, total_size / 1024);
			}

			all_total_size += total_size;
			cvi_printf("================end=================\n");
		}
	}

	all_total_size += vbu_comm_bf.size;
	cvi_printf("all total_size:%d KB\n", all_total_size / 1024);

	return all_total_size;
}


int vcodecMemoryInfo(int32_t argc, char *argv[])
{
	int coreIdx  = 0;

	if (argc > 2) {
		cvi_printf("usage:%s idx\n", argv[0]);
		cvi_printf("usage:%s\n", argv[0]);
		cvi_printf("0: 265,1: 264");
		return -1;
	}

	if (argc == 2) {
		coreIdx = (int)atoi(argv[1]);

		if (coreIdx > 1 || coreIdx < 0) {
			cvi_printf("0: 265,1: 264");
			return -1;
		}
		getVcodecMemoryInfo(coreIdx);
		return 0;
	}

	uint32_t all_size = 0;

	all_size = getVcodecMemoryInfo(0)
		+ getVcodecMemoryInfo(1);

	cvi_printf("h264 and h265 use total_size:%d KB\n", all_size / 1000);
	return 0;
}

// void venc_vpuregs(int32_t argc, char **argv)
// {
// 	showVpuReg(argc, argv);
// }
// void venc_vcdbgmask(int32_t argc, char **argv)
// {
// 	vcodecModMaskDebug(argc, argv);
// }
// void venc_instPoolInfo(int32_t argc, char **argv)
// {
// 	vcodecH26xInstaccePoolInfo(argc, argv);
// }
// void venc_meminfo(int32_t argc, char **argv)
// {
// 	vcodecMemoryInfo(argc, argv);
// }

// // MSH_CMD_EXPORT_ALIAS(venc_vpuregs, venc_vpuregs, show vcodec regs);
// // MSH_CMD_EXPORT_ALIAS(venc_vcdbgmask, venc_vcdbgmask, get or set dbg_mask for vcodec);
// // MSH_CMD_EXPORT_ALIAS(venc_instPoolInfo, venc_instPoolInfo, get vcodec instance pool);
// // MSH_CMD_EXPORT_ALIAS(venc_meminfo, venc_meminfo, get vcodec mem info);

// const struct cli_command vcodec_cmds[] = {
// #if 1
// 	{"venc_vpuregs", "show vcodec regs", venc_vpuregs},
// 	{"venc_vcdbgmask", "get or set dbg_mask for vcodec", venc_vcdbgmask},
// 	{"venc_instPoolInfo", "get vcodec instance pool", venc_instPoolInfo},
// #endif
// 	{"venc_meminfo", "get vcodec mem info", venc_meminfo},
// };

// void vcodec_register_cmd(void)
// {
// 	int ret = 0;

// 	pthread_mutex_lock(&vcodecCliMutex);

// 	if (bVcodecCliReg) {
// 		pthread_mutex_unlock(&vcodecCliMutex);
// 		return;
// 	}

// 	bVcodecCliReg = 1;

// 	ret = aos_cli_register_commands(vcodec_cmds, sizeof(vcodec_cmds) / sizeof(struct cli_command));
// 	if (ret) {
// 		/* ������ */
// 		cvi_printf("test cmds register fail\r\n");
// 	}

// 	pthread_mutex_unlock(&vcodecCliMutex);

// }

