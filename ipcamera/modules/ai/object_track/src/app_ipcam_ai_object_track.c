#include <stdio.h>
#include "stdbool.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "app_ipcam_ai.h"
#include "tdl_sdk.h"
#include <pthread.h>
#include <stdio.h>
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define FEATURE_SIZE 256
#define MAX_DET_NUM 100
#define TRACK_REQUEST_PATH "/tmp/track"
#define SELECTION_BOX_SIZE 200

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_OBJECT_TRACK_MODE g_mode = DETECTION;
static uint64_t g_frame_id = 0;
static uint32_t g_lost_start_time;          // Start time when the object is lost
static bool g_lost_timer_started = false;   // Whether the lost timer has started
static APP_PARAM_AI_OBJECT_TRACK_CFG_S g_stObjTrackCfg;
static APP_PARAM_AI_OBJECT_TRACK_CFG_S *g_pstObjTrackCfg = &g_stObjTrackCfg;
static volatile bool g_bObjectTrackRunning = CVI_FALSE;
static volatile bool g_bObjectTrackPause = CVI_FALSE;
static pthread_t g_ObjectTrackHandle;
static TDLHandle g_ObjectTrackTDLHandle;
static TDLObject g_stObjDraw = {0};
SMT_MUTEXAUTOLOCK_INIT(g_Mutex);
static pthread_mutex_t g_StatusMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_ModeMutex = PTHREAD_MUTEX_INITIALIZER;
const int LOST_TIMEOUT_SECONDS = 5;  // Timeout for object lost

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static uint32_t get_time_in_ms() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0) {
    return 0;
  }
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

CVI_VOID app_ipcam_Ai_Object_Track_DefaultBox_Get(int32_t box[4])
{
    CVI_U32 grp_width = g_pstObjTrackCfg->u32GrpWidth;
    CVI_U32 grp_height = g_pstObjTrackCfg->u32GrpHeight;
    CVI_U32 box_size = SELECTION_BOX_SIZE;
    CVI_U32 center_x = 0;
    CVI_U32 center_y = 0;

    if (box == NULL) return;

    if (grp_width == 0 || grp_height == 0) {
        grp_width = 640;
        grp_height = 384;
    }

    if (box_size > grp_width) {
        box_size = grp_width;
    }
    if (box_size > grp_height) {
        box_size = grp_height;
    }

    center_x = grp_width / 2;
    center_y = grp_height / 2;
    box[0] = (int32_t)(center_x - box_size / 2);
    box[1] = (int32_t)(center_y - box_size / 2);
    box[2] = (int32_t)(box[0] + box_size);
    box[3] = (int32_t)(box[1] + box_size);
}

APP_PARAM_AI_OBJECT_TRACK_CFG_S *app_ipcam_Ai_Object_Track_Param_Get(void)
{
    return g_pstObjTrackCfg;
}

APP_PARAM_OBJECT_TRACK_MODE app_ipcam_Ai_Object_Track_Mode_Get(void) {
    pthread_mutex_lock(&g_ModeMutex);
    APP_PARAM_OBJECT_TRACK_MODE mode = g_mode;
    pthread_mutex_unlock(&g_ModeMutex);
    return mode;
}

CVI_VOID app_ipcam_Ai_Object_Track_Mode_Set(APP_PARAM_OBJECT_TRACK_MODE mode) {
    pthread_mutex_lock(&g_ModeMutex);
    g_mode = mode;
    pthread_mutex_unlock(&g_ModeMutex);
}

CVI_VOID app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_BOOL flag)
{
    g_bObjectTrackRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Object_Track_ProcStatus_Get(void)
{
    return g_bObjectTrackRunning;
}

CVI_VOID app_ipcam_Ai_Object_Track_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_StatusMutex);
    g_bObjectTrackPause = flag;
    pthread_mutex_unlock(&g_StatusMutex);
}

CVI_BOOL app_ipcam_Ai_Object_Track_Pause_Get(void)
{
    pthread_mutex_lock(&g_StatusMutex);
    CVI_BOOL Pause = g_bObjectTrackPause;
    pthread_mutex_unlock(&g_StatusMutex);
    return Pause;
}

CVI_VOID app_ipcam_Ai_Object_Track_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    if (pstAiObj == NULL) return;
    if (pstAiObj->info == NULL) {
        pstAiObj->info = malloc(MAX_DET_NUM * sizeof(TDLObjectInfo));
    }
    if (pstAiObj->info == NULL) {
        pstAiObj->size = 0;
        return;
    }

    {
        SMT_MutexAutoLock(g_Mutex, lock);
        if (g_stObjDraw.size == 0 || g_stObjDraw.info == NULL) {
            pstAiObj->size = 0;
            return;
        }
        pstAiObj->size = g_stObjDraw.size <= MAX_DET_NUM ? g_stObjDraw.size : MAX_DET_NUM;
        memcpy(pstAiObj->info, g_stObjDraw.info, pstAiObj->size * sizeof(TDLObjectInfo));
    }

}

static CVI_VOID *Thread_Object_Track_Proc(CVI_VOID *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPSS_GRP VpssGrp = g_pstObjTrackCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstObjTrackCfg->VpssChn;
    TDLObject det_obj_meta = {0};
    TDLObject cur_det_meta = {0};
    TDLTracker track_meta = {0};
    bool track_init = CVI_FALSE;

    /*
     * 单线程主循环调度说明：
     * 1. 每次循环只取一次 VPSS 帧，按 mode 在同一线程执行 DET 或 TRACK。
     * 2. 检测分支负责更新绘制框和 det_obj_meta 缓存，追踪初始化直接复用该缓存。
     * 3. 通过 /tmp/track 作为追踪触发事件源，避免阻塞等待；无事件时自动回检测态。
     * 4. 统一在循环末尾释放 frame/image/meta，保证资源生命周期稳定可控。
     */
    while (app_ipcam_Ai_Object_Track_ProcStatus_Get()) {
        VIDEO_FRAME_INFO_S stFrame = {0};
        TDLImage image = NULL;
        bool frame_acquired = false;
        bool has_track_request = (access(TRACK_REQUEST_PATH, F_OK) == 0);

        if (app_ipcam_Ai_Object_Track_Pause_Get()) {
            usleep(1000*1000);
            continue;
        }

        /*
         * /tmp/track 不存在表示没有外部追踪请求，主循环立即回落到检测态。
         * 这样可以保持和原双线程相同的“事件触发追踪、无事件持续检测”语义。
         */
        if (!has_track_request) {
            app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
            track_init = CVI_FALSE;
            g_lost_timer_started = false;
        } else if (app_ipcam_Ai_Object_Track_Mode_Get() == DETECTION) {
            app_ipcam_Ai_Object_Track_Mode_Set(TRACKING);
        }

        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stFrame, 3000);
        if (s32Ret != 0){
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to CVI_VPSS_GetChnFrame with %x\n", s32Ret);
            continue;
        }
        frame_acquired = true;

        g_frame_id++;

        image = TDL_WrapFrame(&stFrame, true, false);
        if (image == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to wrap frame \n");
            goto loop_cleanup;
        }

        /*
         * 检测阶段：每帧更新检测结果缓存 det_obj_meta，并同步更新 OSD 绘制数据。
         * det_obj_meta 仅在本线程内读写，后续追踪初始化直接复用，避免跨线程共享。
         */
        if (app_ipcam_Ai_Object_Track_Mode_Get() == DETECTION) {
            s32Ret = TDL_Detection(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det, image, &cur_det_meta);
            if (s32Ret != 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_Detection failed with %#x!\n", s32Ret);
                goto loop_cleanup;
            }
            // What changed: Gate test-only [OBS] detection count by debug_log_enable.
            // Previous behavior: Only LEVEL_DEBUG "Detect N objects" existed and was compiled out.
            // Impact: Production logs stay quiet unless tracking debug is enabled.
            // Debug params: det_n=detected object count this frame; used to confirm detector running pre-trigger.
            if (g_pstObjTrackCfg->debug_log_enable) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[OBS] f=%llu det_n=%d\n",
                                   (unsigned long long)g_frame_id, (int)cur_det_meta.size);
            }

            TDL_ReleaseObjectMeta(&det_obj_meta);
            TDL_CopyObjectMeta(&cur_det_meta, &det_obj_meta);
            if (det_obj_meta.size > 0 && det_obj_meta.info == NULL) {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "det_obj_meta copy invalid, reset to empty\n");
                det_obj_meta.size = 0;
            }

            {
                SMT_MutexAutoLock(g_Mutex, lock);
                g_stObjDraw.size = 0;
                if (cur_det_meta.size > 0 &&
                    cur_det_meta.info != NULL &&
                    g_stObjDraw.info != NULL) {
                    g_stObjDraw.size = cur_det_meta.size <= MAX_DET_NUM ? cur_det_meta.size : MAX_DET_NUM;
                    memcpy(g_stObjDraw.info, cur_det_meta.info, g_stObjDraw.size * sizeof(TDLObjectInfo));
                }
            }

            goto loop_cleanup;
        }

        /*
         * 追踪初始化阶段：读取一次 /tmp/track 事件并调用 SetSingleObjectTracking。
         * 初始化成功后切换到持续追踪；失败则清理事件并回到检测态。
         */
        if (!track_init) {
            char buf[5] = {0};
            const char *model_path = NULL;
            TDLTargetSearchTypeE search_type = g_pstObjTrackCfg->search_type;
            TDLObject empty_det_meta = {0};
            FILE *pFile = fopen(TRACK_REQUEST_PATH, "r");
            if (pFile == NULL) {
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                goto loop_cleanup;
            }
            size_t read_size = fread(buf, 1, sizeof(buf), pFile);
            fclose(pFile);
            if (read_size == 0) {
                remove(TRACK_REQUEST_PATH);
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                goto loop_cleanup;
            }

            int32_t box[4] = {0};
            app_ipcam_Ai_Object_Track_DefaultBox_Get(box);
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "track box : [%d, %d, %d, %d] \n", box[0], box[1], box[2], box[3]);
            if (search_type < TDL_REJECT || search_type > TDL_FASTSAM) {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "invalid search_type(%d), fallback to TDL_REJECT\n", search_type);
                search_type = TDL_REJECT;
            }
            if (search_type == TDL_FASTSAM) {
                model_path = g_pstObjTrackCfg->model_path_sam;
            }

            s32Ret = TDL_SetSingleObjectTracking(g_ObjectTrackTDLHandle, image, &empty_det_meta, box, 4, g_frame_id,
                                                 search_type, model_path);
            if (s32Ret != 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSingleObjectTracking failed with %#x!\n", s32Ret);
                remove(TRACK_REQUEST_PATH);
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                track_init = CVI_FALSE;
            } else {
                track_init = CVI_TRUE;
                g_lost_timer_started = false;
            }
        } else {
            /*
             * 持续追踪阶段：每帧调用 SingleObjectTracking 并更新单目标框。
             * 当连续 LOST 超时后，按原逻辑回退检测态并等待下一次外部触发。
             */
            s32Ret = TDL_SingleObjectTracking(g_ObjectTrackTDLHandle, image, &track_meta, g_frame_id);
            if (s32Ret != 0) {
               APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SingleObjectTracking failed with %#x!\n", s32Ret);
               app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
               track_init = CVI_FALSE;
               remove(TRACK_REQUEST_PATH);
               goto loop_cleanup;
            }

            // What changed: Gate test-only [OBS] SOT score/threshold/pass/box print.
            // Previous behavior: Only LEVEL_DEBUG "track score lower than threshold" existed and was compiled out.
            // Impact: Production logs stay quiet unless tracking debug is enabled.
            // Debug params: f=frame id; score=SOT confidence; thr=ini gate; pass=score>=thr; box=track bbox;
            //               used to compare threshold tightness and kalman effect across groups.
            if (g_pstObjTrackCfg->debug_log_enable) {
                if (track_meta.info != NULL) {
                    APP_PROF_LOG_PRINT(LEVEL_INFO,
                        "[OBS] f=%llu score=%.3f thr=%.3f pass=%d box=[%.0f,%.0f,%.0f,%.0f]\n",
                        (unsigned long long)g_frame_id,
                        track_meta.info[0].score,
                        g_pstObjTrackCfg->tracking_score_threshold,
                        (track_meta.info[0].score >= g_pstObjTrackCfg->tracking_score_threshold) ? 1 : 0,
                        (float)track_meta.info[0].bbox.x1, (float)track_meta.info[0].bbox.y1,
                        (float)track_meta.info[0].bbox.x2, (float)track_meta.info[0].bbox.y2);
                } else {
                    APP_PROF_LOG_PRINT(LEVEL_INFO,
                        "[OBS] f=%llu info=NULL(LOST)\n", (unsigned long long)g_frame_id);
                }
            }
            if (track_meta.info != NULL &&
                track_meta.info[0].score >= g_pstObjTrackCfg->tracking_score_threshold) {
                SMT_MutexAutoLock(g_Mutex, lock);
                if (g_stObjDraw.info != NULL) {
                    g_stObjDraw.size = 1;
                    g_stObjDraw.info[0].box.x1 = track_meta.info[0].bbox.x1;
                    g_stObjDraw.info[0].box.x2 = track_meta.info[0].bbox.x2;
                    g_stObjDraw.info[0].box.y1 = track_meta.info[0].bbox.y1;
                    g_stObjDraw.info[0].box.y2 = track_meta.info[0].bbox.y2;
                } else {
                    g_stObjDraw.size = 0;
                }
            } else {
                if (track_meta.info != NULL) {
                    // What changed: Log weak tracking scores before lost handling.
                    // Previous behavior: Low-score tracking boxes had no visibility.
                    // Impact: Helps tune tracking_score_threshold for board scenes.
                    // Debug params: score means SOT confidence, used to verify weak-track filtering;
                    // threshold means ini gate, used to tune lost detection.
                    APP_PROF_LOG_PRINT(LEVEL_DEBUG,
                                       "track score %.3f lower than threshold %.3f\n",
                                       track_meta.info[0].score,
                                       g_pstObjTrackCfg->tracking_score_threshold);
                }
                if (!g_lost_timer_started) {
                    g_lost_start_time = get_time_in_ms();
                    g_lost_timer_started = true;
                } else {
                    uint32_t current_time = get_time_in_ms();
                    uint32_t elapsed_time = current_time - g_lost_start_time;
                    if (elapsed_time >= (uint32_t)(LOST_TIMEOUT_SECONDS * 1000)) {
                        APP_PROF_LOG_PRINT(LEVEL_WARN,
                                           "The target has been lost for more than [%d] seconds, "
                                           "switching to detection state\n",
                                           LOST_TIMEOUT_SECONDS);
                        app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                        g_lost_timer_started = false;
                        track_init = CVI_FALSE;
                        remove(TRACK_REQUEST_PATH);
                    }
                }
            }
        }

loop_cleanup:
        if (track_meta.info != NULL) {
            TDL_ReleaseTrackMeta(&track_meta);
            memset(&track_meta, 0, sizeof(track_meta));
        }
        if (cur_det_meta.info != NULL) {
            TDL_ReleaseObjectMeta(&cur_det_meta);
            memset(&cur_det_meta, 0, sizeof(cur_det_meta));
        }
        if (image != NULL) {
            TDL_DestroyImage(image);
        }
        if (frame_acquired) {
            s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrame);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            }
        }
    }

    TDL_ReleaseObjectMeta(&det_obj_meta);
    pthread_exit(NULL);

    return NULL;
}

static CVI_S32 app_ipcam_Ai_Object_Track_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI ObjectTrack init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_ObjectTrackTDLHandle == NULL)
    {
        g_ObjectTrackTDLHandle = TDL_CreateHandle(0);
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDLHandle has created\n");
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_ObjectTrackTDLHandle, TDL_MODEL_YOLOV8N_DET_PERSON_VEHICLE, g_pstObjTrackCfg->model_path_det, g_pstObjTrackCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel DET failed with %#x!\n", s32Ret);
        return s32Ret;
    }else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "TDL_OpenModel DET success !\n");
    }

    s32Ret = TDL_OpenModel(g_ObjectTrackTDLHandle, TDL_MODEL_TRACKING_FEARTRACK, g_pstObjTrackCfg->model_path_sot, g_pstObjTrackCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel SOT failed with %#x!\n", s32Ret);
        return s32Ret;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "TDL_OpenModel SOT success !\n");
    }

    s32Ret = TDL_SetSingleObjectTrackingUseKalman(g_ObjectTrackTDLHandle,
                                                  g_pstObjTrackCfg->use_kalman);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSingleObjectTrackingUseKalman failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    if (g_pstObjTrackCfg->debug_log_enable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "[OBS] cfg use_kalman=%d tracking_score_threshold=%.3f search_type=%d kalman_ret=0x%x\n",
            g_pstObjTrackCfg->use_kalman ? 1 : 0,
            g_pstObjTrackCfg->tracking_score_threshold,
            g_pstObjTrackCfg->search_type,
            s32Ret);
    }

    // s32Ret = TDL_SetSotModelThreshold(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->threshold_occluded, g_pstObjTrackCfg->threshold_reappear);
    // if (s32Ret != CVI_SUCCESS)
    // {
    //     APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSotModelThreshold failed with %#x!\n", s32Ret);
    //     return s32Ret;
    // }

    if (g_stObjDraw.info == NULL) {
        g_stObjDraw.info = malloc(MAX_DET_NUM * sizeof(TDLObjectInfo));
        if (g_stObjDraw.info == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc g_stObjDraw.info failed\n");
            return CVI_FAILURE;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI ObjectTrack init ------------------> done \n");

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Object_Track_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstObjTrackCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack not enable\n");
        return CVI_SUCCESS;
    }

    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_FALSE);
    pthread_join(g_ObjectTrackHandle, NULL);
    g_ObjectTrackHandle = 0;

    {
        SMT_MutexAutoLock(g_Mutex, lock);
        if (g_stObjDraw.info != NULL) {
            free(g_stObjDraw.info);
            g_stObjDraw.info = NULL;
        }
        g_stObjDraw.size = 0;
    }

    TDL_CloseModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det);
    TDL_CloseModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_sot);

    s32Ret = TDL_DestroyHandle(g_ObjectTrackTDLHandle);

    if(s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "enter TDL_DestroyHandle fail \n");
        return s32Ret;
    }
    g_ObjectTrackTDLHandle = NULL;

    return s32Ret;
}

int app_ipcam_Ai_Object_Track_Start(void){
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstObjTrackCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bObjectTrackRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Object_Track_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Object_Track_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_TRUE);
    s32Ret = pthread_create(&g_ObjectTrackHandle, NULL, Thread_Object_Track_Proc, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI ObjectTrack_pthread_create failed!\n");
        return s32Ret;
    }

    return s32Ret;
}
