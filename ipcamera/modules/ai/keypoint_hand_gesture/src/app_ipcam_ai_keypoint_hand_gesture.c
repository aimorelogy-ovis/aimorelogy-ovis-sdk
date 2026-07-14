#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include <math.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include "app_ipcam_ai.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
SMT_MUTEXAUTOLOCK_INIT(g_KeypointHandGestureMutex);
static pthread_mutex_t g_KeypointHandGestureStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_KEYPOINT_HAND_GESTURE_CFG_S g_stKeypointHandGestureCfg;

static APP_PARAM_AI_KEYPOINT_HAND_GESTURE_CFG_S *g_pstKeypointHandGestureCfg = &g_stKeypointHandGestureCfg;

// static CVI_U32 g_KeypointHandGestureProc;
static volatile bool g_bKeypointHandGestureRunning = CVI_FALSE;
static volatile bool g_bKeypointHandGesturePause = CVI_FALSE;
static pthread_t g_KeypointHandGestureThreadHandle;
static TDLHandle g_KeypointHandGestureAiHandle = NULL;
static TDLObject g_stKeypointHandGestureObjDraw;
// static pfpInferenceFunc g_pfpKeypointHandGestureInference;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_KEYPOINT_HAND_GESTURE_CFG_S *app_ipcam_Ai_Keypoint_Hand_Gesture_Param_Get(void)
{
    return g_pstKeypointHandGestureCfg;
}

CVI_VOID app_ipcam_Ai_Keypoint_Hand_Gesture_ProcStatus_Set(CVI_BOOL flag)
{
    g_bKeypointHandGestureRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Keypoint_Hand_Gesture_ProcStatus_Get(void)
{
    return g_bKeypointHandGestureRunning;
}

CVI_VOID app_ipcam_Ai_Keypoint_Hand_Gesture_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_KeypointHandGestureStatusMutex);
    g_bKeypointHandGesturePause = flag;
    pthread_mutex_unlock(&g_KeypointHandGestureStatusMutex);
}

CVI_BOOL app_ipcam_Ai_Keypoint_Hand_Gesture_Pause_Get(void)
{
    return g_bKeypointHandGesturePause;
}

static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(TDLModel detect_model_id, TDLModel keypoint_model_id)
{
    // 验证模型组合的有效性，类似于sample_detect_keypoints.c中的get_model_info
    bool valid_combination = false;
    
    // 手部检测组合
    if (detect_model_id == TDL_MODEL_YOLOV8N_DET_HAND && keypoint_model_id == TDL_MODEL_KEYPOINT_HAND) {
        valid_combination = true;
    }
    // 车牌检测组合 
    else if (detect_model_id == TDL_MODEL_YOLOV8N_DET_LICENSE_PLATE && keypoint_model_id == TDL_MODEL_KEYPOINT_LICENSE_PLATE) {
        valid_combination = true;
    }
    // 人体姿态检测组合
    else if (detect_model_id == TDL_MODEL_MBV2_DET_PERSON && keypoint_model_id == TDL_MODEL_KEYPOINT_SIMCC_PERSON17) {
        valid_combination = true;
    }
    
    if (!valid_combination) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Invalid model combination: detect_model_id=%d, keypoint_model_id=%d\n", 
            detect_model_id, keypoint_model_id);
        return CVI_FAILURE;
    }
    
    return CVI_SUCCESS;
}

static void app_ipcam_Ai_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "=== AI Keypoint Hand Gesture Configuration ===\n");
    APP_PROF_LOG_PRINT(LEVEL_INFO, "keypoint_hand_gesture_enable=%d\n", g_pstKeypointHandGestureCfg->bEnable);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "vpss_grp=%d vpss_chn=%d\n", 
        g_pstKeypointHandGestureCfg->VpssGrp, g_pstKeypointHandGestureCfg->VpssChn);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "detect_model_id=%d\n", g_pstKeypointHandGestureCfg->detect_model_id);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "detect_model_path=%s\n", g_pstKeypointHandGestureCfg->detect_model_path);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "keypoint_model_id=%d\n", g_pstKeypointHandGestureCfg->keypoint_model_id);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "keypoint_model_path=%s\n", g_pstKeypointHandGestureCfg->keypoint_model_path);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "enable_classification=%d\n", g_pstKeypointHandGestureCfg->bEnableClassification);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "classify_model_id=%d\n", g_pstKeypointHandGestureCfg->classify_model_id);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "classify_model_path=%s\n", g_pstKeypointHandGestureCfg->classify_model_path);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_path_cfg=%s\n", g_pstKeypointHandGestureCfg->model_path_cfg);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "===============================================\n");
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
            dst->info[i].score = src->info[i].score;
            dst->info[i].class_id = src->info[i].class_id;
            dst->info[i].landmark_size = src->info[i].landmark_size;
            dst->info[i].obj_type = src->info[i].obj_type;
            
            // 对于手势检测，通常需要关键点信息
            if (src->info[i].landmark_properity != NULL && src->info[i].landmark_size > 0) {
                dst->info[i].landmark_properity = (TDLLandmarkInfo*)malloc(src->info[i].landmark_size * sizeof(TDLLandmarkInfo));
                if (dst->info[i].landmark_properity == NULL) {
                    return -1;
                }
                for (uint32_t j = 0; j < src->info[i].landmark_size; j++) {
                    dst->info[i].landmark_properity[j].x = src->info[i].landmark_properity[j].x;
                    dst->info[i].landmark_properity[j].y = src->info[i].landmark_properity[j].y;
                    dst->info[i].landmark_properity[j].score = src->info[i].landmark_properity[j].score;
                }
            }
        }
    } else {
        return 0;
    }

    return 0;
}

static CVI_S32 app_ipcam_Ai_Keypoint_Hand_Gesture_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Keypoint Hand Gesture Detection init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    app_ipcam_Ai_Param_dump();

    if (g_KeypointHandGestureAiHandle == NULL)
    {
        g_KeypointHandGestureAiHandle = TDL_CreateHandle(0);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_CreateHandle has created\n");
        return s32Ret;
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "这里打印detect_model_id: %d\n", g_pstKeypointHandGestureCfg->detect_model_id);
    s32Ret = app_ipcam_Ai_InferenceFunc_Get(g_pstKeypointHandGestureCfg->detect_model_id, g_pstKeypointHandGestureCfg->keypoint_model_id);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupported model id: detect=%d, keypoint=%d \n", 
            g_pstKeypointHandGestureCfg->detect_model_id, g_pstKeypointHandGestureCfg->keypoint_model_id);
        return s32Ret;
    }

    // 打开检测模型
    s32Ret = TDL_OpenModel(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->detect_model_id, g_pstKeypointHandGestureCfg->detect_model_path, g_pstKeypointHandGestureCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_SetModelPath failed for detect model with %#x! maybe reset model path\n", s32Ret);
        return s32Ret;
    }

    // 打开关键点模型
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Opening keypoint model: %s (ID: %d)\n", 
        g_pstKeypointHandGestureCfg->keypoint_model_path, g_pstKeypointHandGestureCfg->keypoint_model_id);
    s32Ret = TDL_OpenModel(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->keypoint_model_id, g_pstKeypointHandGestureCfg->keypoint_model_path, g_pstKeypointHandGestureCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_SetModelPath failed for keypoint model with %#x! maybe reset model path\n", s32Ret);
        return s32Ret;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Keypoint model opened successfully\n");
        
        // // 尝试禁用关键点模型的VPSS预处理以避免冲突
        // if (g_pstKeypointHandGestureCfg->bVpssPreProcSkip == 0) {
        //     APP_PROF_LOG_PRINT(LEVEL_INFO, "Attempting to configure keypoint model preprocessing...\n");
            
        //     // 注意：这里可能需要根据TDL SDK的具体API来设置预处理参数
        //     // 如果有TDL_SetVpssPreprocess之类的函数，可以在这里调用
        // }
    }

    // 设置检测模型阈值
    s32Ret = TDL_SetModelThreshold(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->detect_model_id, g_pstKeypointHandGestureCfg->threshold);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_SetModelThreshold failed for detect model with %#x!\n", s32Ret);
        return s32Ret;
    }

    // 如果启用分类且是手部关键点模型，打开分类模型
    if (g_pstKeypointHandGestureCfg->bEnableClassification && 
        g_pstKeypointHandGestureCfg->keypoint_model_id == TDL_MODEL_KEYPOINT_HAND) {
        
        s32Ret = TDL_OpenModel(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->classify_model_id, 
                               g_pstKeypointHandGestureCfg->classify_model_path, g_pstKeypointHandGestureCfg->model_path_cfg, 0);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to open classification model with %#x! Classification disabled.\n", s32Ret);
            g_pstKeypointHandGestureCfg->bEnableClassification = CVI_FALSE;
        } else {
            // 设置分类模型阈值
            s32Ret = TDL_SetModelThreshold(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->classify_model_id, 0.5);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to set classification model threshold with %#x!\n", s32Ret);
            }
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Hand gesture classification model loaded successfully\n");
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Keypoint Hand Gesture Detection init ------------------> done \n");

    return CVI_SUCCESS;
}

static CVI_VOID *Thread_Keypoint_Hand_Gesture_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Keypoint Hand Gesture Detection start running!\n");

    prctl(PR_SET_NAME, "Thread_Keypoint_Hand_Gesture_PROC");

    VPSS_GRP VpssGrp = g_pstKeypointHandGestureCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstKeypointHandGestureCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};
    TDLImage image_handle;

    int frame_count = 0;  // 添加帧计数器
    static float last_checksum = -1.0f;  // 记录上次的校验和
    
    // 移除时间控制变量，实现连续检测

    while (app_ipcam_Ai_Keypoint_Hand_Gesture_ProcStatus_Get()) {
        pthread_mutex_lock(&g_KeypointHandGestureStatusMutex);
        s32Ret = app_ipcam_Ai_Keypoint_Hand_Gesture_Pause_Get();
        
        if (s32Ret) {
            pthread_mutex_unlock(&g_KeypointHandGestureStatusMutex);
            usleep(1000*1000);
            // printf("打印手势识别暂停中...\n");
            continue;
        }
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 3000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            pthread_mutex_unlock(&g_KeypointHandGestureStatusMutex);
            usleep(100*1000);
            continue;
        }
        image_handle = TDL_WrapFrame((void*)&stfdFrame, false, false);

        pthread_mutex_unlock(&g_KeypointHandGestureStatusMutex);

    // 移除定时检测逻辑，直接连续检测
    frame_count++;

        // 第一步：执行检测
        TDLObject obj_meta = {0};
        CVI_S32 ret = TDL_Detection(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->detect_model_id, image_handle, &obj_meta);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_Detection failed with %#x\n", ret);
            s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
            TDL_DestroyImage(image_handle);
            continue;
        }

        if (obj_meta.size <= 0) {
            // 没有检测到目标
            frame_count++;
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[DETECTION] Frame %d: No objects detected (detection cycle)\n", frame_count);
            TDL_ReleaseObjectMeta(&obj_meta);
            s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
            TDL_DestroyImage(image_handle);
            continue;
        }

        // 第二步：执行关键点检测
        // APP_PROF_LOG_PRINT(LEVEL_INFO, "[KEYPOINT] Calling TDL_DetectionKeypoint for %u detected objects...\n", obj_meta.size);
        
        // 保存关键点检测前的状态用于对比
        uint32_t landmark_size_before = obj_meta.size > 0 ? obj_meta.info[0].landmark_size : 0;
        
        // 尝试设置关键点模型的预处理参数来避免VPSS冲突
        // 对于手部关键点模型，可能需要特定的预处理设置
        if (g_pstKeypointHandGestureCfg->keypoint_model_id == TDL_MODEL_KEYPOINT_HAND) {
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[KEYPOINT] Setting up preprocessing for hand keypoint model...\n");
            
            // 检查关键点检测前是否有预处理错误
            // 如果有错误，跳过这一帧
            if (frame_count > 1) {  // 给第一帧一些时间初始化
                static int consecutive_errors = 0;
                
                // 检查前一次检测的结果是否异常
                if (landmark_size_before == 0) {
                    // 正常情况，检测模型检测到目标但没有关键点
                } else {
                    // 异常情况，可能需要重置
                    consecutive_errors++;
                    if (consecutive_errors > 10) {
                        APP_PROF_LOG_PRINT(LEVEL_WARN, "[KEYPOINT] Too many preprocessing errors, skipping keypoint detection\n");
                        frame_count++;
                        // goto skip_keypoint_detection;
                    }
                }
            }
        }
        
        ret = TDL_DetectionKeypoint(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->keypoint_model_id, image_handle, &obj_meta, NULL);
        
        // skip_keypoint_detection:
        
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_DetectionKeypoint failed with %#x!\n", ret);
        } else {
            frame_count++;
            
            // 检查关键点是否正确添加
            uint32_t landmark_size_after = obj_meta.size > 0 ? obj_meta.info[0].landmark_size : 0;
            // APP_PROF_LOG_PRINT(LEVEL_INFO, "[KEYPOINT] Frame %d: Landmark count changed from %u to %u\n", 
                // frame_count, landmark_size_before, landmark_size_after);
            
            // 如果关键点数量没有变化，这可能表明关键点检测没有正常工作
            if (landmark_size_before == landmark_size_after && landmark_size_after == 0) {
                // APP_PROF_LOG_PRINT(LEVEL_WARN, "[KEYPOINT] WARNING: No keypoints detected after TDL_DetectionKeypoint!\n");
            } else if (landmark_size_after > 0) {
                // APP_PROF_LOG_PRINT(LEVEL_INFO, "[KEYPOINT] SUCCESS: %u keypoints detected\n", landmark_size_after);
                // 输出第一个关键点的坐标作为参考
                if (obj_meta.size > 0 && obj_meta.info[0].landmark_properity != NULL) {
                    // APP_PROF_LOG_PRINT(LEVEL_INFO, "[KEYPOINT] First keypoint: [%.6f, %.6f] score=%.3f\n",
                    //     obj_meta.info[0].landmark_properity[0].x, 
                    //     obj_meta.info[0].landmark_properity[0].y,
                    //     obj_meta.info[0].landmark_properity[0].score);
                }
            }
            
            // APP_PROF_LOG_PRINT(LEVEL_INFO, "[KEYPOINT] Frame %d: Detected %u objects with keypoints\n", frame_count, obj_meta.size);
            // APP_PROF_LOG_PRINT(LEVEL_INFO, "[KEYPOINT] Image dimensions: %ux%u\n", obj_meta.width, obj_meta.height);
            
            // 输出检测结果和关键点坐标
            for (uint32_t i = 0; i < obj_meta.size; i++) {
                // APP_PROF_LOG_PRINT(LEVEL_INFO, "[OBJECT %u] Box: [%f, %f, %f, %f], Score: %f, Class: %d\n", 
                //     i, obj_meta.info[i].box.x1, obj_meta.info[i].box.y1, 
                //     obj_meta.info[i].box.x2, obj_meta.info[i].box.y2,
                //     obj_meta.info[i].score, obj_meta.info[i].class_id);
                
                // 第三步：如果启用分类且有关键点，进行手势分类
                if (obj_meta.info[i].landmark_size > 0) {
                    
                    // 显示关键点坐标信息（用于调试）
                    // APP_PROF_LOG_PRINT(LEVEL_INFO, "[KEYPOINTS] Frame %d: Object %u has %u keypoints\n", 
                    //     frame_count, i, obj_meta.info[i].landmark_size);
                    
                    // 打印前5个关键点的详细坐标信息
                    // for (uint32_t j = 0; j < obj_meta.info[i].landmark_size && j < 5; j++) {
                    //     float x, y;
                    //     if (g_pstKeypointHandGestureCfg->keypoint_model_id == TDL_MODEL_KEYPOINT_HAND) {
                    //         // 手部关键点通常是归一化坐标，也显示像素坐标用于对比
                    //         x = obj_meta.info[i].landmark_properity[j].x * obj_meta.width;
                    //         y = obj_meta.info[i].landmark_properity[j].y * obj_meta.height;
                    //     } else {
                    //         // 其他关键点模型可能已经是像素坐标
                    //         x = obj_meta.info[i].landmark_properity[j].x;
                    //         y = obj_meta.info[i].landmark_properity[j].y;
                    //     }
                        
                        // 显示坐标信息（norm为输入分类器的坐标，pixel为显示坐标）
                        // APP_PROF_LOG_PRINT(LEVEL_INFO, "[KEYPOINT %u] Point %u: norm[%.6f, %.6f] pixel[%.2f, %.2f], Score: %.3f\n", 
                        //     i, j, obj_meta.info[i].landmark_properity[j].x, obj_meta.info[i].landmark_properity[j].y, 
                        //     x, y, obj_meta.info[i].landmark_properity[j].score);
                    // }
                    
                    // 如果启用了分类且关键点检测是手部模型，进行手势分类
                    if (g_pstKeypointHandGestureCfg->bEnableClassification && 
                        g_pstKeypointHandGestureCfg->keypoint_model_id == TDL_MODEL_KEYPOINT_HAND &&
                        obj_meta.info[i].landmark_size == 21) {
                        
                        // APP_PROF_LOG_PRINT(LEVEL_INFO, "[GESTURE] Hand keypoints detected, performing gesture classification...\n");
                        
                        // 准备关键点特征数据 (直接使用归一化坐标)
                        float keypoints[42];
                        uint32_t keypoints_index = 0;
                        float checksum = 0.0f;  // 用于验证输入数据变化
                        
                        for (int k = 0; k < 21; k++) {
                            // 直接使用归一化的关键点坐标，不进行额外转换
                            keypoints[keypoints_index++] = obj_meta.info[i].landmark_properity[k].x;
                            keypoints[keypoints_index++] = obj_meta.info[i].landmark_properity[k].y;
                            
                            // 计算校验和
                            checksum += obj_meta.info[i].landmark_properity[k].x + obj_meta.info[i].landmark_properity[k].y;
                        }
                        
                        // 输出校验和用于验证输入数据变化
                        // APP_PROF_LOG_PRINT(LEVEL_INFO, "[GESTURE] Input checksum: %.6f (Frame %d)\n", checksum, frame_count);
                        
                        // 检查输入数据是否变化
                        if (last_checksum > 0 && fabs(checksum - last_checksum) < 0.001f) {
                            // APP_PROF_LOG_PRINT(LEVEL_WARN, "[GESTURE] WARNING: Input data seems unchanged! (checksum diff: %.6f)\n", 
                            //     fabs(checksum - last_checksum));
                        }
                        last_checksum = checksum;
                        
                        // 打印前3个关键点的输入数据用于调试
                        for (int k = 0; k < 6; k += 2) {
                            // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[INPUT] keypoint[%d]: x=%.6f, y=%.6f\n", 
                            //     k/2, keypoints[k], keypoints[k+1]);
                        }
                        
                        // 准备 frame 结构体 (与 sample_detect_keypoints.c 一致)
                        VIDEO_FRAME_INFO_S frame;
                        memset(&frame, 0, sizeof(VIDEO_FRAME_INFO_S));
                        uint32_t buffer_size = 42 * sizeof(float);
                        CVI_U8 *buffer = (CVI_U8 *)malloc(buffer_size);
                        
                        if (buffer != NULL) {
                            // 直接将 float 数据复制到缓冲区
                            memcpy(buffer, keypoints, buffer_size);
                            
                            // 设置frame参数用于特征数据
                            frame.stVFrame.pu8VirAddr[0] = buffer;
                            frame.stVFrame.u32Width = 42;  // 42 float
                            frame.stVFrame.u32Height = 1;
                            
                            TDLImage cls_image = TDL_WrapFrame(&frame, false, false);
                            if (cls_image != NULL) {
                                // 执行手势分类 (分类模型已在初始化时打开)
                                TDLClassInfo class_info = {0};
                                CVI_S32 classify_ret = TDL_Classification(g_KeypointHandGestureAiHandle, 
                                                                          g_pstKeypointHandGestureCfg->classify_model_id, 
                                                                          cls_image, &class_info);
                                
                                if (classify_ret == CVI_SUCCESS) {
                                    // 手势分类成功，输出结果 (与 sample_detect_keypoints.c 一致)
                                    const char* gesture_names[] = {
                                        "fist", "five", "four", "none", "ok", "one", "three", "three2", "two"
                                    };
                                    
                                    if (class_info.class_id >= 0 && class_info.class_id < 9 && class_info.class_id != 3) {
                                        APP_PROF_LOG_PRINT(LEVEL_INFO, "Hand Gesture Class: %s (ID: %d)\n", 
                                            gesture_names[class_info.class_id], class_info.class_id);
                                    } 
                                    
                                    // 将分类结果保存到对象中 (可用于后续绘制或UI显示)
                                    obj_meta.info[i].class_id = class_info.class_id;
                                    
                                } else {
                                    // APP_PROF_LOG_PRINT(LEVEL_ERROR, "[GESTURE %u] Classification failed with error: %#x\n", i, classify_ret);
                                }
                                
                                // 清理TDLImage资源
                                TDL_DestroyImage(cls_image);
                            } else {
                                // APP_PROF_LOG_PRINT(LEVEL_ERROR, "[GESTURE] Failed to wrap feature data as TDLImage\n");
                            }
                            
                            // 释放特征数据缓冲区
                            free(buffer);
                        } else {
                            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[GESTURE] Failed to allocate feature buffer\n");
                        }
                    }
                }
            }
        }
        
        // APP_PROF_LOG_PRINT(LEVEL_TRACE, "Keypoint Hand Gesture Detect process takes %d\n", g_KeypointHandGestureProc);
        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            // APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        TDL_DestroyImage(image_handle);
        
        // 更新绘制信息
        SMT_MutexAutoLock(g_KeypointHandGestureMutex, lock);
        if (g_stKeypointHandGestureObjDraw.info != NULL) {
            TDL_ReleaseObjectMeta(&g_stKeypointHandGestureObjDraw);
        }
        memset(&g_stKeypointHandGestureObjDraw, 0, sizeof(TDLObject));
        
        // 深拷贝检测结果用于绘制
        if (obj_meta.size > 0) {
            deep_copy_tdl_object(&g_stKeypointHandGestureObjDraw, &obj_meta);
        }
        
        // 释放检测结果
        TDL_ReleaseObjectMeta(&obj_meta);
    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_Keypoint_Hand_Gesture_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    _NULL_POINTER_CHECK_(pstAiObj, -1);

    SMT_MutexAutoLock(g_KeypointHandGestureMutex, lock);
    if (g_stKeypointHandGestureObjDraw.size == 0) {
        return CVI_SUCCESS;
    } else {
        memset(pstAiObj, 0, sizeof(TDLObject));
        deep_copy_tdl_object(pstAiObj, &g_stKeypointHandGestureObjDraw);
        if (g_stKeypointHandGestureObjDraw.info != NULL) { 
            TDL_ReleaseObjectMeta(&g_stKeypointHandGestureObjDraw);
        }
    }

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Keypoint_Hand_Gesture_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstKeypointHandGestureCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Keypoint Hand Gesture Detection not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_Keypoint_Hand_Gesture_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Keypoint Hand Gesture Detection has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Keypoint_Hand_Gesture_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_KeypointHandGestureThreadHandle)
    {
        pthread_join(g_KeypointHandGestureThreadHandle, NULL);
        g_KeypointHandGestureThreadHandle = 0;
    }

    // 关闭检测模型
    TDL_CloseModel(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->detect_model_id);
    // 关闭关键点模型
    TDL_CloseModel(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->keypoint_model_id);
    // 如果分类模型已启用，关闭分类模型
    if (g_pstKeypointHandGestureCfg->bEnableClassification && 
        g_pstKeypointHandGestureCfg->keypoint_model_id == TDL_MODEL_KEYPOINT_HAND) {
        TDL_CloseModel(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->classify_model_id);
    }
    
    s32Ret = TDL_DestroyHandle(g_KeypointHandGestureAiHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_KeypointHandGestureAiHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Keypoint Hand Gesture Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Keypoint_Hand_Gesture_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstKeypointHandGestureCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Keypoint Hand Gesture Detection not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bKeypointHandGestureRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Keypoint Hand Gesture Detection has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Keypoint_Hand_Gesture_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Keypoint_Hand_Gesture_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Keypoint_Hand_Gesture_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_KeypointHandGestureThreadHandle, NULL, Thread_Keypoint_Hand_Gesture_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI pthread_create failed!\n");
        return s32Ret;
    }
    
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Ai_Keypoint_Hand_Gesture_StatusGet(void)
{
    return g_KeypointHandGestureAiHandle ? 1 : 0;
}

CVI_S32 app_ipcam_Keypoint_Hand_Gesture_threshold_Set(float threshold)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_KeypointHandGestureAiHandle) {
        // 只为检测模型设置阈值
        s32Ret = TDL_SetModelThreshold(g_KeypointHandGestureAiHandle, g_pstKeypointHandGestureCfg->detect_model_id, threshold);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelThreshold failed with %#x!\n", g_pstKeypointHandGestureCfg->detect_model_path, s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}
