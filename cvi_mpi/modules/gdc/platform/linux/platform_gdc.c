#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/mman.h>
#include <vi_uapi.h>
#include <vpss_uapi.h>

#include "cvi_buffer.h"
#include "sys_internal.h"
#include "cvi_sys.h"
#include "cvi_vb.h"

#include "cvi_gdc.h"
#include "gdc_mesh.h"
#include "ldc_ioctl.h"

#define LDC_YUV_BLACK 0x808000
#define LDC_RGB_BLACK 0x0

#define CHECK_GDC_FORMAT(imgIn, imgOut)                                                                                \
	do {                                                                                                           \
		if (imgIn.stVFrame.enPixelFormat != imgOut.stVFrame.enPixelFormat) {                                   \
			CVI_TRACE_GDC(CVI_DBG_ERR, "in/out pixelformat(%d-%d) mismatch\n",                             \
				      imgIn.stVFrame.enPixelFormat, imgOut.stVFrame.enPixelFormat);                    \
			return CVI_ERR_GDC_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
		if (!GDC_SUPPORT_FMT(imgIn.stVFrame.enPixelFormat)) {                                                  \
			CVI_TRACE_GDC(CVI_DBG_ERR, "pixelformat(%d) unsupported\n", imgIn.stVFrame.enPixelFormat);     \
			return CVI_ERR_GDC_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
	} while (0)

static CVI_S32 ldc_fd = -1;
static pthread_mutex_t ldc_fd_lock = PTHREAD_MUTEX_INITIALIZER;

CVI_S32 get_ldc_fd(CVI_VOID)
{
	pthread_mutex_lock(&ldc_fd_lock);
	if (ldc_fd <= 0) {
		if (open_device(LDC_DEV_NAME, &ldc_fd) == -1) {
			perror("GDC open fail\n");
			ldc_fd = -1;
		}
	}
	pthread_mutex_unlock(&ldc_fd_lock);

	return ldc_fd;
}


static CVI_S32 gdc_rotation_check_size(ROTATION_E enRotation, const GDC_TASK_ATTR_S *pstTask)
{
	if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	if (enRotation == ROTATION_90 || enRotation == ROTATION_270 || enRotation == ROTATION_XY_FLIP) {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	} else {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 gdc_comm_cfg_frame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "Null pointer!\n");
		return CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize->u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize->u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/

CVI_S32 platform_gdc_suspend(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ldc_fd();

	s32Ret = gdc_suspend(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "suspend fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_gdc_resume(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ldc_fd();

	s32Ret = gdc_resume(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "resume fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_gdc_init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_ldc_fd();

	s32Ret = gdc_init(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "init fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_gdc_deinit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_ldc_fd();

	s32Ret = gdc_deinit(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "deinit fail\n");
		return s32Ret;
	}

	gdc_free_all_tsk_mesh();

	return s32Ret;
}

CVI_S32 platform_gdc_beginjob(GDC_HANDLE *phHandle)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, phHandle);

	CVI_S32 fd = get_ldc_fd();

	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	if (gdc_begin_job(fd, &cfg))
		return CVI_FAILURE;

	*phHandle = cfg.handle;

	return CVI_SUCCESS;
}

CVI_S32 platform_gdc_setjobidentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, identity_attr);

	if (!hHandle) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_GDC_NULL_PTR;
	}

	CVI_S32 fd = get_ldc_fd();

	struct gdc_identity_attr cfg = {0};

	cfg.handle = hHandle;
	memcpy(&cfg.attr, identity_attr, sizeof(*identity_attr));

	return gdc_set_job_identity(fd, &cfg);
}

CVI_S32 platform_gdc_endjob(GDC_HANDLE hHandle)
{
	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	CVI_S32 fd = get_ldc_fd();

	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.handle = hHandle;
	return gdc_end_job(fd, &cfg);
}

CVI_S32 platform_gdc_canceljob(GDC_HANDLE hHandle)
{
	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	CVI_S32 fd = get_ldc_fd();

	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.handle = hHandle;
	return gdc_cancel_job(fd, &cfg);
}

CVI_S32 platform_gdc_addrotationtask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	if (enRotation == ROTATION_180) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "do not support rotation 180\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	if (gdc_rotation_check_size(enRotation, pstTask) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_rotation_check_size fail\n");
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	CVI_S32 fd = get_ldc_fd();

	struct gdc_task_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	//memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	//attr.reserved = pstTask->reserved;
	attr.enRotation = enRotation;
	return gdc_add_rotation_task(fd, &attr);
}

CVI_S32 platform_gdc_addldctask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask
	, const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	CVI_S32 s32Ret;
	ROTATION_E rot[2];

	UNUSED(enRotation);

	if (pstLDCAttr->enRotation < ROTATION_0 || pstLDCAttr->enRotation >= ROTATION_MAX) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "ldc(%d) param invalid\n", pstLDCAttr->enRotation);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	if (pstLDCAttr->enRotation == 1) {
		rot[0] = ROTATION_90;
		rot[1] = ROTATION_0;
	} else if (pstLDCAttr->enRotation == 2) {
		rot[0] = ROTATION_90;
		rot[1] = ROTATION_90;
	} else if (pstLDCAttr->enRotation == 3) {
		rot[0] = ROTATION_270;
		rot[1] = ROTATION_0;
	} else {
		rot[0] = ROTATION_90;
		rot[1] = ROTATION_270;
	}

	if (!hHandle) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_GDC_NULL_PTR;
	}

	if (!pstLDCAttr) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "null pstLDCAttr");
		return CVI_ERR_GDC_NULL_PTR;
	}

	CVI_S32 fd = get_ldc_fd();

	if (pstLDCAttr->stGridInfoAttr.Enable) {
		rot[0] = ROTATION_270;
		rot[1] = ROTATION_90;
	}

	struct gdc_task_attr attr;
	SIZE_S stSizeTmp;
	PIXEL_FORMAT_E enPixelFormatTmp = pstTask->stImgIn.stVFrame.enPixelFormat;
	VIDEO_FRAME_INFO_S stVideoFrameTmp;
	CVI_U32 mesh_1st_size;
	VB_BLK blkTmp;

	stSizeTmp.u32Width = ALIGN(pstTask->stImgIn.stVFrame.u32Height, DEFAULT_ALIGN);
	stSizeTmp.u32Height = ALIGN(pstTask->stImgIn.stVFrame.u32Width, DEFAULT_ALIGN);

	s32Ret = gdc_comm_cfg_frame(&stSizeTmp, enPixelFormatTmp, &stVideoFrameTmp);
	if (s32Ret) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_comm_cfg_frame fail\n");
		return CVI_ERR_GDC_NOBUF;
	}

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &stVideoFrameTmp, sizeof(attr.stImgOut));
	attr.au64privateData[0] = pstTask->au64privateData[0];
	attr.reserved = pstTask->reserved;
	attr.enRotation = rot[0];
	s32Ret = gdc_add_ldc_task(fd, &attr);
	if (s32Ret) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_add_ldc_task 1st fail\n");
		goto FREE_TMP_FRAME;
	}

	mesh_gen_get_1st_size(stSizeTmp, &mesh_1st_size);
	memcpy(&attr.stImgIn, &stVideoFrameTmp, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	attr.au64privateData[0] = pstTask->au64privateData[0] + mesh_1st_size;
	attr.enRotation = rot[1];

	s32Ret = gdc_add_ldc_task(fd, &attr);
	if (s32Ret) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_add_ldc_task 2nd fail\n");
		goto FREE_TMP_FRAME;
	}

FREE_TMP_FRAME:
	blkTmp = CVI_VB_PhysAddr2Handle(stVideoFrameTmp.stVFrame.u64PhyAddr[0]);
	if (blkTmp != VB_INVALID_HANDLE)
		CVI_VB_ReleaseBlock(blkTmp);

	return s32Ret;
}

CVI_S32 platform_gdc_dumpmesh(MESH_DUMP_ATTR_S *pMeshDumpAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pMeshDumpAttr);

	CVI_U64 phyMesh;
	CVI_VOID *virMesh;
	CVI_U32 VpssGrp = 0, VpssChn = 0, ViPipe = 0, ViChn = 0;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size, mesh_2nd_size, mesh_size;
	CVI_U32 u32Width, u32Height;
	FILE *fp;
	CVI_S32 fd;
	MOD_ID_E enModId = pMeshDumpAttr->enModId;
	CVI_CHAR *filePath = pMeshDumpAttr->binFileName;
	CVI_S32 s32Ret;
	struct ldc_internal_chn_attr attr;
	CVI_CHAR mesh_name[128];
	TSK_MESH_ATTR_S tskMeshAttr;
	CVI_U8 idx;

	attr.enModId = enModId;
	fd = get_ldc_fd();

	switch (enModId) {
	case CVI_ID_VI:
		ViChn = pMeshDumpAttr->viMeshAttr.chn;
		ViPipe = pMeshDumpAttr->viMeshAttr.pipe;

		attr.stViChnAttr.ViPipe = ViPipe;
		attr.stViChnAttr.ViChn = ViChn;

		s32Ret = gdc_get_internal_chn_attr(fd, &attr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}

		u32Width = attr.stViChnAttr.stSize.u32Width;
		u32Height = attr.stViChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		snprintf(mesh_name, 128, "vi_%d_%d", ViPipe, ViChn);
		break;
	case CVI_ID_VPSS:
		VpssGrp = pMeshDumpAttr->vpssMeshAttr.grp;
		VpssChn = pMeshDumpAttr->vpssMeshAttr.chn;

		attr.stVpssChnAttr.VpssGrp = VpssGrp;
		attr.stVpssChnAttr.VpssChn = VpssChn;

		s32Ret = gdc_get_internal_chn_attr(fd, &attr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
			return s32Ret;
		}
		u32Width = attr.stVpssChnAttr.stSize.u32Width;
		u32Height = attr.stVpssChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	out_size.u32Width = in_size.u32Width;
	out_size.u32Height = in_size.u32Height;

	mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
	mesh_size = mesh_1st_size + mesh_2nd_size;

	strcpy(tskMeshAttr.Name, mesh_name);
	idx = gdc_get_tsk_mesh_by_name(&tskMeshAttr);

	if (idx >= GDC_MAX_TSK_MESH) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "tsk mesh count(%d) is out of range(%d)\n", idx + 1, GDC_MAX_TSK_MESH);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	phyMesh = tskMeshAttr.paddr;
	virMesh = tskMeshAttr.vaddr;
	CVI_TRACE_GDC(CVI_DBG_DEBUG, "dump mesh size:%d, mesh phy addr:%#"PRIx64", vir addr:%p.\n",
		mesh_size, phyMesh, virMesh);

	fp = fopen(filePath, "wb");
	if (!fp) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "open file:%s failed.\n", filePath);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	fwrite(virMesh, mesh_size, 1, fp);
	fflush(fp);
	fclose(fp);
	return s32Ret;
}

CVI_S32 platform_gdc_loadmesh(MESH_DUMP_ATTR_S *pMeshDumpAttr, const LDC_ATTR_S *pstLDCAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pMeshDumpAttr);

	CVI_U64 phyMesh;
	CVI_VOID *virMesh;
	CVI_U32 VpssGrp = 0, VpssChn = 0, ViPipe = 0, ViChn = 0;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size, mesh_2nd_size, mesh_size;
	CVI_U32 u32Width, u32Height;
	struct cvi_gdc_mesh mesh;
	FILE *fp;
	CVI_S32 fd;
	MOD_ID_E enModId = pMeshDumpAttr->enModId;
	CVI_CHAR *filePath = pMeshDumpAttr->binFileName;
	CVI_S32 s32Ret;
	struct ldc_internal_chn_attr attr;
	struct ldc_internal_chn_ldc_cfg cfg;
	CVI_CHAR mesh_name[128];
	struct ldc_vi_chn_ldc_cfg stViCfg;
	struct ldc_vpss_chn_ldc_cfg stVpssCfg;

	attr.enModId = enModId;
	cfg.enModId = enModId;
	fd = get_ldc_fd();

	switch (enModId) {
	case CVI_ID_VI:
		ViChn = pMeshDumpAttr->viMeshAttr.chn;
		ViPipe = pMeshDumpAttr->viMeshAttr.pipe;

		attr.stViChnAttr.ViPipe = ViPipe;
		attr.stViChnAttr.ViChn = ViChn;

		s32Ret = gdc_get_internal_chn_attr(fd, &attr);

		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}

		u32Width = attr.stViChnAttr.stSize.u32Width;
		u32Height = attr.stViChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		snprintf(mesh_name, 128, "vi_%d_%d", ViPipe, ViChn);
		break;
	case CVI_ID_VPSS:
		VpssGrp = pMeshDumpAttr->vpssMeshAttr.grp;
		VpssChn = pMeshDumpAttr->vpssMeshAttr.chn;

		attr.stVpssChnAttr.VpssGrp = VpssGrp;
		attr.stVpssChnAttr.VpssChn = VpssChn;

		s32Ret = gdc_get_internal_chn_attr(fd, &attr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
			return s32Ret;
		}
		u32Width = attr.stVpssChnAttr.stSize.u32Width;
		u32Height = attr.stVpssChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	out_size.u32Width = in_size.u32Width;
	out_size.u32Height = in_size.u32Height;

	mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
	mesh_size = mesh_1st_size + mesh_2nd_size;

	fp = fopen(filePath, "rb");
	if (!fp) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "open file:%s failed.\n", filePath);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);

	if (mesh_size != (CVI_U32)fileSize) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "loadmesh file:(%s) size is not match.\n", filePath);
		fclose(fp);
		return CVI_FAILURE;
	}
	rewind(fp);

	// acquire memory space for mesh.
	if (CVI_SYS_IonAlloc_Cached(&phyMesh, &virMesh, mesh_name, mesh_size) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Can't acquire memory for gdc mesh.\n");
		fclose(fp);
		return CVI_ERR_GDC_NOMEM;
	}

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "load mesh size:%d, mesh phy addr:%#"PRIx64", vir addr:%p.\n",
		mesh_size, phyMesh, virMesh);
	mesh.paddr = phyMesh;
	mesh.vaddr = virMesh;

	fread(virMesh, mesh_size, 1, fp);
	CVI_SYS_IonFlushCache(phyMesh, virMesh, mesh_size);

	gdc_free_cur_tsk_mesh(mesh_name);
	if (gdc_set_tsk_mesh_by_name(mesh_name, phyMesh, virMesh)) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_set_tsk_mesh_by_name fail.\n");
		fclose(fp);
		return CVI_ERR_GDC_NOMEM;
	}

	switch (enModId) {
	case CVI_ID_VI:
		stViCfg.ViPipe = ViPipe;
		stViCfg.ViChn = ViChn;
		stViCfg.stLDCAttr.bEnable = CVI_TRUE;
		memcpy(&stViCfg.stLDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		stViCfg.meshHandle = mesh.paddr;
		cfg.stViCfg = stViCfg;

		if (gdc_set_internal_chn_ldc_cfg(fd, &cfg) != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "VI Set Chn(%d) LDC fail\n", ViChn);
			fclose(fp);
			return CVI_FAILURE;
		}

		break;
	case CVI_ID_VPSS:
		stVpssCfg.VpssGrp = VpssGrp;
		stVpssCfg.VpssChn = VpssChn;
		stVpssCfg.stLDCAttr.bEnable = CVI_TRUE;
		memcpy(&stVpssCfg.stLDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		stVpssCfg.meshHandle = mesh.paddr;
		cfg.stVpssCfg = stVpssCfg;

		if (gdc_set_internal_chn_ldc_cfg(fd, &cfg) != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "VPSS Set Chn(%d) LDC fail\n", VpssChn);
			fclose(fp);
			return CVI_FAILURE;
		}
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		fclose(fp);
		return CVI_ERR_GDC_NOT_SUPPORT;
	}
	fclose(fp);
	return s32Ret;
}

CVI_S32 platform_gdc_loadmeshwithbuf(MESH_DUMP_ATTR_S *pMeshDumpAttr,
	const LDC_ATTR_S *pstLDCAttr, CVI_VOID *pBuf, CVI_U32 Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pMeshDumpAttr);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstLDCAttr);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pBuf);

	CVI_U64 phyMesh;
	CVI_VOID *virMesh;
	CVI_U32 VpssGrp = 0, VpssChn = 0, ViPipe = 0, ViChn = 0;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size, mesh_2nd_size, mesh_size;
	CVI_U32 u32Width, u32Height;
	struct cvi_gdc_mesh mesh;
	CVI_S32 fd;
	MOD_ID_E enModId = pMeshDumpAttr->enModId;
	CVI_S32 s32Ret;
	struct ldc_internal_chn_attr attr;
	struct ldc_internal_chn_ldc_cfg cfg;
	CVI_CHAR mesh_name[128];
	struct ldc_vi_chn_ldc_cfg stViCfg;
	struct ldc_vpss_chn_ldc_cfg stVpssCfg;

	attr.enModId = enModId;
	cfg.enModId = enModId;
	fd = get_ldc_fd();

	switch (enModId) {
	case CVI_ID_VI:
		ViChn = pMeshDumpAttr->viMeshAttr.chn;
		ViPipe = pMeshDumpAttr->viMeshAttr.pipe;

		attr.stViChnAttr.ViPipe = ViPipe;
		attr.stViChnAttr.ViChn = ViChn;

		s32Ret = gdc_get_internal_chn_attr(fd, &attr);

		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}

		u32Width = attr.stViChnAttr.stSize.u32Width;
		u32Height = attr.stViChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		snprintf(mesh_name, 128, "vi_%d_%d", ViPipe, ViChn);
		break;
	case CVI_ID_VPSS:
		VpssGrp = pMeshDumpAttr->vpssMeshAttr.grp;
		VpssChn = pMeshDumpAttr->vpssMeshAttr.chn;

		attr.stVpssChnAttr.VpssGrp = VpssGrp;
		attr.stVpssChnAttr.VpssChn = VpssChn;

		s32Ret = gdc_get_internal_chn_attr(fd, &attr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
			return s32Ret;
		}
		u32Width = attr.stVpssChnAttr.stSize.u32Width;
		u32Height = attr.stVpssChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	out_size.u32Width = in_size.u32Width;
	out_size.u32Height = in_size.u32Height;

	mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
	mesh_size = mesh_1st_size + mesh_2nd_size;

	if (mesh_size != Len) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "invalid param, Len[%d] not match with MOD[%d] meshsize[%d]\n", Len, enModId, mesh_size);
		return CVI_FAILURE;
	}

	// acquire memory space for mesh.
	if (CVI_SYS_IonAlloc_Cached(&phyMesh, &virMesh, mesh_name, mesh_size) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Can't acquire memory for gdc mesh.\n");
		return CVI_ERR_GDC_NOMEM;
	}

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "load mesh size:%d, mesh phy addr:%#"PRIx64", vir addr:%p.\n",
		mesh_size, phyMesh, virMesh);
	mesh.paddr = phyMesh;
	mesh.vaddr = virMesh;

	memcpy(virMesh, pBuf, Len);
	CVI_SYS_IonFlushCache(phyMesh, virMesh, mesh_size);

	gdc_free_cur_tsk_mesh(mesh_name);
	if (gdc_set_tsk_mesh_by_name(mesh_name, phyMesh, virMesh)) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_set_tsk_mesh_by_name fail.\n");
		return CVI_ERR_GDC_NOMEM;
	}

	switch (enModId) {
	case CVI_ID_VI:
		stViCfg.ViPipe = ViPipe;
		stViCfg.ViChn = ViChn;
		stViCfg.stLDCAttr.bEnable = CVI_TRUE;
		memcpy(&stViCfg.stLDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		stViCfg.meshHandle = mesh.paddr;
		cfg.stViCfg = stViCfg;

		if (gdc_set_internal_chn_ldc_cfg(fd, &cfg) != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "VI Set Chn(%d) LDC fail\n", ViChn);
			return CVI_FAILURE;
		}

		break;
	case CVI_ID_VPSS:
		stVpssCfg.VpssGrp = VpssGrp;
		stVpssCfg.VpssChn = VpssChn;
		stVpssCfg.stLDCAttr.bEnable = CVI_TRUE;
		memcpy(&stVpssCfg.stLDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		stVpssCfg.meshHandle = mesh.paddr;
		cfg.stVpssCfg = stVpssCfg;

		if (gdc_set_internal_chn_ldc_cfg(fd, &cfg) != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "VPSS Set Chn(%d) LDC fail\n", VpssChn);
			return CVI_FAILURE;
		}
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}
	return s32Ret;
}

CVI_S32 platform_gdc_getworkjob(GDC_HANDLE *phHandle)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, phHandle);
	CVI_S32 fd = get_ldc_fd();

	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	if (gdc_get_work_job(fd, &cfg))
		return CVI_FAILURE;

	*phHandle = cfg.handle;
	return CVI_SUCCESS;
}

CVI_S32 platform_gdc_getchnframe(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_GDC, identity);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstFrameInfo);

	CVI_S32 fd = get_ldc_fd();

	struct gdc_chn_frm_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	memcpy(&cfg.identity.attr, identity, sizeof(*identity));
	cfg.MilliSec = s32MilliSec;

	s32Ret = gdc_get_chn_frm(fd, &cfg);
	if (s32Ret) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "identity[%s-%d-%d] get chn frame fail, Ret[%d]\n"
			, identity->Name, identity->enModId, identity->u32ID, s32Ret);
		return s32Ret;
	}
	memcpy(pstFrameInfo, &cfg.VideoFrame, sizeof(*pstFrameInfo));

	return s32Ret;
}

CVI_S32 platform_gdc_genldcmesh(CVI_U32 u32Width, CVI_U32 u32Height, const LDC_ATTR_S *pstLDCAttr,
		const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr)
{
	return gdc_gen_ldcmesh(u32Width, u32Height, pstLDCAttr, name, pu64PhyAddr, ppVirAddr);
}

CVI_S32 platform_gdc_loadldcmesh(CVI_U32 u32Width, CVI_U32 u32Height, const char *fileNname
	, const char *tskName, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr)
{
	return gdc_load_ldcmesh(u32Width, u32Height, fileNname, tskName, pu64PhyAddr, ppVirAddr);
}

CVI_VOID platform_gdc_freecurtaskmesh(CVI_CHAR *tskName)
{
	gdc_free_cur_tsk_mesh(tskName);
}

CVI_S32 platform_gdc_attachvbpool(MMF_CHN_S *pChn, VB_POOL u32VbPool)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pChn);
	CVI_S32 fd = get_ldc_fd();
	struct ldc_vb_pool_cfg cfg;

	cfg.Chn = *pChn;
	cfg.VbPool = u32VbPool;

	return gdc_attach_vbpool(fd, &cfg);
}

CVI_S32 platform_gdc_detachvbpool(MMF_CHN_S *pChn)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pChn);
	CVI_S32 fd = get_ldc_fd();
	struct ldc_vb_pool_cfg cfg;

	cfg.Chn = *pChn;
	cfg.VbPool = -1;

	return gdc_detach_vbpool(fd, &cfg);
}
