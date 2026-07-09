#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "app_ipcam_comm.h"
#include "app_ipcam_venc.h"

#ifdef AUDIO_SUPPORT
#include "app_ipcam_audio.h"
#endif

#include "cvi_mbuf.h"
#include "zfifo.h"

static ZFIFO *s_media_fifo[CVI_MBUF_MAX_NUM] = {0};

static CVI_MBUF_HANDLE s_hVideoWriterId[CVI_MBUF_MAX_NUM];

static int app_ipcam_Mbuf_Check(int fifo_id)
{
    if (fifo_id < 0 || fifo_id >= CVI_MBUF_MAX_NUM)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param error. %d\n", fifo_id);
        return -1;
    }

    return 0;
}

static int app_ipcam_Mbuf_Create(int fifo_id, int size)
{
    if ((0 != app_ipcam_Mbuf_Check(fifo_id)) || (size <= 0))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param error. %d, %d\n", fifo_id, size);
        return -1;
    }

    if (s_media_fifo[fifo_id] != NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Mbuf %d has been initialized.\n", fifo_id);
        return 0;
    }

    char name[10] = {0};
    sprintf(name, "stream%d", fifo_id);
    s_media_fifo[fifo_id] = zfifo_init(name, size);
    if (s_media_fifo[fifo_id] == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "zfifo_init %s failed.\n", name);
        return -1;
    }

    return 0;
}

static int app_ipcam_Mbuf_Destroy(int fifo_id)
{
    if (0 != app_ipcam_Mbuf_Check(fifo_id))
    {
        return -1;
    }

    if (s_media_fifo[fifo_id] == NULL)
    {
        return 0;
    }

    zfifo_uninit(s_media_fifo[fifo_id]);
    s_media_fifo[fifo_id] = NULL;

    return 0;
}

static CVI_MBUF_HANDLE * app_ipcam_Mbuf_AddWriter(int fifo_id)
{
    if (0 != app_ipcam_Mbuf_Check(fifo_id))
    {
        return NULL;
    }
    ZFIFO *zfifo = s_media_fifo[fifo_id];
    ZFIFO_DESC *id = zfifo_open(zfifo);
    if (id == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "zfifo_open stream%d failed.\n", fifo_id);
        return NULL;
    }

    return (CVI_MBUF_HANDLE *)id;
}

static void app_ipcam_Mbuf_DelWriter(CVI_MBUF_HANDLE * writerid)
{
    ZFIFO_DESC *zdesc = (ZFIFO_DESC *)writerid;
    if (writerid == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param error.\n");
        return;
    }

    zfifo_close(zdesc);
}

static ZFIFO_DESC * app_ipcam_Mbuf_AddReader(int fifo_id, int bLastTime)
{
    if (0 != app_ipcam_Mbuf_Check(fifo_id))
    {
        return NULL;
    }

    ZFIFO *zfifo = s_media_fifo[fifo_id];
    ZFIFO_DESC *id = zfifo_open(zfifo);
    if (id == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "zfifo_open stream%d failed.\n", fifo_id);
        return NULL;
    }
    if(bLastTime)
    {
        zfifo_set_newest_frame(id);
    }
    else
    {
        zfifo_set_oldest_frame(id);
    }

    return (ZFIFO_DESC *)id;
}

static void app_ipcam_Mbuf_DelReader(ZFIFO_DESC * readerid)
{
    ZFIFO_DESC *zdesc = (ZFIFO_DESC *)readerid;
    if (readerid == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param error.\n");
        return;
    }
    zfifo_close(zdesc);
}

static int app_ipcam_Mbuf_WriteFrame(ZFIFO_DESC * writerid, CVI_MEDIA_FRAME_INFO_T *pFrameInfo)
{
    int iRet = -1;

    if (pFrameInfo == NULL || writerid == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param error\n");
        return -1;
    }
    CVI_MBUF_HEADER_T stMFrameHeader;
    ZFIFO_NODE node[CVI_MBUF_MAX];
    node[CVI_MBUF_HEADER].base = &stMFrameHeader;
    node[CVI_MBUF_HEADER].len = sizeof(stMFrameHeader);

    memset(&stMFrameHeader, 0, sizeof(stMFrameHeader));
    stMFrameHeader.magicStart = MEDIA_MAGIC_START;
    stMFrameHeader.magicEnd = MEDIA_MAGIC_END;
    memcpy(&stMFrameHeader.frameParam, &pFrameInfo->frameParam, sizeof(stMFrameHeader.frameParam));

    node[CVI_MBUF_DATA].base = pFrameInfo->frameBuf;
    node[CVI_MBUF_DATA].len = pFrameInfo->frameParam.frameLen;

    iRet = zfifo_writev_plus((ZFIFO_DESC *)writerid, node, CVI_MBUF_MAX, (pFrameInfo->frameParam.frameType == CVI_MEDIA_VFRAME_I));
    if (iRet <= 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "zfifo_writev error. %d\n", iRet);
    }

    return iRet;
}

int app_ipcam_Mbuf_ReadFrame(CVI_MBUF_HANDLE readerid, int bKeyFrame, CVI_MEDIA_FRAME_INFO_T *pFrameInfo, int iTimeouts)
{
    int iRet = -1;
    if (readerid == NULL || pFrameInfo == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param error.\n");
        return -1;
    }
    CVI_MBUF_HEADER_T stMFrameHeader;
    ZFIFO_DESC *zfifo_desc = (ZFIFO_DESC *)readerid;
    ZFIFO_NODE node[CVI_MBUF_MAX];
    node[CVI_MBUF_HEADER].base = &stMFrameHeader;
    node[CVI_MBUF_HEADER].len = sizeof(stMFrameHeader);

    if ((pFrameInfo->frameBuf == NULL) || (pFrameInfo->frameBufLen <= 0))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param error. buf:%p, len:%d. \n"
            , pFrameInfo->frameBuf, pFrameInfo->frameBufLen);
        return -1;
    }
    else
    {
        node[CVI_MBUF_DATA].base = pFrameInfo->frameBuf;
        node[CVI_MBUF_DATA].len = pFrameInfo->frameBufLen;
    }
    memset(&stMFrameHeader, 0, sizeof(stMFrameHeader));

    if (bKeyFrame)
    {
        iRet = zfifo_readv_flag_plus(zfifo_desc, node, CVI_MBUF_MAX, iTimeouts);
    }
    else
    {
        iRet = zfifo_readv_plus(zfifo_desc, node, CVI_MBUF_MAX, iTimeouts);
    }
    if (iRet < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "zfifo_readv error. %d\n", iRet);
        return iRet;
    }
    else if (iRet == 0)
    {
        return iRet;
    }
    else
    {
        if ((stMFrameHeader.magicStart != MEDIA_MAGIC_START)
            || (stMFrameHeader.magicEnd != MEDIA_MAGIC_END))
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "zfifo read magic error.%x, %x\n", stMFrameHeader.magicStart, stMFrameHeader.magicEnd);
            return -1;
        }

        memcpy(&pFrameInfo->frameParam, &stMFrameHeader.frameParam, sizeof(pFrameInfo->frameParam));
    }

    return iRet;
}

int app_ipcam_Mbuf_Video_WriteFrame(int mbufId, CVI_MEDIA_FRAME_INFO_T *pFrameInfo)
{
    int iRet = -1;
    if (s_hVideoWriterId[mbufId] && (0 == app_ipcam_Mbuf_Check(mbufId)))
    {
        iRet = app_ipcam_Mbuf_WriteFrame((ZFIFO_DESC *)s_hVideoWriterId[mbufId], pFrameInfo);
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Invalid Write Id:%d\n", mbufId);
    }
    return iRet;
}

int app_ipcam_Mbuf_Audio_WriteFrame(int mbufId, CVI_MEDIA_FRAME_INFO_T *pFrameInfo)
{
    int iRet = -1;
    if (s_hVideoWriterId[mbufId] && (0 == app_ipcam_Mbuf_Check(mbufId)))
    {
        iRet = app_ipcam_Mbuf_WriteFrame((ZFIFO_DESC *)s_hVideoWriterId[mbufId], pFrameInfo);
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Invalid Write Id:%d\n", mbufId);
    }
    return iRet;
}

CVI_MBUF_HANDLE* app_ipcam_Mbuf_CreateReader(int mbufId, int bLastTime)
{
    if (0 != app_ipcam_Mbuf_Check(mbufId))
    {
        return NULL;
    }
    return (CVI_MBUF_HANDLE*)app_ipcam_Mbuf_AddReader(mbufId, bLastTime);
}

int app_ipcam_Mbuf_DestoryReader(CVI_MBUF_HANDLE* readerid)
{
    app_ipcam_Mbuf_DelReader((ZFIFO_DESC *)readerid);
    return 0;
}

int app_ipcam_Mbuf_Init()
{
    int iRet = -1;
    int iMbufSize = 0;

    APP_PARAM_VENC_CTX_S *pstVencCtx = app_ipcam_Venc_Param_Get();
#ifdef AUDIO_SUPPORT
    APP_PARAM_AUDIO_CFG_T *pstAudioCtx = app_ipcam_Audio_Param_Get();
#endif

    for (int i = 0; i < pstVencCtx->s32VencChnCnt; i++)
    {
        if (pstVencCtx->astVencChnCfg[i].bEnable && (pstVencCtx->astVencChnCfg[i].enType != PT_JPEG))
        {
            if (0 != app_ipcam_Mbuf_Check(pstVencCtx->astVencChnCfg[i].VencChn))
            {
                return iRet;
            }

#ifdef AUDIO_SUPPORT
            iMbufSize = ((((pstVencCtx->astVencChnCfg[i].u32BitRate / 8) << 10) +  (pstAudioCtx->astAudioCfg.enSamplerate * 2)) * CVI_CAMERA_PRE_RECORD_TIMES);
#else
            iMbufSize = (((pstVencCtx->astVencChnCfg[i].u32BitRate / 8) << 10) * CVI_CAMERA_PRE_RECORD_TIMES);
#endif
            APP_PROF_LOG_PRINT(LEVEL_INFO, "mbufId:%d iMbufSize %d.\n", pstVencCtx->astVencChnCfg[i].VencChn, iMbufSize);
            iRet = app_ipcam_Mbuf_Create(pstVencCtx->astVencChnCfg[i].VencChn, iMbufSize);
            if (iRet)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Create vmbuf %d failed.\n", pstVencCtx->astVencChnCfg[i].VencChn);
                iRet = -1;
                goto endFunc;
            }

            if (s_hVideoWriterId[pstVencCtx->astVencChnCfg[i].VencChn] == NULL)
            {
                s_hVideoWriterId[pstVencCtx->astVencChnCfg[i].VencChn] = app_ipcam_Mbuf_AddWriter(pstVencCtx->astVencChnCfg[i].VencChn);
                if (s_hVideoWriterId[pstVencCtx->astVencChnCfg[i].VencChn] == NULL)
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "Create vmbuf write %d failed.\n", pstVencCtx->astVencChnCfg[i].VencChn);
                    iRet = -1;
                    goto endFunc;
                }
            }
            else
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "s_hVideoWriterId %d has been initialized.\n", pstVencCtx->astVencChnCfg[i].VencChn);
            }
        }
    }

    iRet = 0;
endFunc:
    if (iRet)
    {
        app_ipcam_Mbuf_UnInit();
    }

    return iRet;
}


int app_ipcam_Mbuf_UnInit()
{
    APP_PARAM_VENC_CTX_S *pstVencCtx = app_ipcam_Venc_Param_Get();
    for (int i = 0; i < pstVencCtx->s32VencChnCnt; i++)
    {
        if (s_hVideoWriterId[pstVencCtx->astVencChnCfg[i].VencChn])
        {
            app_ipcam_Mbuf_DelWriter(s_hVideoWriterId[pstVencCtx->astVencChnCfg[i].VencChn]);
            app_ipcam_Mbuf_Destroy(pstVencCtx->astVencChnCfg[i].VencChn);
            s_hVideoWriterId[pstVencCtx->astVencChnCfg[i].VencChn] = NULL;
        }
    }
    return 0;
}


