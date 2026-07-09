#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "app_ipcam_venc.h"
#include "app_ipcam_audio.h"
#include "app_ipcam_comm.h"
#include "cvi_audio.h"
#include "cvi_audio_aac_adp.h"
#include "app_ipcam_gpio.h"
#include "cvi_mbuf.h"

#ifdef RECORD_SUPPORT
#include "app_ipcam_record.h"
#endif
#ifdef MP3_SUPPORT
#include "cvi_mp3_decode.h"
#endif

#define ACODEC_ADC          "/dev/cv182xaadc"
#define ACODEC_DAC          "/dev/cv182xadac"
#define SPEAKER_GPIO        CVI_GPIOA_15

#define AUDIOAI_RECORD_PATH     "/mnt/sd/sample_record.raw"
#define AUDIOMP3_RECORD_PATH    "/mnt/sd/sample_record.mp3"
#define AUDIOAENC_RECORD_PATH   "/mnt/sd/sample_record.%s"
#define AO_MAX_FRAME_NUMS       1280
#define ADEC_MAX_FRAME_NUMS     320

#define AUDIO_OVERSIZE 512 * 1024 //ao audio file 512K oversize

#define pi2  (6.283185307179586476925286766559)
#define ln10 (2.30258509299404568401799145468)
#define eps2 (0.00000000000001)
#define eps1 (eps2 * eps2)
#define SAMPLE_MAX(x,y) (x) > (y) ? (x) : (y)
#define VOLUMEMAX   32767   //db [-80 0] <---> [0 120]
static CVI_BOOL MP3_RUNNING = CVI_FALSE;

APP_PARAM_AUDIO_CFG_T g_stAudioCfg, *g_pstAudioCfg = &g_stAudioCfg;

static RUN_THREAD_PARAM mAudioAiThread;
static RUN_THREAD_PARAM mAudioAencThread;
static RUN_THREAD_PARAM mAudioAoThread;

static APP_AUDIO_RUNSTATUS_T g_stAudioStatus;
static APP_AUDIO_RECORD_T g_stAudioRecord;
static APP_AUDIO_RECORD_T g_stAudioPlay;

static int gst_bAudioRtsp = 1;
static int gst_SocketFd = -1;
static struct sockaddr_in gst_TargetAddr;

static pthread_mutex_t RsAudioMutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef AI_BABYCRY_SUPPORT
#define AI_BUFFER_SECOND 3  //If use baby cry, plaease set 3
static CVI_U8 *g_pCryBuffer = NULL;
static pthread_mutex_t g_CryMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifdef MP3_SUPPORT
static int app_ipcam_Audio_MP3CB(void *inst, ST_MP3_DEC_INFO *pMp3_DecInfo, char *pBuff, int size)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    if (inst == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"Null pt [%s][%d]\n", __func__, __LINE__);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"Null pt in callback function force return\n");
        return -1;
    }
    if (pMp3_DecInfo == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"Null pt [%s][%d]\n", __func__, __LINE__);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"Null pt in callback pMp3_DecInfo force return\n");
        return -1;
    }
    if (pBuff == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"Null pt [%s][%d]\n", __func__, __LINE__);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"Null pt in callback pBuff force return\n");
        return -1;
    }

    //save the data or play out to AO
    if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_GET_FIRST_INFO)
    {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"stage:CVI_MP3_DEC_CB_STATUS_GET_FIRST_INFO\n");
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"========================================\n");
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"[channel]:[%d]\n", pMp3_DecInfo->channel_num);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"[sample rate]:[%d]\n", pMp3_DecInfo->sample_rate);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"[bit rate]:[%d]\n", pMp3_DecInfo->bit_rate);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"[frame cnt]:[%d]\n", pMp3_DecInfo->frame_cnt);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"[callback state] [0x%x]\n", pMp3_DecInfo->cbState);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"[size in bytes]:[%d]\n", size);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"=========================================\n");
    }
    else if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_STABLE)
    {
        //send the data to ao channel
    }
    else if ((pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_SMP_RATE_CHG) ||
             (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_INFO_CHG))
    {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"stage:CVI_MP3_DEC_CB_STATUS_INFO_CHG\n");
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"new chn[%d]\n", pMp3_DecInfo->channel_num);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"new rate[%d]\n", pMp3_DecInfo->sample_rate);
    }
    else if (pMp3_DecInfo->cbState == CVI_MP3_DEC_CB_STATUS_INIT)
    {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"stage:CVI_MP3_DEC_CB_STATUS_INIT\n");
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"Not decode any mp3 frame info yet........\n");
    }
    return s32Ret;
}
#endif

static float InvSqrt(float x)
{
    float xhalf = 0.5f * x;
    int *tmp = (int *)&x;
    int i = *tmp; // get bits for floating VALUE
    i = 0x5f375a86 - (i >> 1); // gives initial guess y0
    float *tmp1 = (float *)&i;
    x = *tmp1; // convert bits BACK to float
    x = x * (1.5f - (xhalf * x *x)); // Newton step, repeating increases accuracy
    x = x * (1.5f - (xhalf * x *x)); // Newton step, repeating increases accuracy
    x = x * (1.5f - (xhalf * x *x)); // Newton step, repeating increases accuracy

    return 1 / x;
}

static double Sqrt(double x)
{
    if (x == 0)
    {
        return 0;
    }
    double y = (double)InvSqrt((float)x);
    return (y + (x / y)) / 2;
}

static double NegativeLog(double q)
{ // q in ( 0.01, 0.1 ]
    double r = q;
    double s = q;
    double n = q;
    double q2 = q * q;
    double q1 = q2 * q;
    int p = 0;
    for (p = 1; (n *= q1) > eps1; s += n, q1 *= q2)
    {
        r += (p = !p) ? n : -n;
    }
    double u = 1 - (2 * r);
    double v = 1 + (2 * s);
    double t = u / v;
    double a = 1;
    double b = Sqrt(1 - (t * t * t * t));
    for (; (a - b) > eps2; b = Sqrt(a * b), a = t)
    {
        t = (a + b) / 2;
    }
    return pi2 / (a + b) / v / v;
}

static double calc_log(double x)
{
    if (x == 1)
    {
        return 0;
    }
    int k = 0;
    for (; x > 0.1; k++)
    {
        x /= 10;
    }
    for (; x <= 0.01; k--)
    {
        x *= 10;
    }
    return (k * ln10) - NegativeLog(x);
}

static double calc_log10( double x)
{
    return calc_log(x) / ln10;
}

static float app_ipcam_Audio_Calculate_DB(short* pstBuffer, int iFrameLen)
{
    if (NULL == pstBuffer || iFrameLen <= 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstBuffer is NULL or iFrameLen:%d less than 0!\n", iFrameLen);
        return -1;
    }

    int i = 0;
    float fVal = 0;
    float fDB = 0;
    float ret = 0;
    int sum = 0;
    signed short* pos = (signed short *)pstBuffer;
    for (i = 0; i < iFrameLen; i++)
    {
        sum += SAMPLE_MAX(abs(*pos), 3);
        pos++;
    }

    fVal =(float)sum / (iFrameLen * VOLUMEMAX);
    fDB = 20 * calc_log10(fVal);
    ret = (fDB * 1.5) + 120.0;
    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "frames:%d, current DB = (%f), ret=%f, value=%d\n", iFrameLen, fDB, ret, (sum / iFrameLen));
   return ret;
}

static void app_ipcam_Audio_Stereo2Mono(short *sInput, short *sOutput, int len)
{
    if ((NULL == sInput) || (sOutput == sInput) || (len <= 0)) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param invalid!\n");
        return ;
    }
    int i = 0;
    for (i = 0; i < len; i++) {
        *(sOutput + i) = *(sInput + (2 * i));
    }
}

static int app_ipcam_Audio_ParamChcek(APP_AUDIO_CFG_S *pstAudioCfg, APP_AUDIO_VQE_S *pstAudioVqe)
{
    if (NULL == pstAudioCfg || NULL == pstAudioVqe)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg/pstAudioVqe is NULL!\n");
        return -1;
    }

    APP_PROF_LOG_PRINT(LEVEL_WARN, "audio param chn:%d soundmode:%d enSamplerate:%d enAencType:%d vqe(%d %d %d)\n",
    pstAudioCfg->u32ChnCnt, pstAudioCfg->enSoundmode,
    pstAudioCfg->enSamplerate, pstAudioCfg->enAencType,
    pstAudioVqe->bAiAgcEnable, pstAudioVqe->bAiAnrEnable, pstAudioVqe->bAiAecEnable);
    if ((pstAudioCfg->u32ChnCnt == 1) && (pstAudioCfg->enSoundmode == AUDIO_SOUND_MODE_STEREO))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "audio param chn:%d dismatch soundmode:%d\n", pstAudioCfg->u32ChnCnt, pstAudioCfg->enSoundmode);
        return -1;
    }

    if ((pstAudioCfg->u32ChnCnt > 2) || (pstAudioCfg->u32ChnCnt <= 0))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "audio param chn:%d invalid\n", pstAudioCfg->u32ChnCnt);
        return -1;
    }

    if (pstAudioCfg->enAencType == PT_AAC)
    {
        /* AAC_VQE only support on 160 times frame length */
        if ((0 == pstAudioVqe->bAiAgcEnable && 0 == pstAudioVqe->bAiAnrEnable && 0 == pstAudioVqe->bAiAecEnable) &&
            (pstAudioCfg->u32PtNumPerFrm != 1024))
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "aac encode u32PtNumPerFrm must be 1024\n");
            pstAudioCfg->u32PtNumPerFrm = 1024;
        }
        else
        {

        }
    }
    else if (pstAudioCfg->enAencType == PT_G711A || pstAudioCfg->enAencType == PT_G711U)
    {
        if (pstAudioCfg->enSamplerate != AUDIO_SAMPLE_RATE_8000)
        {
            APP_PROF_LOG_PRINT(LEVEL_WARN, "G711 advice to 8K otherwise bandwidth no enough!\n");
        }
    }
    return 0;
}

static char *app_ipcam_Audio_GetFileExtensions(APP_AUDIO_CFG_S *pastAudioCfg)
{
    if (pastAudioCfg == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pastAudioCfg is NULL!\n");
        return "data";
    }
    switch (pastAudioCfg->enAencType)
    {
        case PT_G711A:
            return "g711a";
        case PT_G711U:
            return "g711u";
        case PT_ADPCMA:
            return "adpcm";
        case PT_G726:
            return "g726";
        case PT_LPCM:
            return "pcm";
        case PT_AAC:
            return "aac";
        default:
            return "data";
    }
}

#ifdef AI_BABYCRY_SUPPORT

static CVI_S32 ai_cry_queue_push(CVI_U8 *pQueueBuffer, CVI_U32 u32BufferLen, CVI_U32 u32DataLen, CVI_U8 *pData)
{
    _NULL_POINTER_CHECK_(pQueueBuffer, CVI_FAILURE);
    _NULL_POINTER_CHECK_(pData, CVI_FAILURE);
    memcpy(pQueueBuffer + u32BufferLen - u32DataLen, pData, u32DataLen);  //Push data into queue

    return CVI_SUCCESS;
}

static CVI_S32 ai_cry_queue_pop(CVI_U8 *pQueueBuffer, CVI_U32 u32BufferLen, CVI_U32 u32DataLen)
{
    _NULL_POINTER_CHECK_(pQueueBuffer, CVI_FAILURE);

    CVI_U8 *temp_buffer = malloc(u32BufferLen);
    _NULL_POINTER_CHECK_(temp_buffer, CVI_FAILURE);
    memset(temp_buffer, 0, u32BufferLen);

    memcpy(temp_buffer, pQueueBuffer, u32BufferLen);
    memset(pQueueBuffer, 0, u32BufferLen);
    memcpy(pQueueBuffer, temp_buffer + u32DataLen, u32BufferLen - u32DataLen);  //Delete queue head data
    free(temp_buffer);
    temp_buffer = NULL;

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Cry_Audio_Buffer_Get(CVI_U8 *pAudioBuffer, CVI_U32 u32BufferLen)
{
    _NULL_POINTER_CHECK_(pAudioBuffer, CVI_FAILURE);
    _NULL_POINTER_CHECK_(g_pCryBuffer, CVI_FAILURE);

    pthread_mutex_lock(&g_CryMutex);

    memcpy(pAudioBuffer, g_pCryBuffer, u32BufferLen);

    pthread_mutex_unlock(&g_CryMutex);
    return CVI_SUCCESS;
}
#endif

static CVI_VOID *Thread_AudioAo_Proc(CVI_VOID *pArgs)
{
    if (NULL == pArgs)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pArgs is NULL!\n");
        return NULL;
    }
    prctl(PR_SET_NAME, "Thread_AudioAo_Proc", 0, 0, 0);

    APP_AUDIO_CFG_S *pastAudioCfg = (APP_AUDIO_CFG_S *)pArgs;
    while (mAudioAoThread.bRun_flag)
    {
        if (access("/tmp/ao", F_OK) == 0)
        {
            remove("/tmp/ao");
            app_ipcam_Audio_AoPlay(AUDIOAI_RECORD_PATH, AUDIO_AO_PLAY_TYPE_RAW);
        }
        if (access("/tmp/adec", F_OK) == 0)
        {
            remove("/tmp/adec");
            char cAencFileName[64] = {0};
            snprintf(cAencFileName, sizeof(cAencFileName), AUDIOAENC_RECORD_PATH, app_ipcam_Audio_GetFileExtensions(pastAudioCfg));
            app_ipcam_Audio_AoPlay(cAencFileName, AUDIO_AO_PLAY_TYPE_AENC);
        }
        if (MP3_RUNNING)
        {
            app_ipcam_Audio_AoPlay(AUDIOMP3_RECORD_PATH, AUDIO_AO_PLAY_TYPE_MP3);
            MP3_RUNNING = CVI_FALSE;
        }
        if (g_stAudioPlay.iStatus)
        {
            if (app_ipcam_Audio_AoPlay(g_stAudioPlay.cAencFileName, AUDIO_AO_PLAY_TYPE_AENC) != 0)
            {
                g_stAudioPlay.iStatus = 0;
            }
        }
        if (gst_SocketFd > 0)
        {
            app_ipcam_Audio_AoPlay(g_stAudioPlay.cAencFileName, AUDIO_AO_PLAY_TYPE_INTERCOM);
        }
        else
        {
            sleep(1);
        }
    }
    APP_PROF_LOG_PRINT(LEVEL_WARN, "Thread_AudioAo_Proc EXIT!\n");
    return NULL;
}

static CVI_VOID *Thread_AudioAenc_Proc(CVI_VOID *pArgs)
{
    if (NULL == pArgs)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pArgs is NULL!\n");
        return NULL;
    }
    prctl(PR_SET_NAME, "Thread_Aenc_Proc", 0, 0, 0);
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_AUDIO_CFG_S *pastAudioCfg = (APP_AUDIO_CFG_S *)pArgs;
    AUDIO_STREAM_S stStream;
    FILE *fp_rec = CVI_NULL;
    FILE *fp_recex = CVI_NULL;

    CVI_U32 iAencRecordTime = 0;
    char cAencFileName[64] = {0};
    snprintf(cAencFileName, sizeof(cAencFileName), AUDIOAENC_RECORD_PATH, app_ipcam_Audio_GetFileExtensions(pastAudioCfg));
    while (mAudioAencThread.bRun_flag)
    {
        //for test record aenc
        if(access("/tmp/aenc", F_OK) == 0)
        {
            remove("/tmp/aenc");
            if (0 != pastAudioCfg->enReSamplerate)
            {
                iAencRecordTime = 10 * (pastAudioCfg->enReSamplerate / pastAudioCfg->u32PtNumPerFrm);
            }
            else
            {
                iAencRecordTime = 10 * (pastAudioCfg->enSamplerate / pastAudioCfg->u32PtNumPerFrm);
            }
        }
        s32Ret = CVI_AENC_GetStream(pastAudioCfg->u32AeChn, &stStream, CVI_FALSE);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AENC_GetStream(%d), failed with %#x!!\n", pastAudioCfg->u32AeChn, s32Ret);
            continue;
        }

        if (stStream.u32Len > 0)
        {
#ifdef RECORD_SUPPORT
            /* process streaming ; save to SD Card */
            //app_ipcam_Record_AudioInput(pastAudioCfg->enAencType, &stStream, stFrameNum);
#endif
            if (g_stAudioRecord.iStatus)
            {
                //for web record aenc
                if (fp_recex == NULL)
                {
                    char cAencFileName[128] = {0};
                    snprintf(cAencFileName, sizeof(cAencFileName), "%s_%d.%s",
                        g_stAudioRecord.cAencFileName, pastAudioCfg->enSamplerate, app_ipcam_Audio_GetFileExtensions(pastAudioCfg));
                    fp_recex = fopen(cAencFileName, "wb");
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "Start %s Record AENC!!\n", cAencFileName);
                }
                if (fp_recex)
                {
                    fwrite(stStream.pStream, 1, stStream.u32Len, fp_recex);
                }
                else
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "open %s failed!!\n", cAencFileName);
                }
            }
            else
            {
                if (fp_recex)
                {
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "End Record AENC!!\n");
                    fclose(fp_recex);
                    fp_recex = NULL;
                }
            }

            if (iAencRecordTime)
            {
                if (fp_rec == NULL)
                {
                    fp_rec = fopen(cAencFileName, "wb");
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "Start Record AENC!!\n");
                }
                if (fp_rec)
                {
                    fwrite(stStream.pStream, 1, stStream.u32Len, fp_rec);
                }
                iAencRecordTime--;
            }
            else
            {
                if (fp_rec)
                {
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "End Record AENC!!\n");
                    fclose(fp_rec);
                    fp_rec = NULL;
                }
            }

            if (gst_SocketFd > 0)
            {
                s32Ret = sendto(gst_SocketFd, stStream.pStream, stStream.u32Len, 0,
                    (struct sockaddr *)&gst_TargetAddr, sizeof(gst_TargetAddr));
                if (s32Ret <= 0)
                {
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "sendto failed with %#x!!\n", s32Ret);
                }
            }
        }

        s32Ret = CVI_AENC_ReleaseStream(pastAudioCfg->u32AeChn, &stStream);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AENC_ReleaseStream(%d), failed with %#x!!\n", pastAudioCfg->u32AeChn, s32Ret);
            return NULL;
        }
    }
    if (fp_rec)
    {
        fclose(fp_rec);
        fp_rec = NULL;
    }
    if (fp_recex)
    {
        fclose(fp_recex);
        fp_recex = NULL;
    }
    APP_PROF_LOG_PRINT(LEVEL_WARN, "Thread_AudioAenc_Proc EXIT!\n");
    return NULL;
}

static CVI_VOID *Thread_AudioAi_Proc(CVI_VOID *pArgs)
{
    if (NULL == pArgs)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pArgs is NULL!\n");
        return NULL;
    }
    prctl(PR_SET_NAME, "Thread_AudioAi_Proc", 0, 0, 0);

    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PARAM_VENC_CTX_S *pstVencCtx = app_ipcam_Venc_Param_Get();
    APP_AUDIO_CFG_S *pastAudioCfg = (APP_AUDIO_CFG_S *)pArgs;
    if (pastAudioCfg->u32ChnCnt == 1) {
        //chn0 bind aenc chn1 get raw frame
        APP_PROF_LOG_PRINT(LEVEL_WARN, "chn0 bind aenc can't get raw frame!\n");
        return NULL;
    }

    FILE *fp_rec = CVI_NULL;
    CVI_U32 iAudioFactor = 0;
    CVI_U32 iAiRecordTime = 0;
    AEC_FRAME_S   stAecFrm;
    AUDIO_FRAME_S stFrame;
    AUDIO_FRAME_S stNewFrame;

    #ifdef AI_BABYCRY_SUPPORT
    CVI_U32 ai_buffer_len = pastAudioCfg->enSamplerate * (CVI_U32)(pastAudioCfg->enBitwidth + 1) * AI_BUFFER_SECOND;
    #endif

    //AEC will output only one single channel with 2 channels in
    CVI_S32 s32OutputChnCnt = 1;//(pastAudioCfg->enSoundmode == AUDIO_SOUND_MODE_MONO) ? 1 : 2;
    if (pastAudioCfg->enBitwidth == AUDIO_BIT_WIDTH_16)
    {
        iAudioFactor = s32OutputChnCnt * 2;
    }
    else
    {
        iAudioFactor = s32OutputChnCnt * 4;
    }

    CVI_MEDIA_FRAME_INFO_T stFrameInfo;
    stFrameInfo.frameBuf = malloc(pastAudioCfg->u32PtNumPerFrm * iAudioFactor);
    if (NULL == stFrameInfo.frameBuf)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "frameBuf malloc fail\n");
        return NULL;
    }

    while (mAudioAiThread.bRun_flag)
    {
        if(access("/tmp/audio", F_OK) == 0)
        {
            remove("/tmp/audio");
            iAiRecordTime = 10 * (pastAudioCfg->enSamplerate / pastAudioCfg->u32PtNumPerFrm);
        }
        //chn0 bind aenc chn1 get raw frame
        s32Ret = CVI_AI_GetFrame(pastAudioCfg->u32AiDevId, (pastAudioCfg->u32ChnCnt - 1), &stFrame, &stAecFrm, -1);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_GetFrame None!!\n");
            continue;
        }

        if (stFrame.u32Len > 0)
        {
            memcpy(&stNewFrame, &stFrame, sizeof(AUDIO_FRAME_S));
            stNewFrame.u32Len = (stFrame.u32Len * iAudioFactor);
            stNewFrame.u64VirAddr[0] = malloc(stNewFrame.u32Len);
            if (NULL == stNewFrame.u64VirAddr[0]) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "stNewFrame.u64VirAddr[0] malloc failed!\n");
                break;
            }
            if (pastAudioCfg->enSoundmode == AUDIO_SOUND_MODE_MONO) {
                memcpy(stNewFrame.u64VirAddr[0], stFrame.u64VirAddr[0], stNewFrame.u32Len);
            } else {
                //stereo to mono
                app_ipcam_Audio_Stereo2Mono((short *)stFrame.u64VirAddr[0], (short *)stNewFrame.u64VirAddr[0], stFrame.u32Len);
            }

            #ifdef AI_BABYCRY_SUPPORT
            {
                pthread_mutex_lock(&g_CryMutex);
                ai_cry_queue_pop(g_pCryBuffer, ai_buffer_len, stNewFrame.u32Len);
                ai_cry_queue_push(g_pCryBuffer, ai_buffer_len, stNewFrame.u32Len, stNewFrame.u64VirAddr[0]);
                pthread_mutex_unlock(&g_CryMutex);
            }
            #endif

            memset(stFrameInfo.frameBuf, 0, pastAudioCfg->u32PtNumPerFrm * iAudioFactor);
            memcpy(stFrameInfo.frameBuf, stNewFrame.u64VirAddr[0], stNewFrame.u32Len);
            stFrameInfo.frameParam.frameLen = stNewFrame.u32Len;
            stFrameInfo.frameParam.frameIndex = stNewFrame.u32Seq;
            stFrameInfo.frameParam.frameType  = CVI_MEDIA_AFRAME_A;
            stFrameInfo.frameParam.frameCodec = pastAudioCfg->enAencType;
            stFrameInfo.frameParam.framePts   = stNewFrame.u64TimeStamp;
            stFrameInfo.frameParam.frameTime  = time(NULL);

            for (int i = 0; i < pstVencCtx->s32VencChnCnt; i++)
            {
                if (pstVencCtx->astVencChnCfg[i].bEnable && (pstVencCtx->astVencChnCfg[i].enType != PT_JPEG))
                {
                    app_ipcam_Mbuf_Audio_WriteFrame(i, &stFrameInfo);
                }
            }

            app_ipcam_Audio_Calculate_DB((short *)stNewFrame.u64VirAddr[0], stNewFrame.u32Len / 2);
            // test code record ai auido
            if (iAiRecordTime)
            {
                if (fp_rec == NULL)
                {
                    fp_rec = fopen(AUDIOAI_RECORD_PATH, "wb");
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "Start Record AI!!\n");
                }
                if (fp_rec)
                {
                    if ((stNewFrame.enBitwidth != AUDIO_BIT_WIDTH_16) && (stNewFrame.enBitwidth != AUDIO_BIT_WIDTH_32))
                    {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Not support format bitwidth\n");
                    }
                    else
                    {
                        fwrite(stNewFrame.u64VirAddr[0], 1, stNewFrame.u32Len, fp_rec);
                    }
                }
                iAiRecordTime--;
            }
            else
            {
                if (fp_rec)
                {
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "End Record AI!!\n");
                    fclose(fp_rec);
                    fp_rec = NULL;
                }
            }

            free(stNewFrame.u64VirAddr[0]);
            stNewFrame.u64VirAddr[0] = NULL;
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "block mode return size 0...\n");
        }
    }
    if (fp_rec)
    {
        fclose(fp_rec);
        fp_rec = NULL;
    }
    if (stFrameInfo.frameBuf)
    {
        free(stFrameInfo.frameBuf);
    }
    APP_PROF_LOG_PRINT(LEVEL_WARN, "Thread_AudioAi_Proc EXIT!\n");
    return NULL;
}

static CVI_S32 app_ipcam_Audio_CfgAcodec()
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    /*** INNER AUDIO CODEC ***/
    CVI_S32 volume = 0;
    CVI_S32 volumedb = 0;
    APP_AUDIO_CFG_S *pstAudioCfg = &g_pstAudioCfg->astAudioCfg;

    /* ao gpio enable*/
    app_ipcam_Gpio_Value_Set(SPEAKER_GPIO, CVI_GPIO_VALUE_H);

    //dual os set AO/AI by sdk api
    s32Ret = CVI_AO_GetVolume(pstAudioCfg->u32AoDevId, &volumedb);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Get Volume Aout[%d]\n", volumedb);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "[Error][%s][%d]\n", __func__, __LINE__);
    }
    s32Ret = CVI_AI_GetVolume(pstAudioCfg->u32AiDevId, &volume);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Get Volume Ain[%d]\n", volume);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "[Error][%s][%d]\n", __func__, __LINE__);
    }

    volumedb = g_pstAudioCfg->astAudioVol.iDacRVol;
    s32Ret = CVI_AO_SetVolume(pstAudioCfg->u32AoDevId, volumedb);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "[Error][%s][%d]\n", __func__, __LINE__);
    }
    volume = g_pstAudioCfg->astAudioVol.iAdcLVol;
    s32Ret = CVI_AI_SetVolume(pstAudioCfg->u32AiDevId, volume);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "[Error][%s][%d]\n", __func__, __LINE__);
    }

    return s32Ret;
}

/******************************************************************************/
/* function : Start Aenc*/
/******************************************************************************/
static CVI_S32  app_ipcam_Audio_AencGetAttr(APP_AUDIO_CFG_S *pstAudioCfg, AENC_CHN_ATTR_S *pAencAttrs)
{
    CVI_S32 s32Ret = CVI_FAILURE;
    if (pAencAttrs == NULL || pstAudioCfg == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Attribute NULL !!! Cannot proceed to create AENC channe!!\n");
        return s32Ret;
    }

    memset(pAencAttrs, 0, sizeof(AENC_CHN_ATTR_S));
    pAencAttrs->enType          = pstAudioCfg->enAencType;
    pAencAttrs->u32BufSize      = pstAudioCfg->u32FrmNum;
    pAencAttrs->u32PtNumPerFrm  = pstAudioCfg->u32PtNumPerFrm;
    if (pAencAttrs->enType == PT_ADPCMA)
    {
        AENC_ATTR_ADPCM_S *pstAdpcmAenc = (AENC_ATTR_ADPCM_S *)malloc(sizeof(AENC_ATTR_ADPCM_S));
        if (pstAdpcmAenc == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAdpcmAenc malloc failed!\n");
            return s32Ret;
        }
        pstAdpcmAenc->enADPCMType = ADPCM_TYPE_DVI4;
        pAencAttrs->pValue       = (CVI_VOID *)pstAdpcmAenc;
    }
    else if (pAencAttrs->enType == PT_G711A || pAencAttrs->enType == PT_G711U)
    {
        AENC_ATTR_G711_S *pstAencG711 = (AENC_ATTR_G711_S *)malloc(sizeof(AENC_ATTR_G711_S));
        if (pstAencG711 == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAencG711 malloc failed!\n");
            return s32Ret;
        }
        pAencAttrs->pValue = (CVI_VOID *)pstAencG711;
    }
    else if (pAencAttrs->enType == PT_G726)
    {
        AENC_ATTR_G726_S *pstAencG726 = (AENC_ATTR_G726_S *)malloc(sizeof(AENC_ATTR_G726_S));
        if (pstAencG726 == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAdpcmAenc malloc failed!\n");
            return s32Ret;
        }
        pstAencG726->enG726bps = MEDIA_G726_32K;
        pAencAttrs->pValue = (CVI_VOID *)pstAencG726;
    }
    else if (pAencAttrs->enType == PT_LPCM)
    {
        AENC_ATTR_LPCM_S *pstAencLpcm = (AENC_ATTR_LPCM_S *)malloc(sizeof(AENC_ATTR_LPCM_S));
        if (pstAencLpcm == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAdpcmAenc malloc failed!\n");
            return s32Ret;
        }
        pAencAttrs->pValue = (CVI_VOID *)pstAencLpcm;
    }
    else if (pAencAttrs->enType == PT_AAC)
    {
        APP_AUDIO_VQE_S *pstAudioVqe = &g_pstAudioCfg->astAudioVqe;
        AENC_ATTR_AAC_S  *pstAencAac = (AENC_ATTR_AAC_S *)malloc(sizeof(AENC_ATTR_AAC_S));
        if (pstAencAac == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAencAac malloc failed!\n");
            return s32Ret;
        }
        pstAencAac->enAACType = AAC_TYPE_AACLC;
        pstAencAac->enBitRate = AAC_BPS_32K;
        pstAencAac->enBitWidth = AUDIO_BIT_WIDTH_16;

        pstAencAac->enSmpRate = (0 != pstAudioCfg->enReSamplerate) ? pstAudioCfg->enReSamplerate : pstAudioCfg->enSamplerate;
        pstAencAac->enSoundMode = pstAudioVqe->bAiAecEnable ? AUDIO_SOUND_MODE_MONO : pstAudioCfg->enSoundmode;
        pstAencAac->enTransType = AAC_TRANS_TYPE_ADTS;
        pstAencAac->s16BandWidth = 0;
        pAencAttrs->pValue = pstAencAac;
        s32Ret = CVI_MPI_AENC_AacInit();
        APP_PROF_LOG_PRINT(LEVEL_WARN, "[cvi_info] CVI_MPI_AENC_AacInit s32Ret:%d\n", s32Ret);
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupport aenc type!\n");
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Audio_AdecStop(APP_AUDIO_CFG_S *pstAudioCfg)
{
    if (NULL == pstAudioCfg)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg is NULL!\n");
        return -1;
    }

    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_stAudioStatus.bAdecInit)
    {
        s32Ret = CVI_ADEC_DestroyChn(pstAudioCfg->u32AdChn);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ADEC_DestroyChn(%d) failed with %#x!\n", pstAudioCfg->u32AdChn, s32Ret);
            return s32Ret;
        }

        if (pstAudioCfg->enAencType == PT_AAC)
        {
            CVI_MPI_ADEC_AacDeInit();
        }

        g_stAudioStatus.bAdecInit = 0;
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Stop Adec OK!\n");
    }
    return s32Ret;
}

static CVI_S32 app_ipcam_Audio_AdecStart(APP_AUDIO_CFG_S *pstAudioCfg, APP_AUDIO_VQE_S *pstAudioVqe)
{
    if (NULL == pstAudioCfg || NULL == pstAudioVqe)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg/pstAudioVqe is NULL!\n");
        return -1;
    }

    if (g_stAudioStatus.bAdecInit)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Adec Already Init!\n");
        return 0;
    }

    CVI_S32 s32Ret = CVI_SUCCESS;

    ADEC_CHN_ATTR_S stAdecAttr;
    ADEC_ATTR_ADPCM_S stAdpcm;
    ADEC_ATTR_G711_S stAdecG711;
    ADEC_ATTR_G726_S stAdecG726;
    ADEC_ATTR_LPCM_S stAdecLpcm;
    ADEC_ATTR_AAC_S  stAdecAac;

    memset(&stAdecAttr, 0, sizeof(ADEC_CHN_ATTR_S));
    stAdecAttr.s32Sample_rate       = (0 != pstAudioCfg->enReSamplerate) ? pstAudioCfg->enReSamplerate : pstAudioCfg->enSamplerate;
    //rtsp occupy 1 chn
    stAdecAttr.s32ChannelNums       = (pstAudioVqe->bAiAecEnable || gst_bAudioRtsp) ? 1 : pstAudioCfg->u32ChnCnt;
    stAdecAttr.s32frame_size        = pstAudioCfg->u32PtNumPerFrm;
    stAdecAttr.s32BytesPerSample    = 2;//16 bits , 2 bytes per samples
    stAdecAttr.enType               = pstAudioCfg->enAencType;
    stAdecAttr.u32BufSize           = pstAudioCfg->u32FrmNum;
    stAdecAttr.enMode               = ADEC_MODE_STREAM;/* propose use pack mode in your app */
    stAdecAttr.bFileDbgMode         = CVI_FALSE;
    if (stAdecAttr.enType == PT_ADPCMA)
    {
        memset(&stAdpcm, 0, sizeof(ADEC_ATTR_ADPCM_S));
        stAdecAttr.pValue = &stAdpcm;
        stAdpcm.enADPCMType = ADPCM_TYPE_DVI4;
    }
    else if (stAdecAttr.enType == PT_G711A || stAdecAttr.enType == PT_G711U)
    {
        memset(&stAdecG711, 0, sizeof(ADEC_ATTR_G711_S));
        stAdecAttr.pValue = &stAdecG711;
    }
    else if (stAdecAttr.enType == PT_G726)
    {
        memset(&stAdecG726, 0, sizeof(ADEC_ATTR_G726_S));
        stAdecAttr.pValue = &stAdecG726;
        stAdecG726.enG726bps = MEDIA_G726_32K;
    }
    else if (stAdecAttr.enType == PT_LPCM)
    {
        memset(&stAdecLpcm, 0, sizeof(ADEC_ATTR_LPCM_S));
        stAdecAttr.pValue = &stAdecLpcm;
        stAdecAttr.enMode = ADEC_MODE_PACK;/* lpcm must use pack mode */
    }
    else if (stAdecAttr.enType == PT_AAC)
    {
        CVI_MPI_ADEC_AacInit();
        stAdecAac.enTransType = AAC_TRANS_TYPE_ADTS;
        stAdecAac.enSoundMode = pstAudioVqe->bAiAecEnable ? AUDIO_SOUND_MODE_MONO : pstAudioCfg->enSoundmode;
        stAdecAac.enSmpRate = (0 != pstAudioCfg->enReSamplerate) ? pstAudioCfg->enReSamplerate : pstAudioCfg->enSamplerate;
        stAdecAttr.pValue = &stAdecAac;
        stAdecAttr.enMode = ADEC_MODE_STREAM;   /* aac should be stream mode */
        stAdecAttr.s32frame_size = 1024;
    }

    s32Ret = CVI_ADEC_CreateChn(pstAudioCfg->u32AdChn, &stAdecAttr);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ADEC_CreateChn(%d) failed with %#x!\n", pstAudioCfg->u32AdChn, s32Ret);
        return s32Ret;
    }

    g_stAudioStatus.bAdecInit = 1;
    APP_PROF_LOG_PRINT(LEVEL_WARN, "Start Adec OK!\n");
    return s32Ret;
}

static CVI_S32 app_ipcam_Audio_AencStop(APP_AUDIO_CFG_S *pstAudioCfg)
{
    if (NULL == pstAudioCfg)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg is NULL!\n");
        return -1;
    }

    CVI_S32 s32Ret = CVI_SUCCESS;
    if (g_stAudioStatus.bAencInit)
    {
        mAudioAencThread.bRun_flag = 0;
        if (mAudioAencThread.mRun_PID != 0)
        {
            pthread_join(mAudioAencThread.mRun_PID, NULL);
            mAudioAencThread.mRun_PID = 0;
        }

        MMF_CHN_S stSrcChn, stDestChn;
        memset(&stSrcChn, 0 , sizeof(stSrcChn));
        memset(&stDestChn, 0 , sizeof(stDestChn));
        stSrcChn.enModId = CVI_ID_AI;
        stSrcChn.s32DevId = pstAudioCfg->u32AiDevId;
        stSrcChn.s32ChnId = 0;
        stDestChn.enModId = CVI_ID_AENC;
        stDestChn.s32DevId = pstAudioCfg->u32AeDevId;
        stDestChn.s32ChnId = pstAudioCfg->u32AeChn;

        s32Ret = CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AUD_SYS_Bind failed with %#x!\n", s32Ret);
            return s32Ret;
        }

        s32Ret = CVI_AENC_DestroyChn(pstAudioCfg->u32AeChn);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AENC_DestroyChn(%d) failed with %#x!\n", pstAudioCfg->u32AeChn, s32Ret);
            return s32Ret;
        }

        if (pstAudioCfg->enAencType == PT_AAC)
        {
            CVI_MPI_AENC_AacDeInit();
        }

        g_stAudioStatus.bAencInit = 0;
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Stop Aenc OK!\n");
    }
    return s32Ret;
}

static CVI_S32 app_ipcam_Audio_AencStart(APP_AUDIO_CFG_S *pstAudioCfg)
{
    if (NULL == pstAudioCfg)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg is NULL!\n");
        return -1;
    }

    if (g_stAudioStatus.bAencInit)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Aenc Already Init!\n");
        return 0;
    }

    CVI_S32 s32Ret = CVI_SUCCESS;
    AENC_CHN_ATTR_S stAencAttr;
    s32Ret = app_ipcam_Audio_AencGetAttr(pstAudioCfg, &stAencAttr);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_AencGetAttr failed!!\n");
        return s32Ret;
    }

    /* If toggle the bFileDbgMode, audio in frame will save to file after encode */
    stAencAttr.bFileDbgMode = CVI_FALSE;
    /* create aenc chn*/
    s32Ret = CVI_AENC_CreateChn(pstAudioCfg->u32AeChn, &stAencAttr);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AENC_CreateChn(%d) failed with %#x!\n", pstAudioCfg->u32AeChn, s32Ret);
        return s32Ret;
    }

    if (stAencAttr.pValue)
    {
        free(stAencAttr.pValue);
        stAencAttr.pValue = NULL;
    }

    MMF_CHN_S stSrcChn, stDestChn;
    memset(&stSrcChn, 0 , sizeof(stSrcChn));
    memset(&stDestChn, 0 , sizeof(stDestChn));
    stSrcChn.enModId = CVI_ID_AI;
    stSrcChn.s32DevId = pstAudioCfg->u32AiDevId;
    stSrcChn.s32ChnId = pstAudioCfg->u32AiChn;
    stDestChn.enModId = CVI_ID_AENC;
    stDestChn.s32DevId = pstAudioCfg->u32AeDevId;
    stDestChn.s32ChnId = pstAudioCfg->u32AeChn;

    s32Ret = CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AUD_SYS_Bind failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    pthread_attr_t pthread_attr;
    pthread_attr_init(&pthread_attr);

    memset(&mAudioAencThread, 0, sizeof(mAudioAencThread));
    mAudioAencThread.bRun_flag = 1;

    pfp_task_entry fun_entry = Thread_AudioAenc_Proc;
    s32Ret = pthread_create(
                    &mAudioAencThread.mRun_PID,
                    &pthread_attr,
                    fun_entry,
                    (CVI_VOID *)pstAudioCfg);
    if (s32Ret)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "audio aenc pthread_create failed:0x%x\n", s32Ret);
        return CVI_FAILURE;
    }
    g_stAudioStatus.bAencInit = 1;
    APP_PROF_LOG_PRINT(LEVEL_WARN, "Start Aenc OK!\n");
    return s32Ret;
}

static CVI_S32 app_ipcam_Audio_AoStop(APP_AUDIO_CFG_S *pstAudioCfg, APP_AUDIO_VQE_S *pstAudioVqe)
{
    if (NULL == pstAudioCfg || NULL == pstAudioVqe)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg/pstAudioVqe is NULL!\n");
        return -1;
    }

    CVI_S32 s32Ret = CVI_SUCCESS;
    if (g_stAudioStatus.bAoInit)
    {
        mAudioAoThread.bRun_flag = 0;
        if (mAudioAoThread.mRun_PID != 0)
        {
            pthread_join(mAudioAoThread.mRun_PID, NULL);
            mAudioAoThread.mRun_PID = 0;
        }

#if 0
        if (pstAudioVqe->bAoAgcEnable || pstAudioVqe->bAoAnrEnable)
        {
            s32Ret = CVI_AO_DisableVqe(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_DisableVqe failed with %#x!\n", s32Ret);
                return s32Ret;
            }
        }
#endif

        if (0 != pstAudioCfg->enReSamplerate)
        {
            s32Ret = CVI_AO_DisableReSmp(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_DisableReSmp failed with %#x!\n", s32Ret);
                return s32Ret;
            }
        }

        s32Ret = CVI_AO_DisableChn(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_DisableChn(%d) failed with %#x!\n", pstAudioCfg->u32AoChn, s32Ret);
            return s32Ret;
        }

        s32Ret = CVI_AO_Disable(pstAudioCfg->u32AoDevId);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_Disable(%d) failed with %#x!\n", pstAudioCfg->u32AoDevId, s32Ret);
            return s32Ret;
        }

        g_stAudioStatus.bAoInit = 0;
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Stop Ao OK!\n");
    }

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Audio_AoStart(APP_AUDIO_CFG_S *pstAudioCfg, APP_AUDIO_VQE_S *pstAudioVqe)
{
    if (NULL == pstAudioCfg || NULL == pstAudioVqe)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg/pstAudioVqe is NULL!\n");
        return -1;
    }

    if (g_stAudioStatus.bAoInit)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Ao Already Init!\n");
        return 0;
    }

    CVI_S32 s32Ret = CVI_SUCCESS;
    AIO_ATTR_S stAoAttr;
    memset(&stAoAttr, 0, sizeof(AIO_ATTR_S));
    stAoAttr.enSamplerate   = pstAudioCfg->enSamplerate;
    stAoAttr.enBitwidth     = pstAudioCfg->enBitwidth;
    stAoAttr.enWorkmode     = pstAudioCfg->enWorkmode;
    stAoAttr.u32EXFlag      = pstAudioCfg->u32EXFlag;
    stAoAttr.u32FrmNum      = pstAudioCfg->u32FrmNum;
    stAoAttr.u32PtNumPerFrm = pstAudioCfg->u32PtNumPerFrm;
	stAoAttr.enSoundmode    = pstAudioCfg->enSoundmode;
	stAoAttr.u32ChnCnt      = pstAudioCfg->u32ChnCnt;
    stAoAttr.u32ClkSel      = pstAudioCfg->u32ClkSel;
    stAoAttr.enI2sType      = pstAudioCfg->enI2sType;

//Currently not support ao vqe
#if 0
    if (pstAudioVqe->bAoAgcEnable || pstAudioVqe->bAoAnrEnable)
    {
        AO_VQE_CONFIG_S stAoVqeAttr;
        AO_VQE_CONFIG_S *pstAoVqeAttr = (AO_VQE_CONFIG_S *)&stAoVqeAttr;
        memset(pstAoVqeAttr, 0, sizeof(AO_VQE_CONFIG_S));
        //VQE only support on 8k/16k sample rate
        if ((pstAudioCfg->enSamplerate == AUDIO_SAMPLE_RATE_8000) ||
            (pstAudioCfg->enSamplerate == AUDIO_SAMPLE_RATE_16000))
        {
            if (pstAudioVqe->bAiAgcEnable)
            {
                pstAoVqeAttr->u32OpenMask |= AO_VQE_MASK_AGC;
                pstAoVqeAttr->stAgcCfg    = pstAudioVqe->mAoAgcCfg;
            }
            if (pstAudioVqe->bAiAnrEnable)
            {
                pstAoVqeAttr->u32OpenMask |= AO_VQE_MASK_ANR;
                pstAoVqeAttr->stAnrCfg    = pstAudioVqe->mAiAnrCfg;
            }
            pstAoVqeAttr->s32FrameSample    = pstAudioCfg->u32PtNumPerFrm;
            pstAoVqeAttr->s32WorkSampleRate = pstAudioCfg->enSamplerate;

            s32Ret = CVI_AO_SetVqeAttr(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn, pstAoVqeAttr);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_SetVqeAttr failed with %#x!\n", s32Ret);
            }
            else
            {
                s32Ret = CVI_AO_EnableVqe(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn);
                if (s32Ret != CVI_SUCCESS)
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_EnableVqe failed with %#x!\n", s32Ret);
                }
                else
                {
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "openmask[0x%x]\n", pstAoVqeAttr->u32OpenMask);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "s32WorkSampleRate[%d]\n", pstAoVqeAttr->s32WorkSampleRate);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "s32FrameSample[%d]\n", pstAoVqeAttr->s32FrameSample);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "AGC PARAM---------------------------------------------\n");
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "para_agc_max_gain[%d]\n", pstAoVqeAttr->stAgcCfg.para_agc_max_gain);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "para_agc_target_high[%d]\n", pstAoVqeAttr->stAgcCfg.para_agc_target_high);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "para_agc_target_low[%d]\n", pstAoVqeAttr->stAgcCfg.para_agc_target_low);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "para_agc_vad_enable[%d]\n", pstAoVqeAttr->stAgcCfg.para_agc_vad_enable);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "para_agc_vad_cnt[%d]\n", pstAoVqeAttr->stAgcCfg.para_agc_vad_cnt);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "para_agc_cut6_enable[%d]\n", pstAoVqeAttr->stAgcCfg.para_agc_cut6_enable);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "ANR PARAM---------------------------------------------\n");
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "para_nr_snr_coeff[%d]\n", pstAoVqeAttr->stAnrCfg.para_nr_snr_coeff);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "para_nr_noise_coeff[%d]\n", pstAoVqeAttr->stAnrCfg.para_nr_noise_coeff);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "Start AO VQE!\n");
                }
            }
        }
    }
#endif

    s32Ret = CVI_AO_SetPubAttr(pstAudioCfg->u32AoDevId, &stAoAttr);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_SetPubAttr(%d) failed with %#x!\n", pstAudioCfg->u32AoDevId, s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AO_Enable(pstAudioCfg->u32AoDevId);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_Enable(%d) failed with %#x!\n", pstAudioCfg->u32AoDevId, s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AO_EnableChn(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_EnableChn(%d) failed with %#x!\n", pstAudioCfg->u32AoChn, s32Ret);
        return s32Ret;
    }

    if (0 != pstAudioCfg->enReSamplerate)
    {
        s32Ret = CVI_AO_EnableReSmp(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn, pstAudioCfg->enReSamplerate);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_EnableReSmp(%d) failed with %#x!\n", pstAudioCfg->u32AoChn, s32Ret);
            return s32Ret;
        }
    }
    pthread_attr_t pthread_attr;
    pthread_attr_init(&pthread_attr);
    memset(&mAudioAoThread, 0, sizeof(mAudioAoThread));
    mAudioAoThread.bRun_flag = 1;

    pfp_task_entry fun_entry = Thread_AudioAo_Proc;
    s32Ret = pthread_create(
                    &mAudioAoThread.mRun_PID,
                    &pthread_attr,
                    fun_entry,
                    (CVI_VOID *)pstAudioCfg);
    if (s32Ret)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "audio ao pthread_create failed:0x%x\n", s32Ret);
        return CVI_FAILURE;
    }

    g_stAudioStatus.bAoInit = 1;
    APP_PROF_LOG_PRINT(LEVEL_WARN, "Start AO OK!\n");
    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Audio_AiStop(APP_AUDIO_CFG_S *pstAudioCfg, APP_AUDIO_VQE_S *pstAudioVqe)
{
    if (NULL == pstAudioCfg || NULL == pstAudioVqe)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg/pstAudioVqe is NULL!\n");
        return -1;
    }

    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_S32 i = 0;
    if (g_stAudioStatus.bAiInit)
    {
        mAudioAiThread.bRun_flag = 0;
        if (mAudioAiThread.mRun_PID != 0)
        {
            pthread_join(mAudioAiThread.mRun_PID, NULL);
            mAudioAiThread.mRun_PID = 0;
        }

        {
            #ifdef AI_BABYCRY_SUPPORT
            free(g_pCryBuffer);
            g_pCryBuffer = NULL;
            #endif
        }

        for(i = 0; i <= (CVI_S32)pstAudioCfg->u32ChnCnt; i++)
        {
            if (i == 0)
            {
                if (pstAudioVqe->bAiAgcEnable || pstAudioVqe->bAiAnrEnable || pstAudioVqe->bAiAecEnable)
                {
                    s32Ret = CVI_AI_DisableVqe(pstAudioCfg->u32AiDevId, i);
                    if (s32Ret != CVI_SUCCESS)
                    {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_DisableVqe failed with %#x!\n", s32Ret);
                        return s32Ret;
                    }
                }
                if (0 != pstAudioCfg->enReSamplerate)
                {
                    s32Ret = CVI_AI_DisableReSmp(pstAudioCfg->u32AiDevId, i);
                    if (s32Ret != CVI_SUCCESS)
                    {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_DisableReSmp failed with %#x!\n", s32Ret);
                        return s32Ret;
                    }
                }
            }

            s32Ret = CVI_AI_DisableChn(pstAudioCfg->u32AiDevId, i);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_DisableChn(%d) failed with %#x!\n", i, s32Ret);
                return s32Ret;
            }
        }

        s32Ret = CVI_AI_Disable(pstAudioCfg->u32AiDevId);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_Disable failed with %#x!\n", s32Ret);
            return s32Ret;
        }

        g_stAudioStatus.bAiInit = 0;
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Stop Ai OK!\n");
    }
    return s32Ret;
}

static CVI_S32 app_ipcam_Audio_AiStart(APP_AUDIO_CFG_S *pstAudioCfg, APP_AUDIO_VQE_S *pstAudioVqe)
{
    if (NULL == pstAudioCfg || NULL == pstAudioVqe)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg/pstAudioVqe is NULL!\n");
        return -1;
    }

    if (g_stAudioStatus.bAiInit)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Ai Already Init!\n");
        return 0;
    }

    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_S32 i = 0;

    AIO_ATTR_S stAiAttr;
    memset(&stAiAttr, 0, sizeof(AIO_ATTR_S));
    stAiAttr.enSamplerate   = pstAudioCfg->enSamplerate;
    stAiAttr.enBitwidth     = pstAudioCfg->enBitwidth;
    stAiAttr.enWorkmode     = pstAudioCfg->enWorkmode;
    stAiAttr.enSoundmode    = pstAudioCfg->enSoundmode;
    stAiAttr.u32EXFlag      = pstAudioCfg->u32EXFlag;
    stAiAttr.u32FrmNum      = pstAudioCfg->u32FrmNum;
    stAiAttr.u32PtNumPerFrm = pstAudioCfg->u32PtNumPerFrm;
    stAiAttr.u32ChnCnt      = pstAudioCfg->u32ChnCnt;
    stAiAttr.u32ClkSel      = pstAudioCfg->u32ClkSel;
    stAiAttr.enI2sType      = pstAudioCfg->enI2sType;
    s32Ret = CVI_AI_SetPubAttr(pstAudioCfg->u32AiDevId, &stAiAttr);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_SetPubAttr failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    for (i = 0; i <= (CVI_S32)stAiAttr.u32ChnCnt; i++)
    {
        s32Ret = CVI_AI_EnableChn(pstAudioCfg->u32AiDevId, i);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_EnableChn(%d) failed with %#x!\n", i, s32Ret);
            return s32Ret;
        }

        if (i == 0)
        {
            if (pstAudioVqe->bAiAgcEnable || pstAudioVqe->bAiAnrEnable || pstAudioVqe->bAiAecEnable)
            {
                AI_TALKVQE_CONFIG_S stAiVqeTalkAttr;
                AI_TALKVQE_CONFIG_S *pstAiVqeTalkAttr = (AI_TALKVQE_CONFIG_S *)&stAiVqeTalkAttr;
                memset(&stAiVqeTalkAttr, 0, sizeof(AI_TALKVQE_CONFIG_S));
                //VQE only support on 8k/16k sample rate
                if ((pstAudioCfg->enSamplerate == AUDIO_SAMPLE_RATE_8000) ||
                    (pstAudioCfg->enSamplerate == AUDIO_SAMPLE_RATE_16000))
                {
                    if (pstAudioVqe->bAiAgcEnable)
                    {
                        pstAiVqeTalkAttr->u32OpenMask |= (AGC_ENABLE | DCREMOVER_ENABLE);
                        pstAiVqeTalkAttr->stAgcCfg    = pstAudioVqe->mAiAgcCfg;
                    }
                    if (pstAudioVqe->bAiAnrEnable)
                    {
                        pstAiVqeTalkAttr->u32OpenMask |= (NR_ENABLE | DCREMOVER_ENABLE);
                        pstAiVqeTalkAttr->stAnrCfg    = pstAudioVqe->mAiAnrCfg;
                    }
                    if (pstAudioVqe->bAiAecEnable)
                    {
                        pstAiVqeTalkAttr->u32OpenMask |= (LP_AEC_ENABLE | NLP_AES_ENABLE | DCREMOVER_ENABLE);
                        pstAiVqeTalkAttr->stAecCfg    = pstAudioVqe->mAiAecCfg;
                    }
                    pstAiVqeTalkAttr->s32WorkSampleRate = pstAudioCfg->enSamplerate;
                    pstAiVqeTalkAttr->para_notch_freq   = 0;

                    s32Ret = CVI_AI_SetTalkVqeAttr(pstAudioCfg->u32AiDevId, i, pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn, pstAiVqeTalkAttr);
                    if (s32Ret != CVI_SUCCESS)
                    {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_SetTalkVqeAttr failed with %#x!\n", s32Ret);
                    }
                    else
                    {
                        s32Ret = CVI_AI_EnableVqe(pstAudioCfg->u32AiDevId, i);
                        if (s32Ret != CVI_SUCCESS)
                        {
                            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_EnableVqe failed with %#x!\n", s32Ret);
                        }
                        else
                        {
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "openmask[0x%x]\n", pstAiVqeTalkAttr->u32OpenMask);
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "s32WorkSampleRate[%d]\n", pstAiVqeTalkAttr->s32WorkSampleRate);
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "AGC PARAM---------------------------------------------\n");
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "para_agc_max_gain[%d]\n", pstAiVqeTalkAttr->stAgcCfg.para_agc_max_gain);
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "para_agc_target_high[%d]\n", pstAiVqeTalkAttr->stAgcCfg.para_agc_target_high);
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "para_agc_target_low[%d]\n", pstAiVqeTalkAttr->stAgcCfg.para_agc_target_low);
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "ANR PARAM---------------------------------------------\n");
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "para_nr_snr_coeff[%d]\n", pstAiVqeTalkAttr->stAnrCfg.para_nr_snr_coeff);
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "AEC PARAM---------------------------------------------\n");
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "para_aec_filter_len[%d]\n", pstAiVqeTalkAttr->stAecCfg.para_aec_filter_len);
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "para_aes_std_thrd[%d]\n", pstAiVqeTalkAttr->stAecCfg.para_aes_std_thrd);
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "para_aes_supp_coeff[%d]\n", pstAiVqeTalkAttr->stAecCfg.para_aes_supp_coeff);
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "Start Ai VQE!\n");
                        }
                    }
                }
            }
            if (0 != pstAudioCfg->enReSamplerate)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_EnableReSmp !\n");
                s32Ret = CVI_AI_EnableReSmp(pstAudioCfg->u32AiDevId, i, pstAudioCfg->enReSamplerate);
                if (s32Ret != CVI_SUCCESS)
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_EnableReSmp failed with %#x!\n", s32Ret);
                    return s32Ret;
                }
            }
        }
    }

    s32Ret = CVI_AI_Enable(pstAudioCfg->u32AiDevId);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_Enable failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    //malloc g_pCryBuffer
    {
        #ifdef AI_BABYCRY_SUPPORT
        g_pCryBuffer = malloc(pstAudioCfg->enSamplerate * (CVI_U32)(pstAudioCfg->enBitwidth + 1) * AI_BUFFER_SECOND) ;
        if (NULL == g_pCryBuffer)
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "g_pCryBuffer malloc failed!\n");
        #endif
    }

    pthread_attr_t pthread_attr;
    pthread_attr_init(&pthread_attr);
    memset(&mAudioAiThread, 0, sizeof(mAudioAiThread));
    mAudioAiThread.bRun_flag = 1;
    pfp_task_entry fun_entry = Thread_AudioAi_Proc;
    s32Ret = pthread_create(
                    &mAudioAiThread.mRun_PID,
                    &pthread_attr,
                    fun_entry,
                    (CVI_VOID *)pstAudioCfg);
    if (s32Ret)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "audio ai pthread_create failed:0x%x\n", s32Ret);
        return CVI_FAILURE;
    }

    g_stAudioStatus.bAiInit = 1;
    APP_PROF_LOG_PRINT(LEVEL_WARN, "Start Ai OK!\n");
    return s32Ret;
}

APP_PARAM_AUDIO_CFG_T *app_ipcam_Audio_Param_Get(void)
{
    return g_pstAudioCfg;
}

/*****************************************************************
 *  The following API for command test used             Front
 * **************************************************************/
static int app_ipcam_CmdTask_AudioAttr_Parse(
    CVI_MQ_MSG_t *msg,
    APP_PARAM_AUDIO_CFG_T *pstAudioCfg,
    AUDIO_NEED_STOP_MODULE_E *penStopModule)
{
    if (NULL == msg || NULL == pstAudioCfg || NULL == penStopModule)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "msg/pstAudioCfg/penStopModule is NULL!\n");
        return -1;
    }
    CVI_CHAR param[512] = {0};
    snprintf(param, sizeof(param), "%s", msg->payload);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s param:%s\n", __FUNCTION__, param);

    memcpy(pstAudioCfg, &g_stAudioCfg, sizeof(APP_PARAM_AUDIO_CFG_T));

    CVI_CHAR *temp = strtok(param, ":");
    while (NULL != temp)
    {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s switch case -> %c \n", __FUNCTION__, *temp);
        switch (*temp)
        {
            case 'i':
            {
                temp = strtok(NULL, "/");
                pstAudioCfg->astAudioCfg.u32ChnCnt = atoi(temp);
                *penStopModule = AUDIO_NEED_STOP_ALL;
                break;
            }
            case 'm':
            {
                temp = strtok(NULL, "/");
                pstAudioCfg->astAudioCfg.enSoundmode = atoi(temp);
                *penStopModule = AUDIO_NEED_STOP_ALL;
                break;
            }
            case 'c':
            {
                temp = strtok(NULL, "/");
                pstAudioCfg->astAudioCfg.enAencType = atoi(temp);
                *penStopModule = AUDIO_NEED_STOP_ALL;
                break;
            }
            case 'f':
            {
                temp = strtok(NULL, "/");
                if (*temp & (1 << AUDIO_VQE_FUNCTION_AGC))
                {
                    pstAudioCfg->astAudioVqe.bAiAgcEnable = 1;
                }
                else
                {
                    pstAudioCfg->astAudioVqe.bAiAgcEnable = 0;
                }

                if (*temp & (1 << AUDIO_VQE_FUNCTION_ANR))
                {
                    pstAudioCfg->astAudioVqe.bAiAnrEnable = 1;
                }
                else
                {
                    pstAudioCfg->astAudioVqe.bAiAnrEnable = 0;
                }

                if (*temp & (1 << AUDIO_VQE_FUNCTION_AEC))
                {
                    pstAudioCfg->astAudioVqe.bAiAecEnable = 1;
                }
                else
                {
                    pstAudioCfg->astAudioVqe.bAiAecEnable = 0;
                }
                *penStopModule = AUDIO_NEED_STOP_ALL;
                break;
            }
            case 's':
            {
                temp = strtok(NULL, "/");
                pstAudioCfg->astAudioCfg.enSamplerate = atoi(temp);
                *penStopModule = AUDIO_NEED_STOP_ALL;
                break;
            }
            default:
                return 0;
                break;
        }
        temp = strtok(NULL, ":");
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "enType=%d enSamplerate=%d ChnCnt:%d Soundmode:%d Agc=%d Anr=%d Aec=%d\n",
        pstAudioCfg->astAudioCfg.enAencType, pstAudioCfg->astAudioCfg.enSamplerate,
        pstAudioCfg->astAudioCfg.u32ChnCnt,  pstAudioCfg->astAudioCfg.enSoundmode,
        pstAudioCfg->astAudioVqe.bAiAgcEnable, pstAudioCfg->astAudioVqe.bAiAnrEnable, pstAudioCfg->astAudioVqe.bAiAecEnable);

    return CVI_SUCCESS;
}

static int app_ipcam_Audio_IntercomStart(char *pstTargetIp, int iTargetPort)
{
    CVI_S32 s32Ret = CVI_FAILURE;
    if (NULL == pstTargetIp || strlen(pstTargetIp) == 0 || iTargetPort > 65535)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstTargetIp/iTargetPort invalid!\n");
        return s32Ret;
    }

    if (gst_SocketFd > 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Interom already Start!\n");
        return CVI_SUCCESS;
    }

    gst_SocketFd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (gst_SocketFd < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "socket create failed!!\n");
        return s32Ret;
    }

    int on = 1;
    setsockopt(gst_SocketFd, SOL_SOCKET, SO_REUSEADDR, &on,sizeof(on));

    //get ip address
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
    s32Ret = ioctl(gst_SocketFd, SIOCGIFADDR, &ifr);
    if (s32Ret < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "SIOCGIFADDR failed!!\n");
        close(gst_SocketFd);
        gst_SocketFd = -1;
        return s32Ret;
    }

    char local_ip[16];
    memset(local_ip, 0, sizeof(local_ip));
    strncpy(local_ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), sizeof(local_ip));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(iTargetPort);
    server_addr.sin_addr.s_addr = inet_addr(local_ip);

    s32Ret = bind(gst_SocketFd, (struct sockaddr *)&server_addr,sizeof(server_addr));
    if (s32Ret < 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "bind failed!!\n");
        close(gst_SocketFd);
        gst_SocketFd = -1;
        return s32Ret;
    }

    memset(&gst_TargetAddr, 0, sizeof(gst_TargetAddr));
    gst_TargetAddr.sin_family      = AF_INET;
    gst_TargetAddr.sin_port        = htons(iTargetPort);
    gst_TargetAddr.sin_addr.s_addr = inet_addr(pstTargetIp);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Audio_IntercomStart OK!!\n");
    return s32Ret;
}

static void app_ipcam_Audio_IntercomStop(void)
{
    if (gst_SocketFd)
    {
        close(gst_SocketFd);
        gst_SocketFd = -1;
        memset(&gst_TargetAddr, 0, sizeof(gst_TargetAddr));
        APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Audio_IntercomStop OK!!\n");
    }
}

int app_ipcam_Audio_Init(void)
{
    int s32Ret = 0;

    APP_AUDIO_CFG_S *pstAudioCfg = &g_pstAudioCfg->astAudioCfg;
    APP_AUDIO_VQE_S *pstAudioVqe = &g_pstAudioCfg->astAudioVqe;
    APP_AUDIO_INTERCOM_T *pstAudioIntercom = &g_pstAudioCfg->astAudioIntercom;
    memset(&g_stAudioStatus, 0, sizeof(g_stAudioStatus));
    pthread_mutex_lock(&RsAudioMutex);
    memset(&g_stAudioRecord, 0, sizeof(g_stAudioRecord));
    memset(&g_stAudioPlay, 0, sizeof(g_stAudioPlay));
    pthread_mutex_unlock(&RsAudioMutex);

    if (pstAudioIntercom->bEnable)
    {
        s32Ret = app_ipcam_Audio_IntercomStart(pstAudioIntercom->cIp, pstAudioIntercom->iPort);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_IntercomStart failed with %#x!\n", s32Ret);
        }
    }

    s32Ret = app_ipcam_Audio_ParamChcek(pstAudioCfg, pstAudioVqe);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_ParamChcek failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AUDIO_INIT();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AUDIO_INIT failed with %#x!\n", s32Ret);
    }

    s32Ret = app_ipcam_Audio_CfgAcodec();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_CfgAcodec failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = app_ipcam_Audio_AiStart(pstAudioCfg, pstAudioVqe);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_AiStart failed with %#x!\n", s32Ret);
        goto AUDIO_EXIT;
    }

    s32Ret = app_ipcam_Audio_AoStart(pstAudioCfg, pstAudioVqe);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_AoStart failed with %#x!\n", s32Ret);
        goto AUDIO_EXIT;
    }

    s32Ret = app_ipcam_Audio_AencStart(pstAudioCfg);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_AencStart failed with %#x!\n", s32Ret);
        goto AUDIO_EXIT;
    }

    s32Ret = app_ipcam_Audio_AdecStart(pstAudioCfg, pstAudioVqe);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_AdecStart failed with %#x!\n", s32Ret);
        goto AUDIO_EXIT;
    }


AUDIO_EXIT:
    if (s32Ret != CVI_SUCCESS)
    {
        s32Ret = app_ipcam_Audio_UnInit();
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_UnInit failed with %#x!\n", s32Ret);
            goto AUDIO_EXIT;
        }
    }

    g_pstAudioCfg->bInit = 1;
    return 0;
}

int app_ipcam_Audio_UnInit(void)
{
    int s32Ret = 0;

    APP_AUDIO_CFG_S *pstAudioCfg = &g_pstAudioCfg->astAudioCfg;
    APP_AUDIO_VQE_S *pstAudioVqe = &g_pstAudioCfg->astAudioVqe;
    pthread_mutex_lock(&RsAudioMutex);
    memset(&g_stAudioRecord, 0, sizeof(g_stAudioRecord));
    memset(&g_stAudioPlay, 0, sizeof(g_stAudioPlay));
    pthread_mutex_unlock(&RsAudioMutex);

    s32Ret = app_ipcam_Audio_AoStop(pstAudioCfg, pstAudioVqe);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_DestoryAo failed!\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Audio_AdecStop(pstAudioCfg);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_AdecStop failed!\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Audio_AencStop(pstAudioCfg);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_DestoryAenc failed!\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Audio_AiStop(pstAudioCfg, pstAudioVqe);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_DestoryAi failed!\n");
        return s32Ret;
    }
    /*s32Ret = CVI_AUDIO_DEINIT();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AUDIO_DEINIT failed with %#x!\n", s32Ret);
    }*/
    app_ipcam_Audio_IntercomStop();

    g_pstAudioCfg->bInit = 0;
    return s32Ret;
}

int app_ipcam_Audio_AoPlay(char *pAudioFile, AUDIO_AO_PLAY_TYPE_E eAoType)
{
    int s32Ret = 0;

    if (NULL == pAudioFile)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pAudioFile is NULL!\n");
        sleep(1);
        return -1;
    }

    if (g_stAudioStatus.bAoInit == 0)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Ao no Init!\n");
        sleep(1);
        return -1;
    }

    int flen = 0;
    int offset = 0;
    CVI_U8 *pBuffer = NULL;
    AUDIO_FRAME_S stAoSendFrame;
    APP_AUDIO_CFG_S *pstAudioCfg = &g_pstAudioCfg->astAudioCfg;
    FILE *fp = NULL;

    if (AUDIO_AO_PLAY_TYPE_INTERCOM != eAoType)
    {
        fp = fopen(pAudioFile, "r");
        if (NULL == fp)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "fopen %s fail!\n", pAudioFile);
            return CVI_FAILURE;
        }

        fseek(fp, 0, SEEK_END);
        flen = ftell(fp);
        rewind(fp);
        if ((flen <= 0) || (flen > AUDIO_OVERSIZE))
        {
            fclose(fp);
            fp = NULL;
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Invalid audio file!size1:%d!\n", flen);
            return CVI_FAILURE;
        }

        pBuffer = (CVI_U8 *)malloc(flen + 1);
        if (pBuffer == NULL)
        {
            fclose(fp);
            fp = NULL;
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc pBuffer failed\n");
            return CVI_FAILURE;
        }
        memset(pBuffer, 0, flen + 1);
        fread(pBuffer, 1, flen, fp);
        fclose(fp);
        fp = NULL;
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Open audio file!Len:%d!\n", flen);
    }
    else
    {
        if (gst_SocketFd <= 0)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "gst_SocketFd failed!\n");
            return CVI_FAILURE;
        }

        int tmpLen = (pstAudioCfg->enAencType == PT_AAC) ? 1024 : ADEC_MAX_FRAME_NUMS;
        pBuffer = (CVI_U8 *)malloc(tmpLen);
        if (pBuffer == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc pBuffer failed\n");
            return CVI_FAILURE;
        }
        memset(pBuffer, 0, tmpLen);
        unsigned int addrlen = sizeof(gst_TargetAddr);
        flen = recvfrom(gst_SocketFd, pBuffer, tmpLen, 0,
            (struct sockaddr *)&gst_TargetAddr, &addrlen);
        if (flen <= 0)
        {
            free(pBuffer);
            pBuffer = NULL;
            sleep(1);
            return CVI_FAILURE;
        }
    }

#ifdef MP3_SUPPORT
    void *pMp3DecHandler = NULL;
#endif
    if (eAoType == AUDIO_AO_PLAY_TYPE_MP3)
    {
#ifdef MP3_SUPPORT
        //step1:start init
        pMp3DecHandler = CVI_MP3_Decode_Init(NULL);
        if (pMp3DecHandler == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_MP3_Decode_Init failed!\n");
            return CVI_FAILURE;
        }
        //optional:user call also get the data from call back function
        s32Ret = CVI_MP3_Decode_InstallCb(pMp3DecHandler, app_ipcam_Audio_MP3CB);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_MP3_Decode_InstallCb failed with %#x!\n", s32Ret);
            return CVI_FAILURE;
        }
#endif
    }

    do
    {
        if (eAoType == AUDIO_AO_PLAY_TYPE_AENC || eAoType == AUDIO_AO_PLAY_TYPE_INTERCOM)
        {
            if (g_stAudioStatus.bAdecInit == 0)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Adec no Init!\n");
                sleep(1);
                return 0;
            }
            AUDIO_STREAM_S stAudioStream;
            memset(&stAudioStream, 0, sizeof(AUDIO_STREAM_S));
            if (flen > ADEC_MAX_FRAME_NUMS)
            {
                if (pstAudioCfg->enAencType == PT_AAC)
                {
                    if (flen < 1024) {
                        stAudioStream.u32Len = flen;
                    } else {
                        stAudioStream.u32Len = 1024; //aac len must be 1024
                    }
                }
                else
                {
                    stAudioStream.u32Len = ADEC_MAX_FRAME_NUMS; //testing for g726 //hi set the adec default length= 320
                }
            }
            else
            {
                stAudioStream.u32Len = flen;
            }
            stAudioStream.pStream = pBuffer + offset;

            /* here only demo adec streaming sending mode, but pack sending mode is commended */
            s32Ret = CVI_ADEC_SendStream(pstAudioCfg->u32AdChn, &stAudioStream, CVI_TRUE);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ADEC_SendStream(%d) failed with %#x!\n", pstAudioCfg->u32AdChn, s32Ret);
            }
            flen -= stAudioStream.u32Len;
            offset += stAudioStream.u32Len;

            memset(&stAoSendFrame, 0x0, sizeof(AUDIO_FRAME_S));
            AUDIO_FRAME_S *pstFrame = &stAoSendFrame;
            AUDIO_FRAME_INFO_S sDecOutFrm;
            sDecOutFrm.pstFrame = (AUDIO_FRAME_S *)&stAoSendFrame;
            s32Ret = CVI_ADEC_GetFrame(pstAudioCfg->u32AdChn, &sDecOutFrm, CVI_TRUE);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ADEC_GetFrame(%d) failed with %#x!\n", pstAudioCfg->u32AdChn, s32Ret);
                break;
            }

            if (pstFrame->u32Len != 0)
            {
                s32Ret = CVI_AO_SendFrame(pstAudioCfg->u32AoChn, pstAudioCfg->u32AdChn, pstFrame, 5000);
                if (s32Ret != CVI_SUCCESS)
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_SendFrame failed with %#x!\n", s32Ret);
                }
            }
            else
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "dec out frame size 0\n");
            }

            if ((flen <= 0) && (eAoType == AUDIO_AO_PLAY_TYPE_AENC))
            {
                /*s32Ret = CVI_ADEC_ClearChnBuf(pstAudioCfg->u32AdChn);
                if (s32Ret != CVI_SUCCESS)
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ADEC_ClearChnBuf failed with %#x!\n", s32Ret);
                }*/
                /* aac frame is large so sleep 3s */
                if (pstAudioCfg->enAencType == PT_AAC)
                {
                    sleep(3);
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "End Of file play!\n");
                }
                if(g_stAudioPlay.iStatus)
                    g_stAudioPlay.iStatus = 0;

                break;
            }
        }
        else if (eAoType == AUDIO_AO_PLAY_TYPE_RAW)
        {
            int iAudioLen = 0;
            memset(&stAoSendFrame, 0x0, sizeof(AUDIO_FRAME_S));
            if (flen > AO_MAX_FRAME_NUMS)
            {
                iAudioLen = AO_MAX_FRAME_NUMS;
            }
            else
            {
                iAudioLen = flen;
            }
            //audio is 16bit=2byte so framelen div 2
            stAoSendFrame.u32Len = iAudioLen / 2;
            stAoSendFrame.u64VirAddr[0] = pBuffer + offset;
            stAoSendFrame.u64TimeStamp = 0;
            stAoSendFrame.enSoundmode = pstAudioCfg->enSoundmode;
            stAoSendFrame.enBitwidth = pstAudioCfg->enBitwidth;
            s32Ret = CVI_AO_SendFrame(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn, (const AUDIO_FRAME_S *)&stAoSendFrame, 1000);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_SendFrame failed with %#x!\n", s32Ret);
            }
            flen -= iAudioLen;
            offset += iAudioLen;
        }
        else if (eAoType == AUDIO_AO_PLAY_TYPE_MP3)
        {
#ifdef MP3_SUPPORT
            AUDIO_STREAM_S stAudioStream;
            memset(&stAudioStream, 0, sizeof(AUDIO_STREAM_S));
            if (flen > AO_MAX_FRAME_NUMS)
            {
                stAudioStream.u32Len = AO_MAX_FRAME_NUMS;
            }
            else
            {
                stAudioStream.u32Len = flen;
            }
            stAudioStream.pStream = pBuffer + offset;

            CVI_U32 s32OutLen = 0;
            CVI_U8 *pOutPutBuffer = (CVI_U8 *)malloc(stAudioStream.u32Len * 20);
            if (NULL == pOutPutBuffer)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"pOutPutBuffer malloc failed with %#x!\n", s32Ret);
            }
            else
            {
                /* here only demo adec streaming sending mode, but pack sending mode is commended */
                memset(pOutPutBuffer, 0, stAudioStream.u32Len * 20);
                s32Ret = CVI_MP3_Decode((CVI_VOID *)pMp3DecHandler,
                            (CVI_VOID *)stAudioStream.pStream,
                            (CVI_VOID *)pOutPutBuffer,
                            (CVI_S32)stAudioStream.u32Len,
                            (CVI_S32 *)&s32OutLen);
                if (s32Ret != CVI_SUCCESS)
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_MP3_Decode failed with %#x!\n", s32Ret);
                }
                if (s32OutLen > 0)
                {
                    if (s32OutLen  > (stAudioStream.u32Len * 20))
                    {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Error outbuf size is not enough [%d] < [%d]\n", (stAudioStream.u32Len * 20), s32OutLen);
                    }
                    else
                    {
                        int stMP3Offset = 0;
                        do
                        {
                            int iAudioLen = 0;
                            memset(&stAoSendFrame, 0x0, sizeof(AUDIO_FRAME_S));
                            if (s32OutLen > AO_MAX_FRAME_NUMS)
                            {
                                iAudioLen = AO_MAX_FRAME_NUMS;
                            }
                            else
                            {
                                iAudioLen = s32OutLen;
                            }
                            //audio is 16bit=2byte so framelen div 2
                            stAoSendFrame.u32Len = iAudioLen / 2;
                            stAoSendFrame.u64VirAddr[0] = (pOutPutBuffer + stMP3Offset);
                            stAoSendFrame.u64TimeStamp = 0;
                            stAoSendFrame.enSoundmode = pstAudioCfg->enSoundmode;
                            stAoSendFrame.enBitwidth = pstAudioCfg->enBitwidth;
                            s32Ret = CVI_AO_SendFrame(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn, (const AUDIO_FRAME_S *)&stAoSendFrame, 1000);
                            if (s32Ret != CVI_SUCCESS)
                            {
                                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_SendFrame failed with %#x!\n", s32Ret);
                            }
                            s32OutLen -= iAudioLen;
                            stMP3Offset += iAudioLen;
                        } while (s32Ret != CVI_SUCCESS || s32OutLen > 0);
                    }
                }
                flen -= stAudioStream.u32Len;
                offset += stAudioStream.u32Len;
                free(pOutPutBuffer);
                pOutPutBuffer = NULL;
            }
#endif
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupport eAotype:%#x!\n", eAoType);
            s32Ret = CVI_FALSE;
        }
    } while (s32Ret != CVI_SUCCESS || flen > 0);

    if (eAoType == AUDIO_AO_PLAY_TYPE_MP3)
    {
#ifdef MP3_SUPPORT
        CVI_MP3_Decode_DeInit(pMp3DecHandler);
#endif
    }

    if (pBuffer)
    {
        free(pBuffer);
        pBuffer = NULL;
    }
    return s32Ret;
}

int app_ipcam_CmdTask_AudioAttr_Set(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    if (NULL == msg)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "msg is NULL!\n");
        return -1;
    }

    APP_PARAM_AUDIO_CFG_T stAudioCfg;
    AUDIO_NEED_STOP_MODULE_E enStopModule = AUDIO_NEED_STOP_NULL;

    APP_CHK_RET(app_ipcam_CmdTask_AudioAttr_Parse(msg, &stAudioCfg, &enStopModule), "Audio Attr Parse");
    if (enStopModule & (1 << AUDIO_NEED_STOP_NULL))
    {
        APP_CHK_RET(app_ipcam_Audio_AudioReset(&stAudioCfg), "Stop Audio");
    }

    return CVI_SUCCESS;
}

int app_ipcam_CmdTask_Mp3_Play(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    CVI_CHAR param[512] = {0};
    snprintf(param, sizeof(param), "%s", msg->payload);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s param:%s arg2=%d\n", __FUNCTION__, param, msg->arg2);

    CVI_CHAR *temp = strtok(param, ":");
    while(NULL != temp) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s switch case -> %c \n", __FUNCTION__, *temp);
        switch (*temp) {
            case 's':
                {
                    temp = strtok(NULL, "/");
                    CVI_BOOL bEnable = (CVI_BOOL)atoi(temp);
                    if (bEnable) {
                        if(!MP3_RUNNING)
                        {
                            MP3_RUNNING = CVI_TRUE;
                        }else{
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "mp3 play is running ,please wait!\n");
                        }
                    } else {
                        MP3_RUNNING = CVI_FALSE;
                    }
                }
                break;
            default:
                return 0;
                break;
        }

        temp = strtok(NULL, ":");
    }
    return CVI_SUCCESS;
}
/*****************************************************************
 *  The above API for command test used                 End
 * **************************************************************/

int app_ipcam_Audio_AudioReset(APP_PARAM_AUDIO_CFG_T *pstAudioCfg)
{
    if (NULL == pstAudioCfg)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioCfg is NULL!\n");
        return -1;
    }
    int s32Ret = 0;
    if (memcmp(pstAudioCfg, &g_stAudioCfg, sizeof(APP_PARAM_AUDIO_CFG_T)))
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Start Audio Reset!\n");
        if ((g_stAudioCfg.astAudioCfg.u32ChnCnt > 1) &&
            (0 == g_stAudioCfg.astAudioVqe.bAiAecEnable) &&
            (pstAudioCfg->astAudioCfg.enSamplerate != g_stAudioCfg.astAudioCfg.enSamplerate)) {
            //TODO reset rtsp need add lock, but venc get stream can be influence
            //bRtspReset = 1;
        }
        s32Ret = app_ipcam_Audio_UnInit();
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_UnInit failed with %#x!\n", s32Ret);
        }

        s32Ret = app_ipcam_Audio_ParamChcek(&pstAudioCfg->astAudioCfg, &pstAudioCfg->astAudioVqe);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_ParamChcek failed with %#x!\n", s32Ret);
            return s32Ret;
        }
        memcpy(&g_stAudioCfg, pstAudioCfg, sizeof(APP_PARAM_AUDIO_CFG_T));

        if (pstAudioCfg->bInit)
        {
            s32Ret = app_ipcam_Audio_Init();
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Audio_Init failed with %#x!\n", s32Ret);
            }
        }

        APP_PROF_LOG_PRINT(LEVEL_WARN, "Start Audio Reset OK!\n");
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Audio Config No Change!\n");
    }
    return s32Ret;
}

int app_ipcam_Audio_SetRecordStatus(APP_AUDIO_RECORD_T *pstAudioRecord)
{
    if (NULL == pstAudioRecord)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioRecord is NULL!\n");
        return -1;
    }

    pthread_mutex_lock(&RsAudioMutex);
    if (memcmp(&g_stAudioRecord, pstAudioRecord, sizeof(APP_AUDIO_RECORD_T)) != 0)
    {
        memcpy(&g_stAudioRecord, pstAudioRecord, sizeof(APP_AUDIO_RECORD_T));
    }
    pthread_mutex_unlock(&RsAudioMutex);
    return 0;
}

int app_ipcam_Audio_SetPlayStatus(APP_AUDIO_RECORD_T *pstAudioPlay)
{
    if (NULL == pstAudioPlay)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pstAudioPlay is NULL!\n");
        return -1;
    }

    APP_AUDIO_CFG_S *pstAudioCfg = &g_pstAudioCfg->astAudioCfg;
    pthread_mutex_lock(&RsAudioMutex);
    if (memcmp(&g_stAudioPlay, pstAudioPlay, sizeof(APP_AUDIO_RECORD_T)) != 0)
    {
        memcpy(&g_stAudioPlay, pstAudioPlay, sizeof(APP_AUDIO_RECORD_T));
        if (CVI_AO_DisableChn(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn) != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_DisableChn(%d) failed!\n", pstAudioCfg->u32AoChn);
        }
        usleep(100 * 1000);
        if (CVI_AO_EnableChn(pstAudioCfg->u32AoDevId, pstAudioCfg->u32AoChn) != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AO_EnableChn(%d) failed!\n", pstAudioCfg->u32AoChn);
        }
    }
    pthread_mutex_unlock(&RsAudioMutex);
    return 0;
}

