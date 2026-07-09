
#include <stdlib.h>
#include <stdint.h>
#include "cvi_sys.h"
#include "cvi_vo.h"
#include "cvi_vpss.h"
#include "cvi_vi.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

// sys module -----------------------------------------------------------------------
CVI_S32 *log_levels;
CVI_CHAR const *log_name[8] = {
	(CVI_CHAR *)"EMG", (CVI_CHAR *)"ALT", (CVI_CHAR *)"CRI", (CVI_CHAR *)"ERR",
	(CVI_CHAR *)"WRN", (CVI_CHAR *)"NOT", (CVI_CHAR *)"INF", (CVI_CHAR *)"DBG"
};

void *CVI_SYS_MmapCache(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	UNUSED(u32Size);
	return (void *) (uintptr_t) u64PhyAddr;
}

CVI_S32 CVI_SYS_IonAlloc_Cached(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
				 const CVI_CHAR *strName, CVI_U32 u32Len)
{
	UNUSED(strName);
	*ppVirAddr = (CVI_VOID *) calloc(1, u32Len);
	*pu64PhyAddr = (CVI_U64) (uintptr_t) *ppVirAddr;
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonFree(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr)
{
	UNUSED(u64PhyAddr);
	free(pVirAddr);
	return CVI_SUCCESS;
}

void *CVI_SYS_Mmap(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	UNUSED(u32Size);
	return (void *) (uintptr_t) u64PhyAddr;
}

CVI_S32 CVI_SYS_Munmap(void *pVirAddr, CVI_U32 u32Size)
{
	UNUSED(pVirAddr);
	UNUSED(u32Size);
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonInvalidateCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	UNUSED(u64PhyAddr);
	UNUSED(pVirAddr);
	UNUSED(u32Len);
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonFlushCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	UNUSED(u64PhyAddr);
	UNUSED(pVirAddr);
	UNUSED(u32Len);
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetBindbySrc(const MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
	UNUSED(pstSrcChn);
	UNUSED(pstBindDest);
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_UnBind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	UNUSED(pstSrcChn);
	UNUSED(pstDestChn);
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_Bind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	UNUSED(pstSrcChn);
	UNUSED(pstDestChn);
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetVIVPSSMode(VI_VPSS_MODE_S *pstVIVPSSMode)
{
	memset(pstVIVPSSMode, 0, sizeof(VI_VPSS_MODE_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetChipId(CVI_U32 *pu32ChipId)
{
	*pu32ChipId = 28;
	return CVI_SUCCESS;
}

int devm_open(void)
{
	return 0;
}

void devm_close(int fd)
{
	UNUSED(fd);
}

void *devm_map(int fd, uint64_t phy_addr, size_t len)
{
	UNUSED(fd);
	UNUSED(len);
	return (void *)(uintptr_t)phy_addr;
}

void devm_unmap(void *virt_addr, size_t len)
{
	UNUSED(virt_addr);
	UNUSED(len);
}

// vo module -----------------------------------------------------------------------
static VO_GAMMA_INFO_S g_vo_gamma_info;

CVI_S32 CVI_VO_GetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	*pinfo = g_vo_gamma_info;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	g_vo_gamma_info = *pinfo;
	return CVI_SUCCESS;
}

// vpss module -----------------------------------------------------------------------
static VPSS_ALL_PROC_AMP_S g_vpss_all_proc_amp;

CVI_S32 CVI_VPSS_GetAllProcAmp(VPSS_ALL_PROC_AMP_S *pstProcAmp)
{
	*pstProcAmp = g_vpss_all_proc_amp;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value)
{
	if (VpssGrp < 0 || VpssGrp >= VPSS_MAX_GRP_NUM || type < 0 || type >= PROC_AMP_MAX) {
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	*value = g_vpss_all_proc_amp.s32ProcAmp[VpssGrp][type];
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, const CVI_S32 value)
{
	if (VpssGrp < 0 || VpssGrp >= VPSS_MAX_GRP_NUM || type < 0 || type >= PROC_AMP_MAX) {
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	g_vpss_all_proc_amp.s32ProcAmp[VpssGrp][type] = value;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpParamfromBin(VPSS_GRP VpssGrp, VPSS_BIN_DATA *bin_data)
{
	if (VpssGrp < 0 || VpssGrp >= VPSS_MAX_GRP_NUM || bin_data == NULL) {
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	for (int i = 0; i < PROC_AMP_MAX; i++) {
		g_vpss_all_proc_amp.s32ProcAmp[VpssGrp][i] = bin_data->proc_amp[i];
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr)
{
	UNUSED(VpssGrp);
	UNUSED(VpssChn);
	memset(pstLDCAttr, 0, sizeof(VPSS_LDC_ATTR_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr)
{
	UNUSED(VpssGrp);
	UNUSED(VpssChn);
	UNUSED(pstLDCAttr);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ReleaseChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	UNUSED(VpssGrp);
	UNUSED(VpssChn);
	UNUSED(pstVideoFrame);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstFrameInfo,
				 CVI_S32 s32MilliSec)
{
	UNUSED(VpssGrp);
	UNUSED(VpssChn);
	memset(pstFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));
	UNUSED(s32MilliSec);
	return CVI_SUCCESS;
}

// vi module -----------------------------------------------------------------------
//CVI_S32 CVI_VI_QueryDevStatus(VI_PIPE ViPipe)
//{
//    UNUSED(ViPipe);
//    return CVI_SUCCESS;
//}

CVI_S32 CVI_VI_GetChnLDCAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_LDC_ATTR_S *pstLDCAttr)
{
	UNUSED(ViPipe);
	UNUSED(ViChn);
	memset(pstLDCAttr, 0, sizeof(VI_LDC_ATTR_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetChnLDCAttr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr)
{
	UNUSED(ViPipe);
	UNUSED(ViChn);
	UNUSED(pstLDCAttr);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_ReleaseChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	UNUSED(ViPipe);
	UNUSED(ViChn);
	UNUSED(pstFrameInfo);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_ReleasePipeFrame(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	UNUSED(ViPipe);
	UNUSED(pstFrameInfo);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	UNUSED(ViPipe);
	UNUSED(ViChn);
	memset(pstFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));
	UNUSED(s32MilliSec);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetDevAttr(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
	UNUSED(ViDev);
	memset(pstDevAttr, 0, sizeof(VI_DEV_ATTR_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	UNUSED(ViPipe);
	UNUSED(ViChn);
	memset(pstChnAttr, 0, sizeof(VI_CHN_ATTR_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetPipeDumpAttr(VI_PIPE ViPipe, const VI_DUMP_ATTR_S *pstDumpAttr)
{
	UNUSED(ViPipe);
	UNUSED(pstDumpAttr);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeAttr(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	UNUSED(ViPipe);
	memset(pstPipeAttr, 0, sizeof(VI_PIPE_ATTR_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_DumpHwRegisterToFile(VI_PIPE ViPipe, FILE *fp, VI_DUMP_REGISTER_TABLE_S *pstRegTbl)
{
	UNUSED(ViPipe);
	UNUSED(fp);
	UNUSED(pstRegTbl);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeFrame(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	UNUSED(ViPipe);
	memset(pstFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));
	UNUSED(s32MilliSec);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetBypassFrm(VI_PIPE ViPipe, CVI_U8 bypass_num)
{
	UNUSED(ViPipe);
	UNUSED(bypass_num);
	return CVI_SUCCESS;
}

// VB module -----------------------------------------------------------------------
CVI_S32 CVI_VB_ReleaseBlock(VB_BLK Block)
{
	UNUSED(Block);
	return CVI_SUCCESS;
}

CVI_S32 CVI_VB_DestroyPool(VB_POOL Pool)
{
	UNUSED(Pool);
	return CVI_SUCCESS;
}

VB_POOL CVI_VB_CreatePool(VB_POOL_CONFIG_S *pstVbPoolCfg)
{
	UNUSED(pstVbPoolCfg);
	return CVI_SUCCESS;
}

VB_BLK CVI_VB_GetBlock(VB_POOL Pool, CVI_U32 u32BlkSize)
{
	UNUSED(Pool);
	UNUSED(u32BlkSize);
	return 0;
}

CVI_U64 CVI_VB_Handle2PhysAddr(VB_BLK Block)
{
	UNUSED(Block);
	return 0;
}
