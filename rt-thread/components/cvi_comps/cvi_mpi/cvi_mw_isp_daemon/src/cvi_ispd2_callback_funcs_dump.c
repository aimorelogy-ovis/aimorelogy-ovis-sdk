/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: cvi_ispd2_event_server.c
 * Description:
 */

#include "cvi_ispd2_callback_funcs_dump.h"

#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "3A_internal.h"
#include "cvi_awb.h"
#include "cvi_ispd2_callback_funcs_apps.h"

// ------------------------------Dump-----------------------------------------------
/* json version: "jsonrpc":"2.0" */
#define DUMP_LIST_SIZE 2

static CVI_CHAR *achDumpList[DUMP_LIST_SIZE] = {
    "{\"method\":\"CVI_GetMultipleRAWFrame\",\"params\":{\"dump_reg\":false,\"frames\":1,"
    "\"tightly\":false}}",
    "{\"method\":\"CVI_GetMultipleYUVFrame\",\"params\":{\"frames\":1,\"from\":0,"
    "\"getRawReplayYuv\":false,\"getRawReplayYuvId\":0,\"tightly\":false}}"};

static JSONTokener      *pJsonTok;
static JSONObject       *pJsonObj;
static TISPDeviceInfo    stDeviceInfo;
static TJSONRpcBinaryOut tBinaryOut;
static CVI_CHAR          achFileName[MAX_LOG_FILENAME_LENGTH];
static char              achSaveDirName[MAX_AWB_LOG_PATH_LENGTH];
static TISPDaemon2Info  *g_ptObject;

static CVI_S32 mkdirs(CVI_CHAR *muldir, mode_t mode)
{
    CVI_S32   i, len, ret = CVI_SUCCESS;
    CVI_CHAR *str = muldir;

    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (str[i] == '/') {
            str[i] = '\0';
            if (access(str, F_OK) != 0) {
                if (mkdir(str, mode) != 0) {
                    ret = CVI_FAILURE;
                    goto ERROR_HANDLE;
                }
            }
            str[i] = '/';
        }
    }

    if (len > 0 && access(str, F_OK) != 0) {
        if (mkdir(str, mode) != 0) {
            ret = CVI_FAILURE;
        }
    }

ERROR_HANDLE:
    return ret;
}

static inline CVI_U16 deco_dpcm(CVI_U16 xp, CVI_BOOL s, CVI_U16 v)
{
    return s ? MAX(xp - v, 0) : MIN(xp + v, 4095);
}

static inline CVI_U16 decp_pcm(CVI_U16 xp, CVI_U16 ad, CVI_U16 v)
{
    return (v > xp) ? v + ad : v + ad + 1;
}

static CVI_U16 decode_12_10_12(CVI_BOOL nop, CVI_U16 xenco, CVI_U16 xpred)
{
    CVI_U16 xdeco;

    if (nop) {
        xdeco = (xenco << 2) + 2;
    } else if ((xenco & 0x300) == 0x000) {
        xdeco = deco_dpcm(xpred, xenco & 0x80, xenco & 0x7f);
    } else if ((xenco & 0x380) == 0x100) {
        xdeco = deco_dpcm(xpred, xenco & 0x40, ((xenco & 0x3f) << 1) + 128);
    } else if ((xenco & 0x380) == 0x180) {
        xdeco = deco_dpcm(xpred, xenco & 0x40, ((xenco & 0x3f) << 2) + 257);
    } else {
        xdeco = decp_pcm(xpred, 3, (xenco & 0x1ff) << 3);
    }
    return xdeco;
}

static CVI_U16 decode_12_8_12(CVI_BOOL nop, CVI_U16 xenco, CVI_U16 xpred)
{
    CVI_U16 xdeco;

    if (nop) {
        xdeco = (xenco << 4) + 8;
    } else if ((xenco & 0xf0) == 0x00) {
        xdeco = deco_dpcm(xpred, xenco & 0x8, xenco & 0x7);
    } else if ((xenco & 0xe0) == 0x60) {
        xdeco = deco_dpcm(xpred, xenco & 0x10, ((xenco & 0xf) << 1) + 8);
    } else if ((xenco & 0xe0) == 0x40) {
        xdeco = deco_dpcm(xpred, xenco & 0x10, ((xenco & 0xf) << 2) + 41);
    } else if ((xenco & 0xe0) == 0x20) {
        xdeco = deco_dpcm(xpred, xenco & 0x10, ((xenco & 0xf) << 3) + 107);
    } else if ((xenco & 0xf0) == 0x10) {
        xdeco = deco_dpcm(xpred, xenco & 0x8, ((xenco & 0x7) << 4) + 239);
    } else {
        xdeco = decp_pcm(xpred, 15, (xenco & 0x7f) << 5);
    }

    return xdeco;
}

static CVI_U16 decode_12_7_12(CVI_BOOL nop, CVI_U16 xenco, CVI_U16 xpred)
{
    CVI_U16 xdeco;

    if (nop) {
        xdeco = (xenco << 5) + 16;
    } else if ((xenco & 0x78) == 0x00) {
        xdeco = deco_dpcm(xpred, xenco & 0x4, xenco & 0x3);
    } else if ((xenco & 0x78) == 0x08) {
        xdeco = deco_dpcm(xpred, xenco & 0x4, ((xenco & 0x3) << 1) + 4);
    } else if ((xenco & 0x78) == 0x10) {
        xdeco = deco_dpcm(xpred, xenco & 0x4, ((xenco & 0x3) << 2) + 13);
    } else if ((xenco & 0x70) == 0x20) {
        xdeco = deco_dpcm(xpred, xenco & 0x8, ((xenco & 0x7) << 3) + 31);
    } else if ((xenco & 0x70) == 0x30) {
        xdeco = deco_dpcm(xpred, xenco & 0x8, ((xenco & 0x7) << 4) + 99);
    } else if ((xenco & 0x78) == 0x18) {
        xdeco = deco_dpcm(xpred, xenco & 0x4, ((xenco & 0x3) << 5) + 235);
    } else {
        xdeco = decp_pcm(xpred, 31, (xenco & 0x3f) << 6);
    }

    return xdeco;
}

static CVI_U16 decode_12_6_12(CVI_BOOL nop, CVI_U16 xenco, CVI_U16 xpred)
{
    CVI_U16 xdeco;

    if (nop) {
        xdeco = (xenco << 6) + 32;
    } else if ((xenco & 0x3c) == 0x00) {
        xdeco = deco_dpcm(xpred, xenco & 0x2, xenco & 0x1);
    } else if ((xenco & 0x3c) == 0x04) {
        xdeco = deco_dpcm(xpred, xenco & 0x2, ((xenco & 0x1) << 2) + 3);
    } else if ((xenco & 0x38) == 0x10) {
        xdeco = deco_dpcm(xpred, xenco & 0x4, ((xenco & 0x3) << 3) + 13);
    } else if ((xenco & 0x3c) == 0x08) {
        xdeco = deco_dpcm(xpred, xenco & 0x2, ((xenco & 0x1) << 4) + 49);
    } else if ((xenco & 0x38) == 0x18) {
        xdeco = deco_dpcm(xpred, xenco & 0x4, ((xenco & 0x3) << 5) + 89);
    } else if ((xenco & 0x3c) == 0x0c) {
        xdeco = deco_dpcm(xpred, xenco & 0x2, ((xenco & 0x1) << 6) + 233);
    } else {
        xdeco = decp_pcm(xpred, 63, (xenco & 0x1f) << 7);
    }

    return xdeco;
}

static CVI_U16 predictor1(CVI_U16 x, CVI_U16 *pb, CVI_BOOL *nop)
{
    *nop = x <= 1;
    return (*nop) ? 0 : pb[1];
}

static CVI_U16 predictor2(CVI_U16 x, CVI_U16 *pb, CVI_BOOL *nop)
{
    CVI_U16 xpred = 0;
    *nop          = x == 0;

    if (x == 1) {
        xpred = pb[0];  // use previous decoded different color value
    } else if (x == 2) {
        xpred = pb[1];  // use previous coded same color value
    } else if (x == 3) {
        xpred = ((pb[0] <= pb[1] && pb[1] <= pb[2]) || (pb[0] >= pb[1] && pb[1] >= pb[2])) ? pb[0]
                                                                                           : pb[1];
    } else if (x >= 4) {
        xpred = ((pb[0] <= pb[1] && pb[1] <= pb[2]) || (pb[0] >= pb[1] && pb[1] >= pb[2])) ? pb[0]
                : ((pb[0] <= pb[2] && pb[1] <= pb[3]) || (pb[0] >= pb[2] && pb[1] >= pb[3]))
                    ? pb[1]
                    : ((pb[1] + pb[3] + 1) >> 1);
    }

    return (*nop) ? 0 : xpred;
}

static CVI_VOID dpcm_rx(const CVI_U16 *idata, CVI_U16 *odata, CVI_S32 width, CVI_S32 height,
                        CVI_S32 mode)
{
    CVI_U16  xpred, xdeco, pix_buf[4];
    CVI_BOOL nop;
    CVI_S32  reg_img_height                              = height;
    CVI_S32  reg_img_width                               = width;
    CVI_U16 (*predictor)(CVI_U16, CVI_U16 *, CVI_BOOL *) = NULL;
    CVI_U16 (*decoder)(CVI_BOOL, CVI_U16, CVI_U16)       = NULL;

    switch (mode) {
    case 4:
        decoder   = decode_12_10_12;
        predictor = predictor1;
        break;
    case 5:
        decoder   = decode_12_8_12;
        predictor = predictor1;
        break;
    case 6:
        decoder   = decode_12_7_12;
        predictor = predictor2;
        break;
    case 7:
        decoder   = decode_12_6_12;
        predictor = predictor2;
        break;
    }

    for (CVI_S32 y = 0; y < reg_img_height; y++) {
        for (CVI_S32 x = 0; x < reg_img_width; x++) {
            CVI_S32 idx = y * reg_img_width + x;

            if (mode == 0) {
                // bypass
                odata[idx] = idata[idx];
            } else {
                xpred      = predictor(x, pix_buf, &nop);
                xdeco      = decoder(nop, idata[idx], xpred);
                odata[idx] = xdeco;

                // buffer shift
                pix_buf[3] = pix_buf[2];
                pix_buf[2] = pix_buf[1];
                pix_buf[1] = pix_buf[0];
                pix_buf[0] = xdeco;
            }
        }
    }
}

static CVI_S32 CVI_ISPD2_Dump_CropUnpackRaw(CVI_CHAR *src, TRAWHeader *pRawHeader, CVI_CHAR *dest)
{
#define CV182X_LINE_BUFFER_WIDTH 2304
#define CV182X_TILE_START_INDEX  1536
#define CV183X_LINE_BUFFER_WIDTH 2688
#define CV183X_TILE_START_INDEX  1920

    CVI_CHAR *src_6b = NULL, *src_6b_init = NULL, *det_6b = NULL, *det_6b_init = NULL,
             *src_12b = NULL;
    CVI_U16  temp[4]  = {0};
    CVI_S32  h, w, m, n, i, width;
    CVI_U8   v0, v1, v2;
    CVI_S32  src_stride = pRawHeader->size / pRawHeader->height;
    CVI_CHAR c = 0, zero = 0;
    CVI_S32  ret = CVI_SUCCESS;

    if (pRawHeader->compress == 0) {
        // Unpack and Crop RAW
        for (h = pRawHeader->cropY; h < pRawHeader->cropHeight + pRawHeader->cropY; h++) {
            for (w = pRawHeader->cropX, m = pRawHeader->cropX / 2 * 3, i = h * src_stride;
                 w < pRawHeader->cropWidth + pRawHeader->cropX; w += 2, m += 3) {
                v0      = (CVI_U8)src[i + m];
                v1      = (CVI_U8)src[i + m + 1];
                v2      = (CVI_U8)src[i + m + 2];
                temp[0] = (CVI_U16)((v0 << 4) | ((v2 >> 0) & 0x0f));
                temp[1] = (CVI_U16)((v1 << 4) | ((v2 >> 4) & 0x0f));

                c     = temp[0] & 0xFF;
                *dest = c;
                ++dest;
                c     = (temp[0] >> 8) & 0xFF;
                *dest = c;
                ++dest;
                c     = temp[1] & 0xFF;
                *dest = c;
                ++dest;
                c     = (temp[1] >> 8) & 0xFF;
                *dest = c;
                ++dest;
            }
        }
    } else if (pRawHeader->compress == 1) {
        CVI_S32 line_buffer_width = 0xfffffff;
        CVI_S32 title_start_idx   = 0xfffffff;

#ifdef CHIP_ARCH_CV182X
        line_buffer_width = CV182X_LINE_BUFFER_WIDTH;
        title_start_idx   = CV182X_TILE_START_INDEX;
#elif CHIP_ARCH_CV183X
        line_buffer_width = CV183X_LINE_BUFFER_WIDTH;
        title_start_idx   = CV183X_TILE_START_INDEX;
#endif

        // Unpack RAW
        if (pRawHeader->width > line_buffer_width) {
            width = pRawHeader->width + 8;
        } else {
            width = pRawHeader->width;
        }

        /*prepare mem space*/
        src_6b      = (CVI_CHAR *)calloc(1, pRawHeader->height * width * 2);
        src_6b_init = src_6b;
        if (src_6b == NULL) {
            printf("alloc unpack raw src_6b space fail!\n");
            ret = CVI_FAILURE;
            goto ERROR_HANDLE;
        }
        det_6b      = (CVI_CHAR *)calloc(1, pRawHeader->height * width * 2);
        det_6b_init = det_6b;
        if (det_6b == NULL) {
            printf("alloc unpack raw det_6b space fail!\n");
            ret = CVI_FAILURE;
            goto ERROR_HANDLE;
        }
        /*******************/

        for (h = 0; h < pRawHeader->height; h++) {
            for (w = 0, m = 0, i = h * src_stride; w < width; w += 4, m += 3) {
                v0      = (CVI_U8)src[i + m];
                v1      = (CVI_U8)src[i + m + 1];
                v2      = (CVI_U8)src[i + m + 2];
                temp[0] = (CVI_U16)(((v0 & 0x03) << 4) | (v2 & 0x0F));
                temp[1] = (CVI_U16)((v0 & 0xFC) >> 2);
                temp[2] = (CVI_U16)(((v1 & 0x03) << 4) | ((v2 & 0xF0) >> 4));
                temp[3] = (CVI_U16)((v1 & 0xFC) >> 2);

                c       = temp[0] & 0xFF;
                *src_6b = c;
                ++src_6b;
                *src_6b = zero;
                ++src_6b;
                c       = temp[1] & 0xFF;
                *src_6b = c;
                ++src_6b;
                *src_6b = zero;
                ++src_6b;
                c       = temp[2] & 0xFF;
                *src_6b = c;
                ++src_6b;
                *src_6b = zero;
                ++src_6b;
                c       = temp[3] & 0xFF;
                *src_6b = c;
                ++src_6b;
                *src_6b = zero;
                ++src_6b;
            }
        }

        if (width > line_buffer_width) {
            for (CVI_S32 x = 0; x < pRawHeader->height; x++) {
                for (CVI_S32 y = 0, k = 0; y < pRawHeader->width * 2; y++, k++) {
                    if (y == title_start_idx * 2) {
                        k += (8 * 2);
                    }
                    *det_6b = src_6b_init[x * width * 2 + k];
                    ++det_6b;
                }
            }
        } else {
            memcpy(det_6b_init, src_6b_init, pRawHeader->height * width * 2);
        }

        if (src_6b_init) {
            SAFE_FREE(src_6b_init);
        }
        src_12b = (CVI_CHAR *)calloc(1, pRawHeader->height * width * 2);
        if (src_12b == NULL) {
            printf("alloc unpack raw src_12b space fail!\n");
            ret = CVI_FAILURE;
            goto ERROR_HANDLE;
        }
        dpcm_rx((CVI_U16 *)det_6b, (CVI_U16 *)src_12b, pRawHeader->width, pRawHeader->height, 7);
        if (det_6b_init) {
            SAFE_FREE(det_6b_init);
        }

        // Crop Raw
        CVI_U16 *pSrc_12b, *pCrop_image;

        pSrc_12b    = (CVI_U16 *)src_12b;
        pCrop_image = (CVI_U16 *)dest;
        for (h = 0, n = pRawHeader->cropY; h < pRawHeader->cropHeight; h++, n++) {
            for (w = 0, m = pRawHeader->cropX; w < pRawHeader->cropWidth; w++, m++) {
                pCrop_image[w + h * pRawHeader->cropWidth] = pSrc_12b[m + n * pRawHeader->width];
            }
        }
    } else {
        ret = CVI_FAILURE;
        goto ERROR_HANDLE;
    }

ERROR_HANDLE:
    if (src_6b_init) {
        SAFE_FREE(src_6b_init);
    }
    if (det_6b_init) {
        SAFE_FREE(det_6b_init);
    }
    if (src_12b) {
        SAFE_FREE(src_12b);
    }

    return ret;
}

// get file name
static void CVI_ISPD2_Dump_GetTimeStamp(CVI_CHAR *pStr, CVI_U8 u8Len)
{
    struct tm      tm;
    struct timeval time;

    gettimeofday(&time, NULL);
    localtime_r(&time.tv_sec, &tm);

    snprintf(pStr, u8Len, "%04d%02d%02d%02d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);
}

static CVI_S32 CVI_ISPD2_Dump_WriteInfoTofile(CVI_CHAR *chPathName, const CVI_CHAR *chContent,
                                              const CVI_U32 u32Len)
{
    FILE *fp;

    fp = fopen(chPathName, "w+");
    if (fp == CVI_NULL) {
        printf("file:%s open fail!\n", chPathName);
        return CVI_FAILURE;
    }

    if (fwrite(chContent, 1, u32Len, fp) != u32Len) {
        return CVI_FAILURE;
    }
    fclose(fp);

    return CVI_SUCCESS;
}

static CVI_S32 CVI_ISPD2_Dump_GetFileName(const TFrameData *pstFrameData, CVI_CHAR *pFilePath)
{
    TRAWHeader *pstRAWHeader      = pstFrameData->pstRAWHeader;
    const char *pchBayerFormat[4] = {"BGGR", "GBRG", "GRBG", "RGGB"};
    char       *env               = getenv(ENV_DUMP_ALL_FILE_PATH);
    CVI_CHAR    achTimeStr[35];

    if (pstRAWHeader == NULL) {
        memset(pFilePath, 0, sizeof(MAX_LOG_FILENAME_LENGTH));
        return CVI_FAILURE;
    }

    CVI_ISPD2_Dump_GetTimeStamp(achTimeStr, sizeof(achTimeStr));
    if (!env) {
        snprintf(achSaveDirName, MAX_AWB_LOG_PATH_LENGTH, "./dump_awb/dump_%s", achTimeStr);
    } else {
        snprintf(achSaveDirName, MAX_AWB_LOG_PATH_LENGTH, "%s/dump_awb/dump_%s", env, achTimeStr);
    }

    if (mkdirs(achSaveDirName, 0775) != CVI_SUCCESS) {
        printf("Directory(%s) isn't exist and create failure!\n", achSaveDirName);
        return CVI_FAILURE;
    }

    if (pstRAWHeader->fusionFrame > 1) {
        snprintf((char *)pFilePath, MAX_LOG_FILENAME_LENGTH,
                 "%s/%uX%u_%s_WDR_-color=%d_-bits=12_-frame=1_-hdr=%d_ISO=%d_%s", achSaveDirName,
                 pstRAWHeader->cropWidth * 2, pstRAWHeader->cropHeight,
                 pchBayerFormat[pstRAWHeader->bayerID], pstRAWHeader->bayerID,
                 pstRAWHeader->fusionFrame, pstRAWHeader->iso, achTimeStr);
    } else {
        snprintf((char *)pFilePath, MAX_LOG_FILENAME_LENGTH,
                 "%s/%uX%u_%s_Linear_-color=%d_-bits=12_-frame=1_-hdr=%d_ISO=%d_%s", achSaveDirName,
                 pstRAWHeader->cropWidth, pstRAWHeader->cropHeight,
                 pchBayerFormat[pstRAWHeader->bayerID], pstRAWHeader->bayerID,
                 pstRAWHeader->fusionFrame, pstRAWHeader->iso, achTimeStr);
    }

    return CVI_SUCCESS;
}

static CVI_S32 CVI_ISPD2_Dump_saveLogToFile(TFrameData *ptFrameData, CVI_CHAR *pFilePath)
{
    JSONObject     *pstJsonObj = ISPD2_json_object_new_string((char *)ptFrameData->pu8AWBLogBuffer);
    const CVI_CHAR *pJsonObjContent  = ISPD2_json_object_get_string(pstJsonObj);
    CVI_U32         u32ContentLength = ISPD2_json_object_get_string_len(pstJsonObj);
    CVI_CHAR        chPathName[FILE_NAME_LENGTH];

    snprintf((char *)chPathName, FILE_NAME_LENGTH, "%s-awblog.txt", (char *)pFilePath);
    if (CVI_ISPD2_Dump_WriteInfoTofile(chPathName, pJsonObjContent, u32ContentLength) !=
        CVI_SUCCESS) {
        printf("write log file fail!\n");
    }

    ISPD2_json_object_put(pstJsonObj);

    return CVI_SUCCESS;
}

static CVI_S32 CVI_ISPD2_Dump_SaveBinToFile(TISPDeviceInfo *ptDevInfo, CVI_CHAR *pFilePath)
{
    CVI_U32   u32ContentLength = ptDevInfo->tAWBBinData.u32Size;
    CVI_CHAR  chPathName[FILE_NAME_LENGTH];
    CVI_CHAR *pchBuffer = (CVI_CHAR *)ptDevInfo->tAWBBinData.pu8Buffer;
    CVI_S32   ret       = CVI_SUCCESS;

    snprintf((char *)chPathName, FILE_NAME_LENGTH, "%s-awb.wbin", (char *)pFilePath);
    if (CVI_ISPD2_Dump_WriteInfoTofile(chPathName, pchBuffer, u32ContentLength) != CVI_SUCCESS) {
        printf("write awb bin file fail!\n");
        ret = CVI_FAILURE;
    }

    return ret;
}

static CVI_S32 CVI_ISPD2_Dump_saveRawInfoToFile(TFrameData *ptFrameData, CVI_CHAR *pFilePath)
{
    CVI_CHAR        chPathName[FILE_NAME_LENGTH];
    JSONObject     *pstJsonObj      = ISPD2_json_object_new_string((char *)ptFrameData->pu8RawInfo);
    const CVI_CHAR *pJsonObjContent = ISPD2_json_object_get_string(pstJsonObj);
    CVI_U32         u32ContentLength = ISPD2_json_object_get_string_len(pstJsonObj);
    CVI_S32         ret              = CVI_SUCCESS;

    snprintf((char *)chPathName, FILE_NAME_LENGTH, "%s.txt", (char *)pFilePath);
    if (CVI_ISPD2_Dump_WriteInfoTofile(chPathName, pJsonObjContent, u32ContentLength) !=
        CVI_SUCCESS) {
        printf("write Raw info file fail!\n");
        ret = CVI_FAILURE;
    }

    ISPD2_json_object_put(pstJsonObj);

    return ret;
}

static CVI_S32 CVI_ISPD2_Dump_SaveRawToFile(TFrameData *ptFrameData, CVI_CHAR *pFilePath)
{
    CVI_CHAR  chPathName[FILE_NAME_LENGTH];
    CVI_CHAR *pchUnpackRaw;
    CVI_U32   u32UnpackRawSize =
        ptFrameData->pstRAWHeader->cropHeight * ptFrameData->pstRAWHeader->cropWidth * 2;

    snprintf((char *)chPathName, FILE_NAME_LENGTH, "%s.raw", (char *)pFilePath);
    if ((ptFrameData->pstRAWHeader->compress == 0) || (ptFrameData->pstRAWHeader->compress == 1)) {
        pchUnpackRaw = (CVI_CHAR *)calloc(1, u32UnpackRawSize);
        if (pchUnpackRaw == NULL) {
            printf("alloc unpack raw space fail!\n");
            return CVI_FAILURE;
        }
    } else {
        printf("inexistent raw compress mode!\n");
        return CVI_FAILURE;
    }

    if (CVI_ISPD2_Dump_CropUnpackRaw((CVI_CHAR *)ptFrameData->stFrameInfo.pau8FrameAddr[0],
                                     ptFrameData->pstRAWHeader, pchUnpackRaw) != CVI_SUCCESS) {
        printf("crop unpack raw file fail!\n");
        return CVI_FAILURE;
    }

    if (CVI_ISPD2_Dump_WriteInfoTofile(chPathName, pchUnpackRaw, u32UnpackRawSize) != CVI_SUCCESS) {
        printf("write raw file fail!\n");
    }

    SAFE_FREE(pchUnpackRaw);

    return CVI_SUCCESS;
}

static CVI_S32 CVI_ISPD2_Dump_GenerateYUVFile(TFrameData *ptFrameData, CVI_CHAR *pFilePath)
{
    TYUVHeader *ptYUVHeader   = ptFrameData->pstYUVHeader;
    CVI_CHAR   *pchYUVBufAddr = (CVI_CHAR *)ptFrameData->stFrameInfo.pau8FrameAddr[0];
    CVI_CHAR    chPathName[FILE_NAME_LENGTH];
    CVI_CHAR   *achImgFormat[2]      = {"yuv420p", "nv21"};
    CVI_CHAR    achReadMeContent[50] = "YUV Image Format: unknown";

    if (pFilePath[0] == 0 && pFilePath[1] == 0 && pFilePath[2] == 0) {
        return CVI_FAILURE;
    }

    /*yuv file*/
    snprintf((char *)chPathName, FILE_NAME_LENGTH, "%s.yuv", (char *)pFilePath);
    if (CVI_ISPD2_Dump_WriteInfoTofile(chPathName, pchYUVBufAddr, ptYUVHeader->size) !=
        CVI_SUCCESS) {
        printf("write yuv file fail!\n");
    }

    /*readme file*/
    snprintf(chPathName, FILE_NAME_LENGTH, "%s/readme.txt", achSaveDirName);
    if ((ptYUVHeader->pixelFormat == 13) || (ptYUVHeader->pixelFormat == 19)) {
        snprintf(achReadMeContent, sizeof(achReadMeContent), "YUV Image Format: %s",
                 ptYUVHeader->pixelFormat == 13 ? achImgFormat[0] : achImgFormat[1]);
    }

    JSONObject     *pstJsonObj       = ISPD2_json_object_new_string((char *)achReadMeContent);
    const CVI_CHAR *pJsonObjContent  = ISPD2_json_object_get_string(pstJsonObj);
    CVI_U32         u32ContentLength = ISPD2_json_object_get_string_len(pstJsonObj);

    if (CVI_ISPD2_Dump_WriteInfoTofile(chPathName, pJsonObjContent, u32ContentLength) !=
        CVI_SUCCESS) {
        printf("readme file create fail!\n");
    }

    ISPD2_json_object_put(pstJsonObj);

    return CVI_SUCCESS;
}

static CVI_S32 CVI_ISPD2_Dump_GenerateRawRelatedFile(TJSONRpcContentIn *ptContentIn)
{
    TISPDeviceInfo *ptDevInfo   = ptContentIn->ptDeviceInfo;
    TFrameData     *ptFrameData = &(ptDevInfo->tFrameData);
    CVI_S32         ret         = CVI_SUCCESS;

    /*get dump file name**/
    if (CVI_ISPD2_Dump_GetFileName(ptFrameData, achFileName) != CVI_SUCCESS) {
        ret = CVI_FAILURE;
        goto ERROR_HANDLE;
    }

    /***********************save awb log file************************/
    if (CVI_ISPD2_Dump_saveLogToFile(ptFrameData, achFileName) != CVI_SUCCESS) {
        goto ERROR_HANDLE;
    }

    /***********************save awb.bin file************************/
    if (CVI_ISPD2_Dump_SaveBinToFile(ptDevInfo, achFileName) != CVI_SUCCESS) {
        goto ERROR_HANDLE;
    }

    /***********************save RAW info file***********************/
    if (CVI_ISPD2_Dump_saveRawInfoToFile(ptFrameData, achFileName) != CVI_SUCCESS) {
        goto ERROR_HANDLE;
    }

    /**********get raw data and write raw data to file***********/
    if (CVI_ISPD2_Dump_SaveRawToFile(ptFrameData, achFileName) != CVI_SUCCESS) {
        goto ERROR_HANDLE;
    }

ERROR_HANDLE:
    CVI_ISPD2_ReleaseBinaryData(&(ptDevInfo->tAEBinData));
    CVI_ISPD2_ReleaseBinaryData(&(ptDevInfo->tAWBBinData));

    return ret;
}

static CVI_S32 CVI_ISPD2_Dump_HandleJsonObject(JSONObject              *pJsonRoot,
                                               const TISPD2HandlerInfo *ptHandlerInfo,
                                               TISPDeviceInfo          *ptDeviceInfo)
{
    TISPD2Handler *ptHandlerAddr     = NULL;
    JSONObject    *pJsonMethodPtr    = NULL;
    JSONObject    *pJsonDataResponse = NULL;

    TJSONRpcContentIn  tContentIn;
    TJSONRpcContentOut tContentOut;
    CVI_BOOL           bMethodMatch;
    int                iRes;
    CVI_S32            ret = CVI_SUCCESS;

    iRes = ISPD2_json_pointer_get(pJsonRoot, "/method", &pJsonMethodPtr);
    if (iRes != 0) {
        printf("Parse Error (Method)!\n");
        return CVI_FAILURE;
    }

    const char *pszMethod    = ISPD2_json_object_get_string(pJsonMethodPtr);
    CVI_U32     u32MethodLen = (CVI_U32)strlen(pszMethod);

    // Content In
    tContentIn.pvData       = NULL;
    tContentIn.ptDeviceInfo = ptDeviceInfo;
    tContentIn.pParams      = NULL;

    ISPD2_json_pointer_get(pJsonRoot, "/params", &(tContentIn.pParams));

    // Content Out
    tContentOut.s32StatusCode                = JSONRPC_CODE_OK;
    tContentOut.ptBinaryOut                  = &tBinaryOut;
    tContentOut.ptBinaryOut->pvData          = NULL;
    tContentOut.ptBinaryOut->bDataValid      = CVI_FALSE;
    tContentOut.ptBinaryOut->eBinaryDataType = 0;
    // tContentOut.ptBinaryOut->u32DataSize = 0;

    pJsonDataResponse = ISPD2_json_object_new_object();

    bMethodMatch  = CVI_FALSE;
    ptHandlerAddr = ptHandlerInfo->ptHandler;
    for (CVI_U32 u32Idx = 0; u32Idx < ptHandlerInfo->u32Count; ++u32Idx, ptHandlerAddr++) {
        if (u32MethodLen != ptHandlerAddr->u32NameLen) continue;

        if (strcmp(pszMethod, ptHandlerAddr->szName) == 0) {
            ptHandlerAddr->cbFunction(&tContentIn, &tContentOut, pJsonDataResponse);
            bMethodMatch = CVI_TRUE;
            break;
        }
    }

    if (strcmp(pszMethod, "CVI_GetMultipleRAWFrame") == 0) {
        if (CVI_ISPD2_Dump_GenerateRawRelatedFile(&tContentIn) != CVI_SUCCESS) {
            ret = CVI_FAILURE;
            goto ERROR_HANDLE;
        } else {
            printf("Generate dump raw file successfully!\n");
        }
    } else if (strcmp(pszMethod, "CVI_GetMultipleYUVFrame") == 0) {
        CVI_ISPD2_Dump_GenerateYUVFile(&(ptDeviceInfo->tFrameData), achFileName);
    }

ERROR_HANDLE:
    if (!bMethodMatch) {
        printf("Method Not Found!\n");
    }

    if (tContentOut.s32StatusCode != JSONRPC_CODE_OK) {
        printf("(Error) content output:%s [status code:%d]!\n", tContentOut.szErrorMessage,
               tContentOut.s32StatusCode);
    }

    if (pJsonDataResponse) {
        ISPD2_json_object_put(pJsonDataResponse);
    }

    // release frame data
    CVI_ISPD2_ReleaseFrameData(ptDeviceInfo);

    return ret;
}

static CVI_S32 CVI_ISPD2_Dump_GetData(const TISPDaemon2Info *ptObject, const char *pBufIn,
                                      CVI_U32 u32BufInSize)
{
    CVI_U32 u32ReadOffset, u32ParseOffset;

    pJsonTok      = ISPD2_json_tokener_new();
    u32ReadOffset = 0;
    do {
        pJsonObj = ISPD2_json_tokener_parse_ex(pJsonTok, pBufIn + u32ReadOffset, -1);
        if (pJsonTok->err != ISPD2_json_tokener_success) {
            printf("JSON parsing error: %s (offset : %d)",
                   ISPD2_json_tokener_error_desc(pJsonTok->err), pJsonTok->char_offset);
            break;
        }

        CVI_ISPD2_Dump_HandleJsonObject(pJsonObj, &(ptObject->tHandlerInfo), &stDeviceInfo);

        u32ParseOffset = (CVI_U32)ISPD2_json_tokener_get_parse_end(pJsonTok);
        u32ReadOffset += u32ParseOffset;
    } while (u32ReadOffset < u32BufInSize);

    ISPD2_json_object_put(pJsonObj);
    ISPD2_json_tokener_free(pJsonTok);

    return CVI_SUCCESS;
}

static void *CVI_ISPD2_Dump_Thread(void *data)
{
    CVI_S32 s32ViPipe = *(CVI_S32 *)data;

    while (g_ptObject->bDumpThreadRunning) {
        usleep(500 * 1000);  // 0.5 second
        if (CVI_ISP_GetAwbRunStatus(s32ViPipe)) {
            CVI_ISP_SetAwbRunStatus(s32ViPipe, CVI_FALSE);
            for (CVI_U32 u32Idx = 0; u32Idx < DUMP_LIST_SIZE; ++u32Idx) {
                if (achDumpList[u32Idx]) {
                    CVI_ISPD2_Dump_GetData(g_ptObject, achDumpList[u32Idx],
                                           strlen(achDumpList[u32Idx]));
                }
            }
        }
    }

    return 0;
}

CVI_S32 CVI_ISPD2_Dump_Init(TISPDaemon2Info *ptObject)
{
    g_ptObject = (TISPDaemon2Info *)ptObject;

    return CVI_SUCCESS;
}

TISPDaemon2Info *CVI_ISPD2_Dump_GetDaemon2Info(void)
{
    return g_ptObject;
}

CVI_S32 CVI_ISPD2_Dump_Start(VI_PIPE ViPipe)
{
    /*only to get dump all file when detect awb abnormal file!*/
    pthread_create(&(g_ptObject->thDumpThreadId), NULL, CVI_ISPD2_Dump_Thread, (void *)&ViPipe);
    // pthread_setname_np(g_ptObject->thDumpThreadId, "ISPD2Dump");
    return CVI_SUCCESS;
}
