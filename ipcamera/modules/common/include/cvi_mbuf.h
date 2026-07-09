#ifndef __CVI_MBUF_H__
#define __CVI_MBUF_H__

#include "zfifo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CVI_MBUF_HEADER = 0,
    CVI_MBUF_DATA = 1,
    CVI_MBUF_MAX,
} CVI_MBUF_TYPE_E;

typedef enum {
    CVI_MBUF_VSTREAM_MAIN = 0,
    CVI_MBUF_VSTREAM_SUB = 1,
    CVI_MBUF_VSTREAM_BUTT,
} CVI_MBUF_VSTREAM_E;

#define CVI_CAMERA_PRE_RECORD_TIMES     (2)         // pre record times(second)
#define CVI_MBUF_MAX_NUM                (16)//((CVI_CAMERA_MAX_NUMS * CVI_MBUF_VSTREAM_BUTT) + 1)
#define CVI_MBUF_STREAM_MAX_SIZE        (1024 * 1024)


#define MEDIA_MAGIC_START (0x5a5a5a5a)
#define MEDIA_MAGIC_END (0xa5a5a5a5)

typedef void* CVI_MBUF_HANDLE;

typedef enum
{
    CVI_MEDIA_VFRAME_P = 0,
    CVI_MEDIA_VFRAME_I = 1,

    CVI_MEDIA_AFRAME_A = 10,
} CVI_MEDIA_FRAME_TYPE_E;

typedef struct
{
    int frameCodec;
    CVI_MEDIA_FRAME_TYPE_E frameType;
    unsigned int frameIndex;
    unsigned int frameKeyIndex;
    unsigned long long framePts;
    long frameTime;
    unsigned int frameLen;
} CVI_MEDIA_FRAME_PARAM_T;

typedef struct
{
    CVI_MEDIA_FRAME_PARAM_T frameParam;
    unsigned char *frameBuf;
    unsigned int frameBufLen;
} CVI_MEDIA_FRAME_INFO_T;


/*! Media buffer frame header */
typedef struct {
    unsigned int magicStart;         /* MEDIA_MAGIC_START */
    CVI_MEDIA_FRAME_PARAM_T frameParam;
    unsigned int magicEnd;         /* MEDIA_MAGIC_END */
} CVI_MBUF_HEADER_T;

int app_ipcam_Mbuf_ReadFrame(CVI_MBUF_HANDLE readerid, int bKeyFrame, CVI_MEDIA_FRAME_INFO_T *pFrameInfo, int iTimeouts);

int app_ipcam_Mbuf_Video_WriteFrame(int mbufId, CVI_MEDIA_FRAME_INFO_T *pFrameInfo);

int app_ipcam_Mbuf_Audio_WriteFrame(int mbufId, CVI_MEDIA_FRAME_INFO_T *pFrameInfo);

CVI_MBUF_HANDLE* app_ipcam_Mbuf_CreateReader(int mbufId, int bLastTime);

int app_ipcam_Mbuf_DestoryReader(CVI_MBUF_HANDLE* readerid);

int app_ipcam_Mbuf_Init();

int app_ipcam_Mbuf_UnInit();

#ifdef __cplusplus
};
#endif
#endif


