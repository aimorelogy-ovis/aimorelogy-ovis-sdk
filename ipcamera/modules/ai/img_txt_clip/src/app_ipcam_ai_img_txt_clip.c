#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <dirent.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "app_ipcam_ai.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

SMT_MUTEXAUTOLOCK_INIT(g_ClipMutex);
static pthread_mutex_t g_ClipStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_IMG_TXT_CLIP_S g_stClipCfg;
static APP_PARAM_AI_IMG_TXT_CLIP_S *g_pstClipCfg = &g_stClipCfg;

static volatile bool g_bClipRunning = CVI_FALSE;
static volatile bool g_bClipPause = CVI_FALSE;
static pthread_t g_ClipThreadHandle;
static TDLHandle g_ClipAiHandle = NULL;
static float *g_ClipTextFeature = NULL;
static int32_t g_ClipNumSentences;
static int32_t g_ClipEmbeddingNum;
static char **g_txt_lines = NULL;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_IMG_TXT_CLIP_S *app_ipcam_Ai_Img_Txt_Clip_Param_Get(void)
{
    return g_pstClipCfg;
}

CVI_VOID app_ipcam_Ai_Img_Txt_Clip_ProcStatus_Set(CVI_BOOL flag)
{
    g_bClipRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Img_Txt_Clip_ProcStatus_Get(void)
{
    return g_bClipRunning;
}

CVI_VOID app_ipcam_Ai_Img_Txt_Clip_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_ClipStatusMutex);
    g_bClipPause = flag;
    pthread_mutex_unlock(&g_ClipStatusMutex);
}

CVI_BOOL app_ipcam_Ai_Img_Txt_Clip_Pause_Get(void)
{
    pthread_mutex_lock(&g_ClipStatusMutex);
    CVI_BOOL Pause = g_bClipPause;
    pthread_mutex_unlock(&g_ClipStatusMutex);
    return Pause;
}

static CVI_S32 app_ipcam_Ai_Img_Txt_Clip_Read_Input(CVI_VOID)
{
    char filepath[256];
    strcpy(filepath, g_pstClipCfg->txt_dir);
    strcat(filepath, "/input.txt");
    FILE *file = fopen(filepath, "r");
    if (!file) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "fail to open file\n");
        return CVI_FAILURE;
    }

    int32_t lineCount = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            lineCount++;
        }
    }
    // 处理最后一行可能没有换行符的情况
    if (ftell(file) > 0) {
        fseek(file, 0, SEEK_SET);
        if (fgetc(file) != EOF) lineCount++;
    }
    rewind(file);

    g_txt_lines = malloc(lineCount * sizeof(char *));
    if (!g_txt_lines) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc lines failed\n");
        fclose(file);
        return CVI_FAILURE;
    }

    char buffer[256];
    int32_t index = 0;

    while (fgets(buffer, sizeof(buffer), file) && index < lineCount) {
        buffer[strcspn(buffer, "\r\n")] = '\0';

        g_txt_lines[index] = malloc(strlen(buffer) + 1);
        if (!g_txt_lines[index]) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc lines[index] failed\n");
            break;
        }

        strcpy(g_txt_lines[index], buffer);
        index++;
    }

    fclose(file);
    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Ai_Img_Txt_Clip_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Img_Txt_Clip init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    /* 1.Creat Handle */
    if (g_ClipAiHandle == NULL)
    {
        g_ClipAiHandle = TDL_CreateHandle(0);
        if (g_ClipAiHandle == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_Img_Txt_Clip_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_Img_Txt_Clip_CreateHandle has created\n");
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_ClipAiHandle, g_pstClipCfg->model_id_img, g_pstClipCfg->model_path_img, g_pstClipCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_OpenModel failed with %#x!\n", g_pstClipCfg->model_path_img, s32Ret);
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_ClipAiHandle, g_pstClipCfg->model_id_txt, g_pstClipCfg->model_path_txt, g_pstClipCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        TDL_CloseModel(g_ClipAiHandle, g_pstClipCfg->model_id_img);
        TDL_DestroyHandle(g_ClipAiHandle);
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_OpenModel failed with %#x!\n", g_pstClipCfg->model_path_txt, s32Ret);
        return s32Ret;
    }

    s32Ret = TDL_ClipText(g_ClipAiHandle, g_pstClipCfg->model_id_txt, g_pstClipCfg->txt_dir, &g_ClipTextFeature,
                          &g_ClipNumSentences, &g_ClipEmbeddingNum);
    if (s32Ret != CVI_SUCCESS)
    {
        TDL_CloseModel(g_ClipAiHandle, g_pstClipCfg->model_id_img);
        TDL_CloseModel(g_ClipAiHandle, g_pstClipCfg->model_id_txt);
        TDL_DestroyHandle(g_ClipAiHandle);
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_ClipText failed with %#x, txt_dir = %s!\n", s32Ret, g_pstClipCfg->txt_dir);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Img_Txt_Clip init ------------------> done \n");
    return CVI_SUCCESS;
}

static CVI_VOID *Thread_Img_Txt_Clip_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Img_Txt_Clip start running!\n");
    prctl(PR_SET_NAME, "Thread_Img_Txt_Clip_PROC");

    VPSS_GRP VpssGrp = g_pstClipCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstClipCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};
    TDLImage image_handle;
    TDLFeature feature = {0};

    while (app_ipcam_Ai_Img_Txt_Clip_ProcStatus_Get()) {
        if (app_ipcam_Ai_Img_Txt_Clip_Pause_Get()) {
            usleep(1000*1000);
            continue;
        }
        /* 1.Get frame */
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 3000);
        if (s32Ret != CVI_SUCCESS){
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            pthread_mutex_unlock(&g_ClipStatusMutex);
            usleep(100*1000);
            continue;
        }
        image_handle = TDL_WrapFrame((void*)&stfdFrame, false, false);

        /* 2. Feature Detect*/
        TDL_FeatureExtraction(g_ClipAiHandle, g_pstClipCfg->model_id_img, image_handle, &feature);
        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        TDL_DestroyImage(image_handle);

        float *image_feature = (float *)(feature.ptr);
        if(image_feature) {
            float *result = NULL;
            s32Ret = TDL_ClipPostprocess(g_ClipTextFeature, g_ClipNumSentences, image_feature, 1, g_ClipEmbeddingNum, &result);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_ClipPostprocess failed with %#x\n", s32Ret);
            } else {
                for (int32_t i = 0; i < g_ClipNumSentences; i++) {
                    if (result[i] > 0.8) {
                        APP_PROF_LOG_PRINT(LEVEL_INFO, "CLIP matching %s, score: %.4f\n", g_txt_lines[i], result[i]);
                    }
                }
            }
            free(result);
        }

        TDL_ReleaseFeatureMeta(&feature);
        image_feature = NULL;
    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_Img_Txt_Clip_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstClipCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Img_Txt_Clip not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_Img_Txt_Clip_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Img_Txt_Clip has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Img_Txt_Clip_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_ClipThreadHandle)
    {
        pthread_join(g_ClipThreadHandle, NULL);
        g_ClipThreadHandle = 0;
    }

    if (g_txt_lines) {
        for (int i = 0; i < g_ClipNumSentences; i++) {
            if (g_txt_lines[i]){
                free(g_txt_lines[i]);
            }
        }
        free(g_txt_lines);
    }

    TDL_CloseModel(g_ClipAiHandle, g_pstClipCfg->model_id_img);
    TDL_CloseModel(g_ClipAiHandle, g_pstClipCfg->model_id_txt);
    s32Ret = TDL_DestroyHandle(g_ClipAiHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_Img_Txt_Clip_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_ClipAiHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Img_Txt_Clip Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}


int app_ipcam_Ai_Img_Txt_Clip_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstClipCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Img_Txt_Clip not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bClipRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Img_Txt_Clip has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Img_Txt_Clip_Proc_Init();
	if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Img_Txt_Clip_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Img_Txt_Clip_Read_Input();
    app_ipcam_Ai_Img_Txt_Clip_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_ClipThreadHandle, NULL, Thread_Img_Txt_Clip_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI Clip_pthread_create failed!\n");
        return s32Ret;
    }

    return CVI_SUCCESS;
}
