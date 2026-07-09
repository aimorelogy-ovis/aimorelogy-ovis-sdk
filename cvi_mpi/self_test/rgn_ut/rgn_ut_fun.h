#ifndef __RGN_UT_FUN_H__
#define __RGN_UT_FUN_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "cvi_type.h"
#include <cvi_comm_vb.h>
#include <cvi_comm_vpss.h>
#include <cvi_comm_region.h>

#include "ut_comm.h"

typedef enum _PIC_SIZE_E {
	PIC_CIF,
	PIC_D1_PAL, /* 720 * 576 */
	PIC_D1_NTSC, /* 720 * 480 */
	PIC_720P, /* 1280 * 720  */
	PIC_1600x1200,
	PIC_1080P, /* 1920 * 1080 */
	PIC_1088, /* 1920 * 1088 */
	PIC_1440P, /* 2560 * 1440 */
	PIC_2304x1296,
	PIC_2048x1536,
	PIC_2048x2048,
	PIC_2560x1600,
	PIC_2592x1520,
	PIC_2592x1536,
	PIC_2592x1944,
	PIC_2688x1520,
	PIC_2716x1524,
	PIC_2880x1620,
	PIC_3844x1124,
	PIC_3840x2160,
	PIC_4096x2160,
	PIC_3000x3000,
	PIC_4000x3000,
	PIC_4032x3000,
	PIC_3840x8640,
	PIC_4608x4320,
	PIC_5120x3840,
	PIC_7688x1124,
	PIC_7680x4320,
	PIC_8192x4320,
	PIC_640x480,
	PIC_479P, /* 632 * 479 */
	PIC_400x400,
	PIC_288P, /* 384 * 288 */
	PIC_CUSTOMIZE,
	PIC_BUTT
} PIC_SIZE_E;

extern RGN_RGBQUARD_S overlay_palette[256];

/* RGN_COMM_SYS_Exit:
 *   Exit the system and release resources.
 */
CVI_VOID RGN_COMM_SYS_Exit(void);

/* RGN_COMM_SYS_Init:
 *   Initialize the system with VB configuration.
 *
 * [in] pstVbConfig: Pointer to VB configuration structure.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_SYS_Init(VB_CONFIG_S *pstVbConfig);

/* RGN_COMM_VPSS_Init:
 *   Initialize the VPSS group and channels.
 *
 * [in] VpssGrp: VPSS group ID.
 * [in] pabChnEnable: Array indicating enabled channels.
 * [in] pstVpssGrpAttr: Pointer to VPSS group attributes.
 * [in] pastVpssChnAttr: Pointer to VPSS channel attributes.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_VPSS_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
				  VPSS_CHN_ATTR_S *pastVpssChnAttr);

/* RGN_COMM_VPSS_Start:
 *   Start the VPSS group and channels.
 *
 * [in] VpssGrp: VPSS group ID.
 * [in] pabChnEnable: Array indicating enabled channels.
 * [in] pstVpssGrpAttr: Pointer to VPSS group attributes.
 * [in] pastVpssChnAttr: Pointer to VPSS channel attributes.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_VPSS_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
				  VPSS_CHN_ATTR_S *pastVpssChnAttr);

/* RGN_COMM_REGION_Create:
 *   Create a region with specified attributes.
 *
 * [in] HandleNum: Number of handles.
 * [in] enType: Region type.
 * [in] pixelFormat: Pixel format.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_REGION_Create(CVI_S32 HandleNum, RGN_TYPE_E enType, PIXEL_FORMAT_E pixelFormat);

/* RGN_COMM_REGION_AttachToChn:
 *   Attach a region to a channel.
 *
 * [in] HandleNum: Handle number.
 * [in] enType: Region type.
 * [in] pstChn: Pointer to channel structure.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_REGION_AttachToChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn);

/* RGN_COMM_REGION_GetMinHandle:
 *   Get the minimum handle for a region type.
 *
 * [in] enType: Region type.
 * return: Minimum handle number.
 */
CVI_S32 RGN_COMM_REGION_GetMinHandle(RGN_TYPE_E enType);

/* RGN_COMM_VPSS_SendFrame:
 *   Send a frame to the VPSS.
 *
 * [in] VpssGrp: VPSS group ID.
 * [in] stSize: Pointer to size structure.
 * [in] enPixelFormat: Pixel format.
 * [in] filename: File containing the frame.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_VPSS_SendFrame(VPSS_GRP VpssGrp, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename);

/* RGN_COMM_FRAME_CompareWithFile:
 *   Compare a frame with a file.
 *
 * [in] filename: File to compare.
 * [in] pstVideoFrame: Pointer to video frame info.
 * return: CVI_TRUE if they match, CVI_FALSE otherwise.
 */
CVI_BOOL RGN_COMM_FRAME_CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

/* RGN_COMM_FRAME_SaveToFile:
 *   Save a frame to a file.
 *
 * [in] filename: File to save the frame.
 * [in] pstVideoFrame: Pointer to video frame info.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_FRAME_SaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

/* RGN_COMM_REGION_DetachFrmChn:
 *   Detach a region from a channel.
 *
 * [in] HandleNum: Handle number.
 * [in] enType: Region type.
 * [in] pstChn: Pointer to channel structure.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_REGION_DetachFrmChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn);

/* RGN_COMM_REGION_Destroy:
 *   Destroy a region.
 *
 * [in] HandleNum: Handle number.
 * [in] enType: Region type.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_REGION_Destroy(CVI_S32 HandleNum, RGN_TYPE_E enType);

/* RGN_COMM_VPSS_Stop:
 *   Stop the VPSS group and channels.
 *
 * [in] VpssGrp: VPSS group ID.
 * [in] pabChnEnable: Array indicating enabled channels.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_VPSS_Stop(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable);

/* RGN_COMM_REGION_MST_LoadBmp:
 *   Load a bitmap for a region.
 *
 * [in] filename: File containing the bitmap.
 * [out] pstBitmap: Pointer to bitmap structure.
 * [in] bFil: Whether to filter the bitmap.
 * [in] u16FilColor: Filter color.
 * [in] enPixelFormat: Pixel format.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_REGION_MST_LoadBmp(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
			CVI_U32 u16FilColor, PIXEL_FORMAT_E enPixelFormat);


/*
 * RGN_COMM_REGION_MST_UpdateCanvas:
 *   Update the canvas with a bitmap for a region.
 *
 * [in] filename: File containing the bitmap.
 * [out] pstBitmap: Pointer to the bitmap structure.
 * [in] bFil: Whether to filter the bitmap.
 * [in] u16FilColor: Filter color.
 * [in] pstSize: Pointer to the size structure of the canvas.
 * [in] u32Stride: Stride of the canvas.
 * [in] enPixelFormat: Pixel format of the canvas.
 * return: CVI_SUCCESS if the operation is successful.
 */
CVI_S32 RGN_COMM_REGION_MST_UpdateCanvas(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
			CVI_U32 u16FilColor, SIZE_S *pstSize, CVI_U32 u32Stride, PIXEL_FORMAT_E enPixelFormat);

/* RGN_COMM_ODEC_REGION_Create:
 *    Create ODEC region.
 *
 * [in] u32FileSize: File size.
 * [in] stSize: Region size.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_ODEC_REGION_Create(CVI_U32 u32FileSize, SIZE_S *stSize);

/* RGN_COMM_ODEC_REGION_Destroy:
 *   Destroy ODEC region.
 *
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_ODEC_REGION_Destroy(void);

/* RGN_COMM_ODEC_REGION_AttachToChn:
 *   ODEC region apply to chn.
 *
 * [in] pstChn: Module chn.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_ODEC_REGION_AttachToChn(MMF_CHN_S *pstChn);

/* RGN_COMM_ODEC_REGION_DetachFrmChn:
 *   Cancel ODEC region on chn.
 *
 * [in] pstChn: Module chn.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_ODEC_REGION_DetachFrmChn(MMF_CHN_S *pstChn);

/* RGN_COMM_CompareWithMD5:
 *   Compare MD5.
 *
 * [in] md5sum: MD5 checksum.
 * [in] pstVideoFrame: Video frame info.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 RGN_COMM_CompareWithMD5(const CVI_CHAR *md5sum, VIDEO_FRAME_INFO_S *pstVideoFrame);

/*
 * RGN_COMM_FileSendToVpss:
 *   Send a file to the VPSS (Video Processing Subsystem).
 *
 * [in] VpssGrp: VPSS group ID.
 * [in] stSize: Pointer to the size structure.
 * [in] enPixelFormat: Pixel format of the file.
 * [in] filename: Path to the file to be sent.
 * return: CVI_SUCCESS if the file is successfully sent.
 */
CVI_S32 RGN_COMM_FileSendToVpss(VPSS_GRP VpssGrp, const SIZE_S *stSize,
	PIXEL_FORMAT_E enPixelFormat, const CVI_CHAR *filename);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __RGN_UT_FUN_H__ */
