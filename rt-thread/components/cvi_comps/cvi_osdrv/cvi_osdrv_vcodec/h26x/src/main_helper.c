//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2014  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include "errno.h"
#include "vpuapifunc.h"
#include "coda9_regdefine.h"
#include "common_vpuconfig.h"
#include "common_regdefine.h"
#include "wave4_regdefine.h"
#include "vpuerror.h"
#include "main_helper.h"
#include "getopt.h"
#include "fw_h264.h"
#include "fw_h265.h"
#include "vdi_osal.h"
#include "malloc.h"

#define BIT_DUMMY_READ_GEN 0x06000000
#define BIT_READ_LATENCY 0x06000004
#define W4_SET_READ_DELAY 0x01000000
#define W4_SET_WRITE_DELAY 0x01000004
#define MAX_CODE_BUF_SIZE (512 * 1024)

#ifdef PLATFORM_WIN32
#pragma warning(disable : 4996)
	//!<< disable waring C4996: The POSIX name for
	//!<this item is deprecated.
#endif

char *EncPicTypeStringH264[] = {
	"IDR/I",
	"P",
};

char *EncPicTypeStringMPEG4[] = {
	"I",
	"P",
};

char *productNameList[] = {
	"CODA980", "CODA960", "CODA7503", "WAVE320",  "WAVE410", "WAVE4102",
	"WAVE420", "WAVE412", "WAVE7Q",	  "WAVE420L", "WAVE510", "WAVE512",
	"WAVE515", "WAVE520", "Unknown",  "Unknown",
};

Int32 LoadFirmware_EX(Uint32 coreIdx, Uint8 **retFirmware, Uint32 *retSizeInWord)
{
	Uint32 readSize;

	if (coreIdx == 0) {
        readSize     = fw_h265_size;
        *retFirmware = fw_h265;
	} else if (coreIdx == 1) {
        readSize     = fw_h264_size;
        *retFirmware = fw_h264;
	} else {
		CVI_VC_ERR("coreIdx = %d\n", coreIdx);
		return -1;
	}

	readSize = ((readSize + 1) >> 1) << 1;
	*retSizeInWord = readSize >> 1;

	return 0;
}

Int32 LoadFirmwareH(Int32 productId, Uint8 **retFirmware, Uint32 *retSizeInWord)
{
	Uint8 *firmware = NULL, *fw_h = NULL;
	Uint32 readSize;

	if (productId == PRODUCT_ID_420L) {
        readSize = fw_h265_size;
        fw_h     = fw_h265;
    } else if (productId == PRODUCT_ID_980) {
        readSize = fw_h264_size;
        fw_h     = fw_h264;
    } else {
		CVI_VC_ERR("productId = %d\n", productId);
		return -1;
	}

	CVI_VC_TRACE("productId = %d\n", productId);

	readSize = ((readSize + 1) >> 1) << 1;
	firmware = (Uint8 *)malloc(readSize);
	if (firmware == NULL) {
		CVI_VC_ERR("allocation fail, firmware !\n");
		return (-1);
	}

	memcpy(firmware, fw_h, sizeof(char)* readSize);
	*retSizeInWord = readSize >> 1;

	*retFirmware = firmware;

	return 0;
}

void PrintVpuVersionInfo(Uint32 core_idx)
{
	Uint32 version;
	Uint32 revision;
	Uint32 productId;

	VPU_GetVersionInfo(core_idx, &version, &revision, &productId);

	CVI_VC_INFO( "VPU coreNum : [%d]\n", core_idx);
	CVI_VC_INFO(
	     "Firmware : CustomerCode: %04x | version : %d.%d.%d rev.%d\n",
	     (Uint32)(version >> 16), (Uint32)((version >> (12)) & 0x0f),
	     (Uint32)((version >> (8)) & 0x0f), (Uint32)((version)&0xff),
	     revision);
	CVI_VC_INFO( "Hardware : %04x\n", productId);
	CVI_VC_INFO( "API      : %d.%d.%d\n\n", API_VERSION_MAJOR,
	     API_VERSION_MINOR, API_VERSION_PATCH);
}

void PrintDecSeqWarningMessages(Uint32 productId, DecInitialInfo *seqInfo)
{
	if (PRODUCT_ID_W_SERIES(productId)) {
		if (seqInfo->seqInitErrReason & 0x00000001)
			CVI_VC_WARN(
			     "sps_max_sub_layer_minus1 shall be 0 to 6\n");
		if (seqInfo->seqInitErrReason & 0x00000002)
			CVI_VC_WARN(
			     "general_reserved_zero_44bits shall be 0.\n");
		if (seqInfo->seqInitErrReason & 0x00000004)
			CVI_VC_WARN( "reserved_zero_2bits shall be 0\n");
		if (seqInfo->seqInitErrReason & 0x00000008)
			CVI_VC_WARN( "sub_layer_reserved_zero_44bits shall be 0");
		if (seqInfo->seqInitErrReason & 0x00000010)
			CVI_VC_WARN(
			     "general_level_idc shall have one of level of Table A.1\n");
		if (seqInfo->seqInitErrReason & 0x00000020)
			CVI_VC_WARN(
			     "sps_max_dec_pic_buffering[i] <= MaxDpbSize\n");
		if (seqInfo->seqInitErrReason & 0x00000040)
			CVI_VC_WARN(
			     "trailing bits shall be 1000... pattern, 7.3.2.1\n");
		if (seqInfo->seqInitErrReason & 0x00100000)
			CVI_VC_WARN( "Not supported or undefined profile: %d\n",
			     seqInfo->profile);
		if (seqInfo->seqInitErrReason & 0x00200000)
			CVI_VC_WARN( "Spec over level(%d)\n", seqInfo->level);
	}
}

void DisplayDecodedInformationForHevc(DecHandle handle, Uint32 frameNo,
				      DecOutputInfo *decodedInfo)
{
	Int32 logLevel;

	if (decodedInfo == NULL) {
		{
			CVI_VC_INFO(
			     "I    NO  T     POC     NAL  DECO  DISP PRESCAN DISPFLAG  RD_PTR   WR_PTR FRM_START FRM_END   WxH      SEQ  TEMP CYCLE\n");
		}
		CVI_VC_INFO(
		     "------------------------------------------------------------------------------------------------------------\n");
	} else {
		logLevel = (decodedInfo->decodingSuccess & 0x01) == 0 ? ERR : TRACE;
		if (handle->productId == PRODUCT_ID_4102) {
			logLevel = (decodedInfo->indexFramePrescan == -2) ? TRACE : logLevel;
		}
		// Print informations
		{
			CVI_VC_INFO(
			     "%02d %5d %d %4d(%4d) %3d %2d(%2d) %2d(%2d) %7d %08x %08x %08x %08lx %08lx %4dx%-4d %4d %4d %d\n",
			     handle->instIndex, frameNo, decodedInfo->picType,
			     decodedInfo->h265Info.decodedPOC,
			     decodedInfo->h265Info.displayPOC,
			     decodedInfo->nalType,
			     decodedInfo->indexFrameDecoded,
			     decodedInfo->indexFrameDecodedForTiled,
			     decodedInfo->indexFrameDisplay,
			     decodedInfo->indexFrameDisplayForTiled,
			     decodedInfo->indexFramePrescan,
			     decodedInfo->frameDisplayFlag, decodedInfo->rdPtr,
			     decodedInfo->wrPtr, decodedInfo->bytePosFrameStart,
			     decodedInfo->bytePosFrameEnd,
			     decodedInfo->dispPicWidth,
			     decodedInfo->dispPicHeight,
			     decodedInfo->sequenceNo,
			     decodedInfo->h265Info.temporalId,
			     decodedInfo->frameCycle);
		}
		if (logLevel == ERR) {
			CVI_VC_ERR( "\t>>ERROR REASON: 0x%08x(0x%08x)\n",
			     decodedInfo->errorReason,
			     decodedInfo->errorReasonExt);
		}
		if (decodedInfo->numOfErrMBs) {
			CVI_VC_WARN( "\t>> ErrorBlock: %d\n",
			     decodedInfo->numOfErrMBs);
		}
	}
}

void DisplayDecodedInformationCommon(DecHandle handle, CodStd codec,
				     Uint32 frameNo, DecOutputInfo *decodedInfo)
{
	Int32 logLevel;

	if (decodedInfo == NULL) {
		// Print header
		CVI_VC_INFO(
		     "I  NO    T TID DEC_POC   DECO   DISP  DISPFLAG RD_PTR   WR_PTR   FRM_START FRM_END WxH\n");
		CVI_VC_INFO(
		     "---------------------------------------------------------------------------------------\n");
	} else {
		VpuRect rc = decodedInfo->rcDisplay;
		Uint32 width = rc.right - rc.left;
		Uint32 height = rc.bottom - rc.top;
		char strTemporalId[16];
		char strPoc[16];

#ifdef SUPPORT_980_ROI_RC_LIB
		strcpy(strTemporalId, "-");
		strcpy(strPoc, "--------");
#endif

		logLevel = (decodedInfo->decodingSuccess & 0x01) == 0 ? ERR : TRACE;
		// Print informations
		CVI_VC_INFO(
		     "%02d %5d %d  %s  %s %2d(%2d) %2d(%2d) %08x %08x %08x %08x  %08x %dx%d\n",
		     handle->instIndex, frameNo, decodedInfo->picType,
		     strTemporalId, strPoc, decodedInfo->indexFrameDecoded,
		     decodedInfo->indexFrameDecodedForTiled,
		     decodedInfo->indexFrameDisplay,
		     decodedInfo->indexFrameDisplayForTiled,
		     decodedInfo->frameDisplayFlag, decodedInfo->rdPtr,
		     decodedInfo->wrPtr, (Uint32)decodedInfo->bytePosFrameStart,
		     (Uint32)decodedInfo->bytePosFrameEnd, width, height);
		if (logLevel == ERR) {
			CVI_VC_ERR( "\t>>ERROR REASON: 0x%08x(0x%08x)\n",
			     decodedInfo->errorReason,
			     decodedInfo->errorReasonExt);
		}
		if (decodedInfo->numOfErrMBs) {
			CVI_VC_WARN( "\t>> ErrorBlock: %d\n",
			     decodedInfo->numOfErrMBs);
		}
	}
}

/**
 * \brief                   Print out decoded information such like RD_PTR,
 * WR_PTR, PIC_TYPE, .. \param   decodedInfo     If this parameter is not NULL
 * then print out decoded informations otherwise print out header.
 */
void DisplayDecodedInformation(DecHandle handle, CodStd codec, Uint32 frameNo,
			       DecOutputInfo *decodedInfo)
{
	switch (codec) {
	case STD_HEVC:
		DisplayDecodedInformationForHevc(handle, frameNo, decodedInfo);
		break;
	default:
		DisplayDecodedInformationCommon(handle, codec, frameNo,
						decodedInfo);
		break;
	}
}

static void Wave4DisplayEncodedInformation(EncHandle handle,
					   EncOutputInfo *encodedInfo,
					   Int32 srcEndFlag, Int32 srcFrameIdx)
{
	if (encodedInfo == NULL) {
#ifdef REPORT_PIC_SUM_VAR
		CVI_VC_INFO( "--------------------------------------------------------------------------------------------\n");
		CVI_VC_INFO( "I     NO     T   RECON   RD_PTR    WR_PTR     BYTES  SRCIDX  USEDSRCIDX Cycle    SumVariance\n");
		CVI_VC_INFO( "--------------------------------------------------------------------------------------------\n");
#else
		CVI_VC_INFO( "------------------------------------------------------------------------------\n");
		CVI_VC_INFO( "C    I     NO     T   RECON   RD_PTR    WR_PTR     BYTES  SRCIDX  USEDSRCIDX Cycle\n");
		CVI_VC_INFO( "------------------------------------------------------------------------------\n");
#endif
	} else {
#ifdef REPORT_PIC_SUM_VAR
		CVI_VC_INFO(
		     "HEVC %02d %5d %5d %5d    %08lx  %08lx %8x     %2d        %2d    %8d   %8d\n",
		     handle->instIndex, encodedInfo->encPicCnt,
		     encodedInfo->picType, encodedInfo->reconFrameIndex,
		     encodedInfo->rdPtr, encodedInfo->wrPtr,
		     encodedInfo->bitstreamSize,
		     (srcEndFlag == 1 ? -1 : srcFrameIdx),
		     encodedInfo->encSrcIdx, encodedInfo->frameCycle,
		     encodedInfo->sumPicVar);
#else
		CVI_VC_INFO(
		     "HEVC %02d %5d %5d %5d    %08x  %08x %8x     %2d        %2d    %8d\n",
		     handle->instIndex, encodedInfo->encPicCnt,
		     encodedInfo->picType, encodedInfo->reconFrameIndex,
		     encodedInfo->rdPtr, encodedInfo->wrPtr,
		     encodedInfo->bitstreamSize,
		     (srcEndFlag == 1 ? -1 : srcFrameIdx),
		     encodedInfo->encSrcIdx, encodedInfo->frameCycle);
#endif
	}
}

static void Coda9DisplayEncodedInformation(DecHandle handle, CodStd codec,
					   EncOutputInfo *encodedInfo)
{
	if (encodedInfo == NULL) {
		// Print header
		CVI_VC_INFO( "C    I    NO   T   RECON  RD_PTR   WR_PTR\n");
		CVI_VC_INFO( "-------------------------------------\n");
	} else {
		char **picTypeArray = (codec == STD_AVC ? EncPicTypeStringH264 : EncPicTypeStringMPEG4);
		char *strPicType;
		FrameBuffer *pRec = NULL;

		if (encodedInfo->picType > 2)
			strPicType = "?";
		else
			strPicType = picTypeArray[encodedInfo->picType];
		// Print informations
		CVI_VC_INFO( "AVC  %02d %5d %5s %5d    %08lx %08lx\n",
		     handle->instIndex, encodedInfo->encPicCnt, strPicType,
		     encodedInfo->reconFrameIndex, encodedInfo->rdPtr,
		     encodedInfo->wrPtr);

		pRec = &encodedInfo->reconFrame;
		CVI_VC_INFO("pRec, bufY = 0x%lX, bufCb = 0x%lX, bufCr = 0x%lX\n",
				pRec->bufY, pRec->bufCb, pRec->bufCr);
	}
}

/*lint -esym(438, ap) */
void DisplayEncodedInformation(EncHandle handle, CodStd codec,
			EncOutputInfo *encodedInfo, int srcEndFlag, int srcFrameIdx)
{
	switch (codec) {
	case STD_HEVC:
			Wave4DisplayEncodedInformation(handle, encodedInfo, srcEndFlag, srcFrameIdx);
		break;
	default:
		Coda9DisplayEncodedInformation(handle, codec, encodedInfo);
		break;
	}
}

void ReleaseVideoMemory(Uint32 coreIndex, vpu_buffer_t *memoryArr, Uint32 count)
{
	Uint32 index;

	for (index = 0; index < count; index++) {
		if (memoryArr[index].size)
			VDI_FREE_MEMORY(coreIndex, &memoryArr[index]);
	}
}

BOOL CalcYuvSize(Int32 format, Int32 picWidth, Int32 picHeight,
		 Int32 cbcrInterleave, size_t *lumaSize, size_t *chromaSize,
		 size_t *frameSize, Int32 *bitDepth, Int32 *packedFormat,
		 Int32 *yuv3p4b)
{
	Int32 temp_picWidth;
	Int32 chromaWidth;

	if (bitDepth != 0)
		*bitDepth = 0;
	if (packedFormat != 0)
		*packedFormat = 0;
	if (yuv3p4b != 0)
		*yuv3p4b = 0;

	CVI_VC_TRACE("format = %d\n", format);

	switch (format) {
	case FORMAT_420:
		if (lumaSize)
			*lumaSize = picWidth * picHeight;
		if (chromaSize)
			*chromaSize = (picWidth * picHeight) >> 1;
		if (frameSize)
			*frameSize = (picWidth * picHeight * 3) >> 1;
		break;
	case FORMAT_YUYV:
	case FORMAT_YVYU:
	case FORMAT_UYVY:
	case FORMAT_VYUY:
		if (packedFormat != 0)
			*packedFormat = 1;
		if (lumaSize)
			*lumaSize = picWidth * picHeight;
		if (chromaSize)
			*chromaSize = picWidth * picHeight;
		if (frameSize)
			*frameSize = *lumaSize + *chromaSize;
		break;
	case FORMAT_224:
		if (lumaSize)
			*lumaSize = picWidth * picHeight;
		if (chromaSize)
			*chromaSize = picWidth * picHeight;
		if (frameSize)
			*frameSize = picWidth * picHeight * 2;
		break;
	case FORMAT_422:
		if (lumaSize)
			*lumaSize = picWidth * picHeight;
		if (chromaSize)
			*chromaSize = picWidth * picHeight;
		if (frameSize)
			*frameSize = picWidth * picHeight * 2;
		break;
	case FORMAT_444:
		if (lumaSize)
			*lumaSize = picWidth * picHeight;
		if (chromaSize)
			*chromaSize = picWidth * picHeight * 2;
		if (frameSize)
			*frameSize = picWidth * picHeight * 3;
		break;
	case FORMAT_400:
		if (lumaSize)
			*lumaSize = picWidth * picHeight;
		if (chromaSize)
			*chromaSize = 0;
		if (frameSize)
			*frameSize = picWidth * picHeight;
		break;
	case FORMAT_422_P10_16BIT_MSB:
	case FORMAT_422_P10_16BIT_LSB:
		if (bitDepth != NULL) {
			*bitDepth = 10;
		}
		if (lumaSize)
			*lumaSize = picWidth * picHeight * 2;
		if (chromaSize)
			*chromaSize = *lumaSize;
		if (frameSize)
			*frameSize = *lumaSize + *chromaSize;
		break;
	case FORMAT_420_P10_16BIT_MSB:
	case FORMAT_420_P10_16BIT_LSB:
		if (bitDepth != 0)
			*bitDepth = 10;
		if (lumaSize)
			*lumaSize = picWidth * picHeight * 2;
		if (chromaSize)
			*chromaSize = picWidth * picHeight;
		if (frameSize)
			*frameSize = *lumaSize + *chromaSize;
		break;
	case FORMAT_YUYV_P10_16BIT_MSB: // 4:2:2 10bit packed
	case FORMAT_YUYV_P10_16BIT_LSB:
	case FORMAT_YVYU_P10_16BIT_MSB:
	case FORMAT_YVYU_P10_16BIT_LSB:
	case FORMAT_UYVY_P10_16BIT_MSB:
	case FORMAT_UYVY_P10_16BIT_LSB:
	case FORMAT_VYUY_P10_16BIT_MSB:
	case FORMAT_VYUY_P10_16BIT_LSB:
		if (bitDepth != 0)
			*bitDepth = 10;
		if (packedFormat != 0)
			*packedFormat = 1;
		if (lumaSize)
			*lumaSize = picWidth * picHeight * 2;
		if (chromaSize)
			*chromaSize = picWidth * picHeight * 2;
		if (frameSize)
			*frameSize = *lumaSize + *chromaSize;
		break;
	case FORMAT_420_P10_32BIT_MSB:
	case FORMAT_420_P10_32BIT_LSB:
		if (bitDepth != 0)
			*bitDepth = 10;
		if (yuv3p4b != 0)
			*yuv3p4b = 1;
		temp_picWidth = VPU_ALIGN32(picWidth);
		chromaWidth = ((VPU_ALIGN16((temp_picWidth >> 1) *
					    (1 << cbcrInterleave)) +
				2) /
			       3 * 4);
		if (cbcrInterleave == 1) {
			if (lumaSize)
				*lumaSize =
					(temp_picWidth + 2) / 3 * 4 * picHeight;
			if (chromaSize)
				*chromaSize = (chromaWidth * picHeight) >> 1;
		} else {
			if (lumaSize)
				*lumaSize =
					(temp_picWidth + 2) / 3 * 4 * picHeight;
			if (chromaSize)
				*chromaSize = ((chromaWidth * picHeight) >> 1) << 1;
		}
		if (frameSize)
			*frameSize = *lumaSize + *chromaSize;
		break;
	case FORMAT_YUYV_P10_32BIT_MSB:
	case FORMAT_YUYV_P10_32BIT_LSB:
	case FORMAT_YVYU_P10_32BIT_MSB:
	case FORMAT_YVYU_P10_32BIT_LSB:
	case FORMAT_UYVY_P10_32BIT_MSB:
	case FORMAT_UYVY_P10_32BIT_LSB:
	case FORMAT_VYUY_P10_32BIT_MSB:
	case FORMAT_VYUY_P10_32BIT_LSB:
		if (bitDepth != 0)
			*bitDepth = 10;
		if (packedFormat != 0)
			*packedFormat = 1;
		if (yuv3p4b != 0)
			*yuv3p4b = 1;
		if (frameSize)
			*frameSize = ((picWidth * 2) + 2) / 3 * 4 * picHeight;
		if (lumaSize)
			*lumaSize = *frameSize >> 1;
		if (chromaSize)
			*chromaSize = *frameSize >> 1;
		break;
	default:
		if (frameSize)
			*frameSize = (picWidth * picHeight * 3) >> 1;
		CVI_VC_ERR( "%s:%d Not supported format(%d)\n", __FILE__,
		     __LINE__, format);
		return FALSE;
	}
	return TRUE;
}

FrameBufferFormat GetPackedFormat(int srcBitDepth, PackedFormatNum packedType, int p10bits, int msb)
{
	int format = FORMAT_YUYV;

	// default pixel format = P10_16BIT_LSB (p10bits = 16, msb = 0)
	if (srcBitDepth == 8) {
		switch (packedType) {
		case PACKED_YUYV:
			format = FORMAT_YUYV;
			break;
		case PACKED_YVYU:
			format = FORMAT_YVYU;
			break;
		case PACKED_UYVY:
			format = FORMAT_UYVY;
			break;
		case PACKED_VYUY:
			format = FORMAT_VYUY;
			break;
		default:
			format = -1;
		}
	} else if (srcBitDepth == 10) {
		switch (packedType) {
		case PACKED_YUYV:
			if (p10bits == 16) {
				format = (msb == 0) ? FORMAT_YUYV_P10_16BIT_LSB : FORMAT_YUYV_P10_16BIT_MSB;
			} else if (p10bits == 32) {
				format = (msb == 0) ? FORMAT_YUYV_P10_32BIT_LSB : FORMAT_YUYV_P10_32BIT_MSB;
			} else {
				format = -1;
			}
			break;
		case PACKED_YVYU:
			if (p10bits == 16) {
				format = (msb == 0) ? FORMAT_YVYU_P10_16BIT_LSB : FORMAT_YVYU_P10_16BIT_MSB;
			} else if (p10bits == 32) {
				format = (msb == 0) ? FORMAT_YVYU_P10_32BIT_LSB : FORMAT_YVYU_P10_32BIT_MSB;
			} else {
				format = -1;
			}
			break;
		case PACKED_UYVY:
			if (p10bits == 16) {
				format = (msb == 0) ? FORMAT_UYVY_P10_16BIT_LSB : FORMAT_UYVY_P10_16BIT_MSB;
			} else if (p10bits == 32) {
				format = (msb == 0) ? FORMAT_UYVY_P10_32BIT_LSB : FORMAT_UYVY_P10_32BIT_MSB;
			} else {
				format = -1;
			}
			break;
		case PACKED_VYUY:
			if (p10bits == 16) {
				format = (msb == 0) ? FORMAT_VYUY_P10_16BIT_LSB : FORMAT_VYUY_P10_16BIT_MSB;
			} else if (p10bits == 32) {
				format = (msb == 0) ? FORMAT_VYUY_P10_32BIT_LSB : FORMAT_VYUY_P10_32BIT_MSB;
			} else {
				format = -1;
			}
			break;
		default:
			format = -1;
		}
	} else {
		format = -1;
	}

	return format;
}

void GenRegionToMap(VpuRect *region, /**< The size of the ROI region for H.265
					(start X/Y in CTU, end X/Y int CTU)  */
		    int *roiLevel, int num, Uint32 mapWidth, Uint32 mapHeight,
		    Uint8 *roiCtuMap)
{
	Int32 roi_id, blk_addr;
	Uint32 roi_map_size = mapWidth * mapHeight;

	// init roi map
	for (blk_addr = 0; blk_addr < (Int32)roi_map_size; blk_addr++)
		roiCtuMap[blk_addr] = 0;

	// set roi map. roi_entry[i+1] has higher priority than roi_entry[i]
	for (roi_id = 0; roi_id < (Int32)num; roi_id++) {
		Uint32 x, y;
		VpuRect *roi = region + roi_id;

		for (y = roi->top; y <= roi->bottom; y++) {
			for (x = roi->left; x <= roi->right; x++) {
				roiCtuMap[y * mapWidth + x] =
					*(roiLevel + roi_id);
			}
		}
	}
}

void GenRegionToQpMap(
		VpuRect *region, /**< The size of the ROI region for H.265 (start X/Y in  CTU, end X/Y int CTU)  */
		int *roiLevel, int num, int initQp, Uint32 mapWidth, Uint32 mapHeight,
		Uint8 *roiCtuMap)
{
	Int32 roi_id, blk_addr;
	Uint32 roi_map_size = mapWidth * mapHeight;

	// init roi map
	for (blk_addr = 0; blk_addr < (Int32)roi_map_size; blk_addr++)
		roiCtuMap[blk_addr] = initQp;

	// set roi map. roi_entry[i] has higher priority than roi_entry[i+1]
	for (roi_id = 0; roi_id < num; roi_id++) {
		Uint32 x, y;
		VpuRect *roi = region + roi_id;

		for (y = roi->top; y <= roi->bottom; y++) {
			for (x = roi->left; x <= roi->right; x++) {
				roiCtuMap[y * mapWidth + x] =
					*(roiLevel + roi_id);
			}
		}
	}
}

Int32 writeVuiRbsp(int coreIdx, TestEncConfig *pEncConfig, EncOpenParam *pEncOP,  vpu_buffer_t *vbVuiRbsp)
{
	CVI_VC_TRACE("\n");

	if (pEncOP->encodeVuiRbsp == TRUE) {
		vbVuiRbsp->size = VUI_HRD_RBSP_BUF_SIZE;

		CVI_VC_MEM("vbVuiRbsp->size = 0x%X\n", vbVuiRbsp->size);
		if (VDI_ALLOCATE_MEMORY(coreIdx, vbVuiRbsp, 0, NULL) < 0) {
			CVI_VC_ERR( "fail to allocate VUI rbsp buffer\n");
			return FALSE;
		}
		pEncOP->vuiRbspDataAddr = vbVuiRbsp->phys_addr;
	}
	return TRUE;
}

Int32 writeHrdRbsp(int coreIdx, TestEncConfig *pEncConfig, EncOpenParam *pEncOP, vpu_buffer_t *vbHrdRbsp)
{
	CVI_VC_TRACE("\n");

	if (pEncOP->encodeHrdRbspInVPS || pEncOP->encodeHrdRbspInVUI) {
		vbHrdRbsp->size = VUI_HRD_RBSP_BUF_SIZE;
		CVI_VC_MEM("vbHrdRbsp->size = 0x%X\n", vbHrdRbsp->size);
		if (VDI_ALLOCATE_MEMORY(coreIdx, vbHrdRbsp, 0, NULL) < 0) {
			CVI_VC_ERR( "fail to allocate HRD rbsp buffer\n");
			return FALSE;
		}

		pEncOP->hrdRbspDataAddr = vbHrdRbsp->phys_addr;
	}
	return TRUE;
}

#ifdef TEST_ENCODE_CUSTOM_HEADER
Int32 writeCustomHeader(int coreIdx, EncOpenParam *pEncOP, vpu_buffer_t *vbCustomHeader, hrd_t *hrd)
{
	Int32 rbspBitSize;
	Uint8 *pRbspBuf;
	Int32 rbspByteSize;
	vui_t vui;

	CVI_VC_TRACE("\n");

	pEncOP->encodeVuiRbsp = 1;

	vbCustomHeader->size = VUI_HRD_RBSP_BUF_SIZE;

	CVI_VC_MEM("vbCustomHeader->size = 0x%X\n", vbCustomHeader->size);
	if (VDI_ALLOCATE_MEMORY(coreIdx, vbCustomHeader, 0) < 0) {
		CVI_VC_ERR( "fail to allocate VUI rbsp buffer\n");
		return FALSE;
	}

	pEncOP->vuiRbspDataAddr = vbCustomHeader->phys_addr;

	pRbspBuf = (Uint8 *)malloc(VUI_HRD_RBSP_BUF_SIZE);
	if (pRbspBuf) {
		memset(pRbspBuf, 0, VUI_HRD_RBSP_BUF_SIZE);
		vui.vui_parameters_presesent_flag = 1;
		vui.vui_timing_info_present_flag = 1;
		vui.vui_num_units_in_tick = 1000;
		vui.vui_time_scale = 60 * 1000.0;
		vui.vui_hrd_parameters_present_flag = 1;
		vui.def_disp_win_left_offset = 1;
		vui.def_disp_win_right_offset = 1;
		vui.def_disp_win_top_offset = 1;
		vui.def_disp_win_bottom_offset = 1;

		// HRD param : refer xInitHrdParameters in HM
		{
			int useSubCpbParams = 0;
			int bitRate = pEncOP->bitRate;
			int isRandomAccess;
			int cpbSize = bitRate; // Adjusting value to be equal to TargetBitrate
			int bitRateScale;
			int cpbSizeScale;
			int i, j, numSubLayersM1;
			Uint32 bitrateValue, cpbSizeValue;
			Uint32 duCpbSizeValue;
			Uint32 duBitRateValue = 0;

			if (bitRate > 0) {
				hrd->nal_hrd_parameters_present_flag = 1;
				hrd->vcl_hrd_parameters_present_flag = 1;
			} else {
				hrd->nal_hrd_parameters_present_flag = 0;
				hrd->vcl_hrd_parameters_present_flag = 0;
			}

			if (pEncOP->EncStdParam.hevcParam.independSliceMode != 0)
				useSubCpbParams = 1;

			if (pEncOP->EncStdParam.hevcParam.intraPeriod > 0)
				isRandomAccess = 1;
			else
				isRandomAccess = 0;

			hrd->sub_pic_hrd_params_present_flag = useSubCpbParams;
			if (useSubCpbParams) {
				hrd->tick_divisor_minus2 = 100 - 2;
				hrd->du_cpb_removal_delay_increment_length_minus1 = 7;
				// 8-bit precision ( plus 1 for last DU in AU )
				hrd->sub_pic_cpb_params_in_pic_timing_sei_flag = 1;
				hrd->dpb_output_delay_du_length_minus1 = 5 + 7;
				// With sub-clock tick factor of
				// 100, at least 7 bits to have
				// the same value as AU dpb delay
			} else
				hrd->sub_pic_cpb_params_in_pic_timing_sei_flag =
					0;

			//  calculate scale value of bitrate and initial delay
			bitRateScale = calcScale(bitRate);
			if (bitRateScale <= 6)
				hrd->bit_rate_scale = 0;
			else
				hrd->bit_rate_scale = bitRateScale - 6;

			cpbSizeScale = calcScale(cpbSize);
			if (cpbSizeScale <= 4)
				hrd->cpb_size_scale = 0;
			else
				hrd->cpb_size_scale = cpbSizeScale - 4;

			hrd->cpb_size_du_scale = 6; // in units of 2^( 4 + 6 ) = 1,024 bit
			hrd->initial_cpb_removal_delay_length_minus1 = 15; // assuming 0.5 sec, log2( 90,000 * 0.5 ) = 16-bit

			if (isRandomAccess > 0) {
				hrd->au_cpb_removal_delay_length_minus1 = 5;
				hrd->dpb_output_delay_length_minus1 = 5;
			} else {
				hrd->au_cpb_removal_delay_length_minus1 = 9;
				hrd->dpb_output_delay_length_minus1 = 9;
			}

			numSubLayersM1 = 0;
			if (pEncOP->EncStdParam.hevcParam.gopPresetIdx ==
			    0) { // custom GOP
				for (i = 0; i < pEncOP->EncStdParam.hevcParam.gopParam.customGopSize;
				     i++) {
					if (numSubLayersM1 < pEncOP->EncStdParam.hevcParam.gopParam.picParam[i].temporalId)
						numSubLayersM1 =
							pEncOP->EncStdParam.hevcParam.gopParam.picParam[i].temporalId;
				}
			}
			hrd->vps_max_sub_layers_minus1 = numSubLayersM1;
			// sub_layer_hrd_parameters
			// BitRate[ i ] = ( bit_rate_value_minus1[ i ] + 1 ) *
			// 2^( 6 + bit_rate_scale )
			bitrateValue =
				bitRate /
				(1 << (6 + hrd->bit_rate_scale)); // bitRate is
			// in bits, so
			// it needs to
			// be scaled
			// down
			// CpbSize[ i ] = ( cpb_size_value_minus1[ i ] + 1 ) *
			// 2^( 4 + cpb_size_scale )
			cpbSizeValue = cpbSize /(1 << (4 + hrd->cpb_size_scale));
			 // using bitRate results in 1 second CPB size

			// DU CPB size could be smaller (i.e. bitrateValue /
			// number of DUs), but we don't know in how many DUs the
			// slice segment settings will result
			duCpbSizeValue = bitrateValue;
			duBitRateValue = cpbSizeValue;

			for (i = 0; i < (int)hrd->vps_max_sub_layers_minus1; i++) {
				hrd->fixed_pic_rate_general_flag[i] = 1;
				hrd->fixed_pic_rate_within_cvs_flag[i] = 1;
				// fixed_pic_rate_general_flag[ i ]
				// is equal to 1, the value of
				// fixed_pic_rate_within_cvs_flag[ i
				// ] is inferred to be equal to 1
				hrd->elemental_duration_in_tc_minus1[i] = 0;

				hrd->low_delay_hrd_flag[i] = 0;
				hrd->cpb_cnt_minus1[i] = 0;

				if (hrd->nal_hrd_parameters_present_flag) {
					for (j = 0; hrd->cpb_cnt_minus1[i];
					     j++) {
						hrd->bit_rate_value_minus1[j][i] =
							bitrateValue - 1;
						hrd->cpb_size_value_minus1[j][i] =
							cpbSize - 1;
						hrd->cpb_size_du_value_minus1[j]
									     [i] =
							duCpbSizeValue - 1;
						hrd->bit_rate_du_value_minus1[j]
									     [i] =
							duBitRateValue - 1;
						hrd->cbr_flag[j][i] = 0;
					}
				}
				if (hrd->vcl_hrd_parameters_present_flag) {
					for (j = 0; hrd->cpb_cnt_minus1[i];
					     j++) {
						hrd->bit_rate_value_minus1[j][i] =
							bitrateValue - 1;
						hrd->cpb_size_value_minus1[j][i] =
							cpbSize - 1;
						hrd->cpb_size_du_value_minus1[j]
									     [i] =
							duCpbSizeValue - 1;
						hrd->bit_rate_du_value_minus1[j]
									     [i] =
							duBitRateValue - 1;
						hrd->cbr_flag[j][i] = 0;
					}
				}
			}
		}

		EncodeVUI(hrd, &vui, pRbspBuf, VUI_HRD_RBSP_BUF_SIZE,
			  &rbspByteSize, &rbspBitSize, 60);
		pEncOP->vuiRbspDataSize = rbspBitSize;
		vdi_write_memory(coreIdx, pEncOP->vuiRbspDataAddr, pRbspBuf, rbspByteSize, pEncOP->streamEndian);
		free(pRbspBuf);
	}
	return TRUE;
}

Int32 allocateSeiNalDataBuf(int coreIdx, vpu_buffer_t *vbSeiNal, int srcFbNum)
{
	Int32 i;
	for (i = 0; i < srcFbNum; i++) {
	 // the number of roi buffer should be the same as source buffer num.
		vbSeiNal[i].size = SEI_NAL_DATA_BUF_SIZE;
		CVI_VC_MEM("vbSeiNal[%d].size = 0x%X\n", i, vbSeiNal[i].size);
		if (VDI_ALLOCATE_MEMORY(coreIdx, &vbSeiNal[i], 0) < 0) {
			CVI_VC_ERR( "fail to allocate SeiNalData buffer\n");
			return FALSE;
		}
	}
	return TRUE;
}

Int32 writeSeiNalData(EncHandle handle, int streamEndian,
		      PhysicalAddress prefixSeiNalAddr, hrd_t *hrd)
{
	sei_buffering_period_t buffering_period_sei;
	Uint8 *pSeiBuf;
	sei_active_parameter_t active_parameter_sei;
	Int32 seiByteSize = 0;
	HevcSEIDataEnc seiDataEnc;
	sei_pic_timing_t pic_timing_sei;

	pSeiBuf = (Uint8 *)malloc(SEI_NAL_DATA_BUF_SIZE);
	if (pSeiBuf) {
		memset(pSeiBuf, 0x00, SEI_NAL_DATA_BUF_SIZE);
		memset(&seiDataEnc, 0x00, sizeof(seiDataEnc));

		seiDataEnc.prefixSeiNalEnable = 1;
		seiDataEnc.prefixSeiDataEncOrder = 0;
		seiDataEnc.prefixSeiNalAddr = prefixSeiNalAddr;

		active_parameter_sei.active_video_parameter_set_id = 0;
		// vps_video_parameter_set_id of the VPS. wave420 is 0
		active_parameter_sei.self_contained_cvs_flag = 0;
		active_parameter_sei.no_parameter_set_update_flag = 0;
		active_parameter_sei.num_sps_ids_minus1 = 0;
		active_parameter_sei.active_seq_parameter_set_id[0] =
			0; // sps_seq_parameter_set_id of the SPS. wave420 is 0

		// put sei_pic_timing
		pic_timing_sei.pic_struct = 0;
		pic_timing_sei.source_scan_type = 1;
		pic_timing_sei.duplicate_flag = 0;

		buffering_period_sei.nal_initial_cpb_removal_delay[0] = 2229;
		buffering_period_sei.nal_initial_cpb_removal_offset[0] = 0;

		seiByteSize =
			EncodePrefixSEI(&active_parameter_sei, &pic_timing_sei,
					&buffering_period_sei, &hrd, pSeiBuf,
					SEI_NAL_DATA_BUF_SIZE);
		seiDataEnc.prefixSeiDataSize = seiByteSize;
		vdi_write_memory(handle->coreIdx, seiDataEnc.prefixSeiNalAddr,
				 pSeiBuf, seiDataEnc.prefixSeiDataSize,
				 streamEndian);

		free(pSeiBuf);
	}
	VPU_EncGiveCommand(handle, ENC_SET_SEI_NAL_DATA, &seiDataEnc);
	return TRUE;
}
#endif

void setRoiMapFromMap(int coreIdx, TestEncConfig *pEncConfig,
		      EncOpenParam *pEncOP, PhysicalAddress addrRoiMap,
		      Uint8 *roiMapBuf, EncParam *encParam, int maxCtuNum)
{
	int mapWidth, mapHeight, roi_map_size, grid_i;
	unsigned char motionByte, bitMask;
	BOOL isForeground;

	CVI_VC_TRACE("roi_enable = %d\n", pEncConfig->roi_enable);

	if (!(pEncOP->bgEnhanceEn == TRUE && pEncConfig->cviApiMode == API_MODE_SDK)) {
		return;
	}

	mapWidth = ((pEncOP->picWidth + 63) & ~63) >> 6;
	mapHeight = ((pEncOP->picHeight + 63) & ~63) >> 6;
	roi_map_size = mapWidth * mapHeight;
	encParam->ctuOptParam.mapEndian = VDI_LITTLE_ENDIAN;
	encParam->ctuOptParam.mapStride = ((pEncOP->picWidth + 63) & ~63) >> 6;
	encParam->ctuOptParam.addrRoiCtuMap = addrRoiMap;

	int byteNum = MIN(256, (roi_map_size + 7) >> 3);
	int bit_i = 0, byte_i = 0, foregroundCnt = 0;
	// depack motion map (1bit/grid), MSB first
	for (byte_i = 0; byte_i < byteNum; byte_i++) {
		motionByte = pEncOP->picMotionMap[byte_i];
		bitMask = 0x80;
		for (bit_i = 0; bit_i < 8; bit_i++){
			grid_i = (byte_i << 3) + bit_i;
			if (grid_i >= roi_map_size) {
				break;
			}
			isForeground = ((motionByte & bitMask) == 0) ? 0 : 1;
			foregroundCnt += 1;
			roiMapBuf[grid_i] = (pEncOP->bgDeltaQp < 0) ? isForeground :!isForeground;
			bitMask >>= 1;
		}
	}
	encParam->ctuOptParam.roiEnable = (foregroundCnt >= 0) ? TRUE : FALSE;
	encParam->ctuOptParam.roiDeltaQp = ABS(pEncOP->bgDeltaQp);
	// DEBUG
	CVI_VC_MOTMAP("###############################\n");
	int x, y;
	for (y = 0; y < mapHeight; y++) {
		for (x = 0; x < mapWidth; x++) {
			if (roiMapBuf[x + (y * mapWidth)] == 1) {
				CVI_VC_MOTMAP("1");
			} else {
				CVI_VC_MOTMAP(".");
			}
		}
		CVI_VC_MOTMAP("\n");
	}

	vdi_write_memory(coreIdx, encParam->ctuOptParam.addrRoiCtuMap,
			 roiMapBuf, maxCtuNum, encParam->ctuOptParam.mapEndian);
}

int setRoiMapFromBoundingBox(int coreIdx, TestEncConfig *pEncConfig,
			     EncOpenParam *pEncOP, PhysicalAddress addrRoiMap,
			     Uint8 *roiMapBuf, EncParam *encParam,
			     int maxCtuNum)
{
	CVI_VC_TRACE("roi_enable = %d\n", pEncConfig->roi_enable);

	if (pEncConfig->roi_enable && encParam->srcEndFlag != 1) {
		int roiNum = pEncConfig->actRegNum;
		encParam->ctuOptParam.addrRoiCtuMap = addrRoiMap;
		encParam->ctuOptParam.mapEndian = VDI_LITTLE_ENDIAN;
		encParam->ctuOptParam.mapStride = ((pEncOP->picWidth + 63) & ~63) >> 6;
		encParam->ctuOptParam.roiEnable = (roiNum != 0) ? pEncConfig->roi_enable : 0;
		encParam->ctuOptParam.roiDeltaQp = pEncConfig->roi_delta_qp;

		if (encParam->ctuOptParam.roiEnable) {
			GenRegionToMap(&pEncConfig->region[0],
				       &pEncConfig->roiLevel[0], roiNum,
				       encParam->ctuOptParam.mapStride,
				       ((pEncOP->picHeight + 63) & ~63) >> 6,
				       &roiMapBuf[0]);
			vdi_write_memory(coreIdx,
					 encParam->ctuOptParam.addrRoiCtuMap,
					 roiMapBuf, maxCtuNum,
					 encParam->ctuOptParam.mapEndian);
		}
	}
	return TRUE;
}

int checkParamRestriction(Uint32 productId, TestEncConfig *pEncConfig)
{
	CVI_VC_TRACE("IN, productId = 0x%X\n", productId);

	/* CHECK PARAMETERS */
	if (productId == PRODUCT_ID_420L) {
		if (pEncConfig->rotAngle != 0) {
			CVI_VC_ERR( "WAVE420L Not support rotation option\n");
			return FALSE;
		}
		if (pEncConfig->mirDir != 0) {
			CVI_VC_ERR( "WAVE420L Not support mirror option\n");
			return FALSE;
		}
		if (pEncConfig->srcFormat3p4b == TRUE) {
			CVI_VC_ERR(
			     "WAVE420L Not support 3 pixel 4byte format option\n");
			return FALSE;
		}

		if (pEncConfig->ringBufferEnable == TRUE) {
			pEncConfig->ringBufferEnable = FALSE;
			CVI_VC_ERR( "WAVE420L doesn't support ring-buffer.\n");
		}
	}
	return TRUE;
}

void cviSetWaveRoiBySdk(TestEncConfig *pEncConfig, EncParam *param, EncOpenParam *pEncOP, Int32 base_qp)
{
	Int32 roi_idx, valid_roi_cnt = 0;
	BOOL roi_param_change = param->roi_request;

	if (roi_param_change) {
		Int32 maxCtuWidth = (pEncOP->picWidth >> 6) - 1;
		Int32 maxCtuHeight = (pEncOP->picHeight >> 6) - 1;

		for (roi_idx = 0; roi_idx < 8; roi_idx++) {
			VpuRect *rect = &pEncConfig->region[roi_idx];

			if (param->roi_enable[roi_idx]) {
				rect->left = CLIP3(0, maxCtuWidth, param->roi_rect_x[roi_idx] >> 6);
				rect->top = CLIP3(0, maxCtuHeight, param->roi_rect_y[roi_idx] >> 6);
				rect->right = CLIP3(0, maxCtuWidth, (Int32)(rect->left + ((param->roi_rect_width[roi_idx]+63)>>6) - 1));
				rect->bottom = CLIP3(0, maxCtuHeight, (Int32)(rect->top + ((param->roi_rect_height[roi_idx]+63)>>6) - 1));
				valid_roi_cnt ++;
			} else {
				rect->left = rect->top = 0;
				rect->right = rect->bottom = 0;
			}
		}
		pEncConfig->actRegNum = 8;
		pEncConfig->roi_enable = (valid_roi_cnt > 0) ? 1 : 0;
		param->roi_request = FALSE;
	}

	if (roi_param_change || (param->roi_base_qp != base_qp)) {
		if (pEncConfig->roi_enable == 0) {
			return;
		}

		for (roi_idx = 0; roi_idx < 8; roi_idx++) {
			if (param->roi_enable[roi_idx]) {
				if (param->roi_qp_mode[roi_idx] == TRUE) {
					pEncConfig->roiLevel[roi_idx] =
						(base_qp > 0 && base_qp < 52) ? CLIP3(0, 7, base_qp - param->roi_qp[roi_idx]) : 0;
				} else {
					pEncConfig->roiLevel[roi_idx]  = CLIP3(0, 7, -param->roi_qp[roi_idx]);
				}
			} else {
				pEncConfig->roiLevel[roi_idx] = 0;
			}
		}
	}
	param->roi_base_qp = base_qp;
	pEncConfig->roi_delta_qp = 1;
}

void cviSetCodaRoiBySdk(EncParam *param, EncOpenParam *pEncOP, Int32 base_qp)
{
	Int32 roi_idx, valid_roi_cnt = 0;
	BOOL roi_param_change = param->roi_request;
	if (roi_param_change) {
		Int32 maxMbWidth = (pEncOP->picWidth >> 4) - 1;
		Int32 maxMbHeight = (pEncOP->picHeight >> 4) - 1;
		for (roi_idx = 0; roi_idx < 8; roi_idx++) {
			VpuRect *rect = &param->setROI.region[roi_idx];
			if (param->roi_enable[roi_idx]) {
				rect->left = CLIP3(0, maxMbWidth, param->roi_rect_x[roi_idx]>>4);
				rect->top = CLIP3(0, maxMbHeight, param->roi_rect_y[roi_idx]>>4);
				rect->right = CLIP3(0, maxMbWidth, (Int32)(rect->left + ((param->roi_rect_width[roi_idx] + 15)>>4) - 1));
				rect->bottom = CLIP3(0, maxMbHeight, (Int32)(rect->top + ((param->roi_rect_height[roi_idx] + 15)>>4) - 1));
			  valid_roi_cnt ++;
		  }
			else {
				rect->left = rect->top = -1;
				rect->right = rect->bottom = 0;
			}
		}
		param->setROI.number = 8;
		param->coda9RoiEnable = (valid_roi_cnt > 0) ? 1 : 0;
		param->roi_request = FALSE;
	}
	if (roi_param_change || (param->roi_base_qp != base_qp)) {
		if (param->coda9RoiEnable == 0) {
			return;
		}
		param->nonRoiQp = base_qp;
		param->coda9RoiPicAvgQp = base_qp;
		for (roi_idx = 0; roi_idx < 8; roi_idx++) {
			if (param->roi_enable[roi_idx]) {
				param->setROI.qp[roi_idx] = (param->roi_qp_mode[roi_idx]==TRUE) ?
							param->roi_qp[roi_idx] :
							base_qp + param->roi_qp[roi_idx];

			} else {
				param->setROI.qp[roi_idx] = 0;
			}
		}
	}
	param->roi_base_qp = base_qp;
}

int allocateRoiMapBuf(int coreIdx, TestEncConfig *pEncConfig,
		      vpu_buffer_t *vbRoi, int srcFbNum, int ctuNum)
{
	int i;
	if (pEncConfig->roi_enable || pEncConfig->cviApiMode != API_MODE_DRIVER) {
		// number of roi buffer should be the same as source buffer num.
		for (i = 0; i < srcFbNum; i++) {
			vbRoi[i].size = ctuNum;
			CVI_VC_MEM("vbRoi[%d].size = 0x%X\n", i, vbRoi[i].size);
			if (VDI_ALLOCATE_MEMORY(coreIdx, &vbRoi[i], 0, NULL) < 0) {
				CVI_VC_ERR("fail to allocate ROI buffer\n");
				return FALSE;
			}
		}
	}
	return TRUE;
}

int allocateCtuModeMapBuf(int coreIdx, TestEncConfig encConfig,
		vpu_buffer_t *vbCtuMode, int srcFbNum, int ctuNum)
{
	int i;

	if (encConfig.ctu_mode_enable) {
		// the number of CTU mode buffer should be the same as source buffer num.
		for (i = 0; i < srcFbNum ; i++) {
			vbCtuMode[i].size = ctuNum;

			CVI_VC_MEM("vbCtuMode[%d].size = 0x%X\n", i, vbCtuMode[i].size);

			if (VDI_ALLOCATE_MEMORY(coreIdx, &vbCtuMode[i], 0, NULL) < 0) {
				CVI_VC_ERR( "fail to allocate CTU mode buffer\n" );
				return FALSE;
			}
		}
	}
	return TRUE;
}

int cviSetCtuQpMap(int coreIdx, TestEncConfig *pEncConfig, EncOpenParam *pEncOP,
		   PhysicalAddress addrCtuQpMap, Uint8 *ctuQpMapBuf,
		   EncParam *encParam, int maxCtuNum)
{
	if (pEncConfig->ctu_qp_enable && encParam->srcEndFlag != 1) {
		int ctbHeight = ((pEncOP->picHeight + 63) & ~63) >> 6;
		int ctbStride = ((pEncOP->picWidth + 63) & ~63) >> 6;

		if (ctbHeight * ctbStride > maxCtuNum) {
			CVI_VC_ERR("ctbHeight = %d, ctbStride = %d, maxCtuNum = %d\n",
				ctbHeight, ctbStride, maxCtuNum);
			return FALSE;
		}

		encParam->ctuOptParam.addrCtuQpMap = addrCtuQpMap;
		encParam->ctuOptParam.mapEndian = VDI_LITTLE_ENDIAN;
		encParam->ctuOptParam.mapStride = ctbStride;
		encParam->ctuOptParam.ctuQpEnable = pEncConfig->ctu_qp_enable;

		CVI_VC_CFG(
			"bQpMapValid = %d, mapStride = %d, ctuQpMapBuf = %p\n",
			pEncConfig->cviEc.bQpMapValid,
			encParam->ctuOptParam.mapStride, ctuQpMapBuf);
		CVI_VC_TRACE("addrCtuQpMap = 0x"VCODEC_UINT64_FORMAT_HEX"\n", addrCtuQpMap);

		if (pEncConfig->cviEc.bQpMapValid) {
			vdi_write_memory(coreIdx,
					 encParam->ctuOptParam.addrCtuQpMap,
					 ctuQpMapBuf, ctbHeight * ctbStride,
					 encParam->ctuOptParam.mapEndian);
		}

		if (dbg_mask & CVI_MASK_TRACE) {
			int row, col;
			CVI_VC_TRACE(
				"ctbStride = %d, ctbHeight = %d, QpMap =%p\n",
				ctbStride, ctbHeight, ctuQpMapBuf);

			CVI_VC_TRACE("ctuQpMapBuf =\n");
			for (row = 0; row < ctbHeight; row++) {
				for (col = 0; col < ctbStride; col++) {
					printf("%3d ", ctuQpMapBuf[ctbStride * row + col]);
				}
				printf("\n");
			}
		}
	}
	return TRUE;
}

int allocateCtuQpMapBuf(int coreIdx, TestEncConfig *pEncConfig,
			vpu_buffer_t *vbCtuQp, int srcFbNum, int ctuNum)
{
	int i;

	if (pEncConfig->ctu_qp_enable) {
		// the number of CTU mode buffer should be the same as source
		// buffer num.

		for (i = 0; i < srcFbNum; i++) {
			vbCtuQp[i].size = ctuNum;
			CVI_VC_MEM("vbCtuQp[%d].size = 0x%X\n", i, vbCtuQp[i].size);
			if (VDI_ALLOCATE_MEMORY(coreIdx, &vbCtuQp[i], 0, NULL) < 0) {
				CVI_VC_ERR("fail to allocate CTU QP buffer\n");
				return FALSE;
			}
		}
	}
	return TRUE;
}
