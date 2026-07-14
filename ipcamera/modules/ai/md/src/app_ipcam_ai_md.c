#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "app_ipcam_ai.h"
#include "tdl_sdk.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

SMT_MUTEXAUTOLOCK_INIT(g_MDMutex);
static pthread_mutex_t g_MdStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_MD_CFG_S g_stMdCfg;
static APP_PARAM_AI_MD_CFG_S *g_pstMdCfg = &g_stMdCfg;

static volatile CVI_U32 g_MDThreshold = 0;
static CVI_U32 g_MDFps;
static CVI_U32 g_MDProc;
static volatile bool g_bMDRunning = CVI_FALSE;
static volatile bool g_bMDPause = CVI_FALSE;
static pthread_t g_MDThreadHandle;
static TDLHandle g_MDHandle = NULL;
static TDLObject g_stMDObjDraw;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_MD_CFG_S *app_ipcam_Ai_MD_Param_Get(void)
{
    return g_pstMdCfg;
}

CVI_VOID app_ipcam_Ai_MD_Thresold_Set(CVI_U32 value)
{
    g_MDThreshold = value;
}

CVI_U32 app_ipcam_Ai_MD_Thresold_Get(void)
{
    return g_MDThreshold;
}

CVI_VOID app_ipcam_Ai_MD_ProcStatus_Set(CVI_BOOL flag)
{
    g_bMDRunning = flag;
}

CVI_BOOL app_ipcam_Ai_MD_ProcStatus_Get(void)
{
    return g_bMDRunning;
}

CVI_VOID app_ipcam_Ai_MD_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_MdStatusMutex);
    g_bMDPause = flag;
    pthread_mutex_unlock(&g_MdStatusMutex);
}

CVI_BOOL app_ipcam_Ai_MD_Pause_Get(void)
{
    return g_bMDPause;
}

CVI_U32 app_ipcam_Ai_MD_ProcFps_Get(void)
{
    return g_MDFps;
}

CVI_S32 app_ipcam_Ai_MD_ProcTime_Get(void)
{
    return g_MDProc;
}

static void app_ipcam_Ai_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n", 
        g_pstMdCfg->bEnable, g_pstMdCfg->VpssGrp, g_pstMdCfg->VpssChn, g_pstMdCfg->u32GrpWidth, g_pstMdCfg->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "threshold=%d miniArea=%d u32BgUpPeriod=%d\n",
        g_pstMdCfg->threshold, g_pstMdCfg->miniArea, g_pstMdCfg->u32BgUpPeriod);

}

/**
 * 深拷贝 TDLObject 结构体
 * @param dst 目标对象（需预先分配内存）
 * @param src 源对象
 * @return 0成功，-1失败
 */
static int deep_copy_tdl_object(TDLObject* dst, const TDLObject* src) {
    // 1. 参数检查
    if (dst == NULL || src == NULL) {
        return -1;
    }

    // 2. 拷贝基本成员
    dst->size = src->size;
    dst->width = src->width;
    dst->height = src->height;

    // 3. 处理 TDLObjectInfo 数组
    if (src->info != NULL && src->size > 0) {
        // 分配新数组内存
        dst->info = (TDLObjectInfo*)malloc(src->size * sizeof(TDLObjectInfo));
        if (dst->info == NULL) {
            return -1;
        }

        // 逐个拷贝对象信息
        for (uint32_t i = 0; i < src->size; i++) {
            // 拷贝基本成员
            dst->info[i].box.x1 = src->info[i].box.x1;
            dst->info[i].box.x2 = src->info[i].box.x2;
            dst->info[i].box.y1 = src->info[i].box.y1;
            dst->info[i].box.y2 = src->info[i].box.y2;
            // dst->info[i].score = src->info[i].score;
            // dst->info[i].class_id = src->info[i].class_id;
            // dst->info[i].landmark_size = src->info[i].landmark_size;
            // dst->info[i].obj_type = src->info[i].obj_type;
            
            // landmark_properity 设为 NULL（不拷贝原数据）
            dst->info[i].landmark_properity = NULL;
        }
    } else {
        return 0;
    }

    return 0;
}

static CVI_S32 app_ipcam_Ai_MD_Proc_Init()
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI MD init ------------------> start \n");

    if (g_MDHandle != NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Ai MD Proc Init but handle is not NULL!\n");
        return CVI_SUCCESS;
    }

    app_ipcam_Ai_Param_dump();

    g_MDHandle = TDL_CreateHandle(0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "MD CreateHandle failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI MD init ------------------> done \n");

    return CVI_SUCCESS;
}

static CVI_VOID *Thread_MD_Proc(CVI_VOID *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_CHAR TaskName[64] = {'\0'};
    sprintf(TaskName, "Thread_MD_Proc");
    prctl(PR_SET_NAME, TaskName, 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI MD start running!\n");

    CVI_U32 md_frame = 0;
    CVI_S32 iTime_start, iTime_proc,iTime_fps;
    float iTime_gop;
    iTime_start = GetCurTimeInMsec();

    TDLObject roi = {0};
    roi.size = 1;
    roi.info = (TDLObjectInfo *)malloc(sizeof(TDLObjectInfo));
    if (roi.info == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to allocate memory for roi.info\n");
    }
    roi.info[0].box.x1 = 0;
    roi.info[0].box.y1 = 0;
    roi.info[0].box.x2 = g_pstMdCfg->u32GrpWidth - 1;
    roi.info[0].box.y2 = g_pstMdCfg->u32GrpHeight - 1;

    CVI_U32 count = 0;
    CVI_U32 u32BgUpPeriod = g_pstMdCfg->u32BgUpPeriod;

    CVI_U32 miniArea = g_pstMdCfg->miniArea;
    app_ipcam_Ai_MD_Thresold_Set(g_pstMdCfg->threshold);
    TDLObject obj_meta;
    memset(&obj_meta, 0, sizeof(TDLObject));
    
    VIDEO_FRAME_INFO_S stVencFrame_back;
    memset(&stVencFrame_back, 0, sizeof(VIDEO_FRAME_INFO_S));
    VIDEO_FRAME_INFO_S stVencFrame_det;
    memset(&stVencFrame_det, 0, sizeof(VIDEO_FRAME_INFO_S));

    TDLImage image_back, image_det;

    VPSS_GRP VpssGrp = g_pstMdCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstMdCfg->VpssChn;

    while (app_ipcam_Ai_MD_ProcStatus_Get()) {
        pthread_mutex_lock(&g_MdStatusMutex);
        s32Ret = app_ipcam_Ai_MD_Pause_Get();
        
        if (s32Ret) {
            pthread_mutex_unlock(&g_MdStatusMutex);
            usleep(1000*1000);
            continue;
        }
        
        if ((count % u32BgUpPeriod) == 0)   // 更新背景图
        {
            APP_PROF_LOG_PRINT(LEVEL_TRACE, "update BG interval=%d, threshold=%d, miniArea=%d\n",
            u32BgUpPeriod, g_MDThreshold, miniArea);
            s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVencFrame_back, 3000);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
                pthread_mutex_unlock(&g_MdStatusMutex);
                usleep(100*1000);
                continue;
            }
            image_back = TDL_WrapFrame((void*)&stVencFrame_back, false, false);
        }
        pthread_mutex_unlock(&g_MdStatusMutex);
        iTime_proc = GetCurTimeInMsec();

        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVencFrame_det, 3000);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        image_det = TDL_WrapFrame((void*)&stVencFrame_det, false, false);

        TDL_MotionDetection(g_MDHandle, image_back, image_det, &roi, g_MDThreshold, miniArea, &obj_meta, 0);
        // for(uint32_t i = 0; i < obj_meta.size; i++)
        // {
        //     printf( "++++++++++++++ MD obj: %d, box: (%f, %f, %f, %f)\n", 
        //         i+1, obj_meta.info[i].box.x1, obj_meta.info[i].box.y1, 
        //         obj_meta.info[i].box.x2, obj_meta.info[i].box.y2);
        // }

        g_MDProc = GetCurTimeInMsec() - iTime_proc;
        APP_PROF_LOG_PRINT(LEVEL_TRACE, "MD process takes %d\n", g_MDProc);
        md_frame ++;
        iTime_fps = GetCurTimeInMsec();
        iTime_gop = (float)(iTime_fps - iTime_start)/1000;
        if(iTime_gop >= 1)
        {
            g_MDFps = md_frame/iTime_gop;
            md_frame = 0;
            iTime_start = iTime_fps;
        }      

        if (obj_meta.size == 0 || obj_meta.info == NULL) {
            TDL_ReleaseObjectMeta(&obj_meta);
            s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVencFrame_det);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            }
            TDL_DestroyImage(image_det);

            if ((count % u32BgUpPeriod) == (u32BgUpPeriod - 1) )
            {
                s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVencFrame_back);     
                if (s32Ret != CVI_SUCCESS)
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
                }
                TDL_DestroyImage(image_back);
            }

            count = (count == u32BgUpPeriod) ? (1) : (count+1); // 计数+1
            continue;
        }
        SMT_MutexAutoLock(g_MDMutex, lock);
        if (g_stMDObjDraw.info != NULL) {
            TDL_ReleaseObjectMeta(&g_stMDObjDraw);
        }
        memset(&g_stMDObjDraw, 0, sizeof(TDLObject));
        deep_copy_tdl_object(&g_stMDObjDraw, &obj_meta);
        TDL_ReleaseObjectMeta(&obj_meta);

        if ((count % u32BgUpPeriod) == (u32BgUpPeriod - 1) )
        {
            s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVencFrame_back);     
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            }
            TDL_DestroyImage(image_back);
        }
        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVencFrame_det);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        TDL_DestroyImage(image_det);
        count = (count == u32BgUpPeriod) ? (1) : (count+1);     // 计数+1
    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_MD_ObjDrawInfo_Get(TDLObject *pstMdObj)
{
    _NULL_POINTER_CHECK_(pstMdObj, -1);

    SMT_MutexAutoLock(g_MDMutex, lock);

    if (g_stMDObjDraw.size == 0) {
        return CVI_SUCCESS;
    } else {
        memset(pstMdObj, 0, sizeof(TDLObject));
        deep_copy_tdl_object(pstMdObj, &g_stMDObjDraw);
        if (g_stMDObjDraw.info != NULL) { 
            TDL_ReleaseObjectMeta(&g_stMDObjDraw);
        }
    }
    return CVI_SUCCESS;
}

int app_ipcam_Ai_MD_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstMdCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI MD not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_MD_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI MD has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_MD_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_MDThreadHandle)
    {
        pthread_join(g_MDThreadHandle, NULL);
        g_MDThreadHandle = 0;
    }

    s32Ret = TDL_DestroyHandle(g_MDHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_MD_Destroy_Handle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_MDHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI MD Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

int app_ipcam_Ai_MD_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstMdCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI MD not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bMDRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI MD has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_MD_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_MD_Proc_Init failed with error %d\n", s32Ret);
        return s32Ret;
    }

    app_ipcam_Ai_MD_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_MDThreadHandle, NULL, Thread_MD_Proc, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Thread_Motion_Detecte failed with error %d\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

/*****************************************************************
 *  The following API for command test used             Front
 * **************************************************************/
CVI_S32 app_ipcam_Ai_MD_StatusGet(void)
{
    return g_MDHandle ? 1 : 0;
}

/*****************************************************************
 *  The above API for command test used                 End
 * **************************************************************/
