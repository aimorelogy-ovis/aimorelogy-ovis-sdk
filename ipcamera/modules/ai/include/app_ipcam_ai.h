#ifndef __APP_IPCAM_AI_H__
#define __APP_IPCAM_AI_H__

#include "cvi_type.h"
#include "tdl_sdk.h"
#include "tdl_utils.h"
#include "cvi_comm_video.h"
#include "cvi_vpss.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_vi.h"
#include "app_ipcam_vpss.h"
#include "app_ipcam_venc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MODEL_PATH_LEN  128


#define SMT_MUTEXAUTOLOCK_INIT(mutex) pthread_mutex_t AUTOLOCK_##mutex = PTHREAD_MUTEX_INITIALIZER;

#define SMT_MutexAutoLock(mutex, lock) __attribute__((cleanup(AutoUnLock))) \
       pthread_mutex_t *lock= &AUTOLOCK_##mutex;\
       pthread_mutex_lock(lock);

__attribute__ ((always_inline)) inline void AutoUnLock(void *mutex) {
       pthread_mutex_unlock( *(pthread_mutex_t**) mutex);
}

#define SEND_SIGNAL_DRAW_AI_OBJS_RECT(mutex, osdcCmprCond)  \
    do {                                                    \
        pthread_mutex_lock(&mutex);                         \
        pthread_cond_signal(&osdcCmprCond);                 \
        pthread_mutex_unlock(&mutex);                       \
    } while(0)

#define WAIT_SIGNAL_DRAW_AI_OBJS_RECT(mutex, osdcCmprCond, timeout)  \
    do {                                                             \
        pthread_mutex_lock(&mutex);                                  \
        pthread_cond_timedwait(&osdcCmprCond, &mutex, &timeout);     \
        pthread_mutex_unlock(&mutex);                                \
    } while(0)

typedef CVI_S32 (*pfpInferenceFunc)(TDLHandle handle, const TDLModel model_id, TDLImage image_handle, TDLObject *obj);
typedef CVI_S32 (*pfpFaceInferenceFunc)(TDLHandle handle, const TDLModel model_id, TDLImage image_handle, TDLFace *obj);
typedef CVI_S32 (*pfpRescaleFunc)(const TDLImage image_handle, TDLObject *obj);

typedef struct APP_PARAM_AI_MD_CFG_T {
    CVI_BOOL bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32GrpWidth;
	CVI_U32 u32GrpHeight;
    CVI_U32 threshold;
    CVI_U32 u32BgUpPeriod;
    CVI_U32 miniArea;
} APP_PARAM_AI_MD_CFG_S;


typedef struct APP_PARAM_AI_CRY_CFG_T {
    CVI_BOOL bEnable;
    TDLModel model_id;
    CVI_U32 application_scene;
    char model_path[MODEL_PATH_LEN];
} APP_PARAM_AI_CRY_CFG_S;

typedef struct APP_PARAM_AI_PD_CFG_T {
    CVI_BOOL bEnable;
    // CVI_BOOL Intrusion_bEnable;
    // CVI_BOOL capture_enable;
    // CVI_S32 capture_frames;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32GrpWidth;
	CVI_U32 u32GrpHeight;
    // CVI_U32 model_size_w;
    // CVI_U32 model_size_h;
    // CVI_U32 region_stRect_x1;
    // CVI_U32 region_stRect_y1;
    // CVI_U32 region_stRect_x2;
    // CVI_U32 region_stRect_y2;
    // CVI_U32 region_stRect_x3;
    // CVI_U32 region_stRect_y3;
    // CVI_U32 region_stRect_x4;
    // CVI_U32 region_stRect_y4;
    // CVI_U32 region_stRect_x5;
    // CVI_U32 region_stRect_y5;
    // CVI_U32 region_stRect_x6;
    // CVI_U32 region_stRect_y6;
    // CVI_BOOL bVpssPreProcSkip;
    float threshold;
    TDLModel model_id;
    char model_path[MODEL_PATH_LEN];
} APP_PARAM_AI_PD_CFG_S;

typedef struct APP_PARAM_AI_FD_CFG_T {
    CVI_BOOL FD_bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32GrpWidth;
	CVI_U32 u32GrpHeight;
    float threshold_fd;
    TDLModel model_id_fd;
    char model_path_fd[MODEL_PATH_LEN];
} APP_PARAM_AI_FD_CFG_S;

typedef struct APP_PARAM_AI_FACE_AE_CFG_T {
    CVI_BOOL bEnable;
    CVI_U32 Face_ae_restore_time;
	CVI_U32 Face_target_Luma;
    CVI_U32 Face_target_Luma_L_range;
    CVI_U32 Face_target_Luma_H_range;
    CVI_U32 Face_target_Evbias_L_range;
    CVI_U32 Face_target_Evbias_H_range;
    CVI_U32 AE_Channel_GB;
    CVI_U32 AE_Channel_B;
    CVI_U32 AE_Channel_GR;
    CVI_U32 AE_Channel_R;
    CVI_U32 AE_Grid_Row;
    CVI_U32 AE_Grid_Column;
    CVI_U32 Face_AE_Min_Cnt;
    CVI_U32 Face_Score_Threshold;
} APP_PARAM_AI_FACE_AE_CFG_S;

typedef struct APP_PARAM_AI_IR_FD_CFG_T {
    CVI_BOOL bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_S32 attachPoolId;
    CVI_BOOL bVpssPreProcSkip;
    char model_path_fd[MODEL_PATH_LEN];
    char model_path_ln[MODEL_PATH_LEN];
    char model_path_fr[MODEL_PATH_LEN];
} APP_PARAM_AI_IR_FD_CFG_S;

typedef struct APP_PARAM_AI_HUMAN_KEYPOINT_CFG_T {
    CVI_BOOL bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 model_size_w;
    CVI_U32 model_size_h;
    CVI_BOOL bVpssPreProcSkip;
    float threshold;
    TDLModel model_id;
    char model_path[MODEL_PATH_LEN];
    // cvtdl_service_brush_t rect_brush;
} APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S;

typedef struct APP_PARAM_AI_OBJECT_TRACK_CFG_T
{
    CVI_BOOL bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32GrpWidth;
    CVI_U32 u32GrpHeight;
    TDLModel model_id_det;
    TDLModel model_id_sot;
    char model_path_det[MODEL_PATH_LEN];
    char model_path_sot[MODEL_PATH_LEN];
    char model_path_sam[MODEL_PATH_LEN];
    char model_path_cfg[MODEL_PATH_LEN];
    float threshold_occluded;
    float threshold_reappear;
    TDLTargetSearchTypeE search_type;
    CVI_BOOL use_kalman;
    float tracking_score_threshold;
    CVI_BOOL debug_log_enable;
} APP_PARAM_AI_OBJECT_TRACK_CFG_S;

typedef struct APP_PARAM_AI_IMG_TXT_CLIP_CFG_T
{
    CVI_BOOL bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32GrpWidth;
    CVI_U32 u32GrpHeight;
    TDLModel model_id_img;
    TDLModel model_id_txt;
    char model_path_img[MODEL_PATH_LEN];
    char model_path_txt[MODEL_PATH_LEN];
    char model_path_cfg[MODEL_PATH_LEN];
    char txt_dir[MODEL_PATH_LEN];
} APP_PARAM_AI_IMG_TXT_CLIP_S;

typedef enum { DETECTION = 0, TRACKING = 1 } APP_PARAM_OBJECT_TRACK_MODE;

typedef struct APP_PARAM_AI_KEYPOINT_HAND_GESTURE_CFG_S {
    CVI_BOOL bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    float threshold;
    TDLModel detect_model_id;
    char detect_model_path[MODEL_PATH_LEN];
    TDLModel keypoint_model_id;
    char keypoint_model_path[MODEL_PATH_LEN];
    TDLModel classify_model_id;
    char classify_model_path[MODEL_PATH_LEN];
    char model_path_cfg[MODEL_PATH_LEN];
    CVI_BOOL bEnableClassification;  // 是否启用手势分类
} APP_PARAM_AI_KEYPOINT_HAND_GESTURE_CFG_S;

#ifdef LPR_SUPPORT
typedef struct APP_PARAM_AI_LPR_CFG_T {
    CVI_BOOL bEnable;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32GrpWidth;
    CVI_U32 u32GrpHeight;
    float threshold;
    TDLModel model_id_det;
    char model_path_det[MODEL_PATH_LEN];
    TDLModel model_id_kp;
    char model_path_kp[MODEL_PATH_LEN];
    TDLModel model_id_rec;
    char model_path_rec[MODEL_PATH_LEN];
} APP_PARAM_AI_LPR_CFG_S;
#endif

/* Personnel detection function */
APP_PARAM_AI_PD_CFG_S *app_ipcam_Ai_PD_Param_Get(void);
CVI_VOID app_ipcam_Ai_PD_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_PD_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_PD_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_PD_Pause_Get(void);
int app_ipcam_Ai_PD_Rect_Draw(VIDEO_FRAME_INFO_S *pstVencFrame);
int app_ipcam_Ai_PD_Start(void);
int app_ipcam_Ai_PD_Stop(void);
int app_ipcam_Ai_PD_ObjDrawInfo_Get(TDLObject *pstAiObj);
CVI_U32 app_ipcam_Ai_PD_ProcFps_Get(void);
CVI_S32 app_ipcam_Ai_PD_ProcTime_Get(void);
CVI_S32 app_ipcam_Pd_threshold_Set(float threshold);
CVI_S32 app_ipcam_Ai_Pd_Intrusion_Init(void);
CVI_U32 app_ipcam_Ai_PD_ProcIntrusion_Num_Get(void);
CVI_S32 app_ipcam_Ai_PD_StatusGet(void);

#ifdef FACE_SUPPORT
/* Face detection function */
APP_PARAM_AI_FD_CFG_S *app_ipcam_Ai_FD_Param_Get(void);
CVI_VOID app_ipcam_Ai_FD_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_FD_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_FD_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_FD_Pause_Get(void);
int app_ipcam_Ai_FD_Rect_Draw(VIDEO_FRAME_INFO_S *pstVencFrame);
int app_ipcam_Ai_FD_Start(void);
int app_ipcam_Ai_FD_Stop(void);
int app_ipcam_Ai_FD_ObjDrawInfo_Get(TDLFace *pstAiObj);
CVI_U32 app_ipcam_Ai_FD_ProcFps_Get(void);
CVI_S32 app_ipcam_Ai_FD_ProcTime_Get(void);
#endif

#ifdef IR_FACE_SUPPORT
/* IR Face detection function */
CVI_S32 app_ipcam_Ai_IR_FD_Start(void);
CVI_S32 app_ipcam_Ai_IR_FD_Stop(void);
APP_PARAM_AI_IR_FD_CFG_S * app_ipcam_Ai_IR_FD_Param_Get(void);
CVI_S32 app_ipcam_Ai_IR_FD_UnRegister(char * gallery_name);
CVI_S32 app_ipcam_Ai_IR_FD_Register(char * galler_name);
#endif

/* motion detection function */
APP_PARAM_AI_MD_CFG_S *app_ipcam_Ai_MD_Param_Get(void);
CVI_VOID app_ipcam_Ai_MD_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_MD_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_MD_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_MD_Pause_Get(void);
// int app_ipcam_Ai_MD_Rect_Draw(VIDEO_FRAME_INFO_S *pstVencFrame);
int app_ipcam_Ai_MD_Start(void);
int app_ipcam_Ai_MD_Stop(void);
int app_ipcam_Ai_MD_ObjDrawInfo_Get(TDLObject *pstMdObj);
CVI_U32 app_ipcam_Ai_MD_ProcFps_Get(void);
CVI_S32 app_ipcam_Ai_MD_ProcTime_Get(void);
CVI_VOID app_ipcam_Ai_MD_Thresold_Set(CVI_U32 value);
CVI_U32 app_ipcam_Ai_MD_Thresold_Get(void);
CVI_S32 app_ipcam_Ai_MD_StatusGet(void);
// CVI_VOID app_ipcam_Ai_MD_obj_Free(cvimd_object_t *pstMdObj);

/* baby_cry detection function */
#ifdef AUDIO_SUPPORT
typedef enum {
    BABY_CRY = 0,
    AUDIO_ORDER = 1,
}AI_AUDIO_APPLACATION_SCENE;

APP_PARAM_AI_CRY_CFG_S *app_ipcam_Ai_Cry_Param_Get(void);
CVI_VOID app_ipcam_Ai_Cry_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_Cry_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_Cry_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_Cry_Pause_Get(void);
int app_ipcam_Ai_Cry_Stop(void);
int app_ipcam_Ai_Cry_Start(void);
CVI_S32 app_ipcam_Ai_Cry_StatusGet(void);
#endif

#ifdef FACE_SUPPORT
/* face ae */
// CVI_VOID app_ipcam_Ai_FD_AEStart(VIDEO_FRAME_INFO_S *pstFrame, TDLFace *pstFace);
// /* face capture*/
// CVI_S32 app_ipcam_Ai_Face_Capture_Init(TDLHandle *handle);
// CVI_S32 app_ipcam_Ai_Face_Capture(VIDEO_FRAME_INFO_S *stfdFrame, TDLFace *capture_face);
// CVI_S32 app_ipcam_Ai_Face_Capture_Stop(void);
#endif

#ifdef HUMAN_KEYPOINT_SUPPORT
/* Human Keypoint Detection function */
APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S *app_ipcam_Ai_Human_Keypoint_Param_Get(void);
CVI_VOID app_ipcam_Ai_Human_Keypoint_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_Human_Keypoint_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_Human_Keypoint_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_Human_Keypoint_Pause_Get(void);
int app_ipcam_Ai_Human_Keypoint_Start(void);
int app_ipcam_Ai_Human_Keypoint_Stop(void);
int app_ipcam_Ai_Human_Keypoint_ObjDrawInfo_Get(TDLObject *pstAiObj);
CVI_S32 app_ipcam_Human_Keypoint_threshold_Set(float threshold);
CVI_S32 app_ipcam_Ai_Human_Keypoint_StatusGet(void);
#endif

#ifdef OBJECT_TRACK_SUPPORT
APP_PARAM_AI_OBJECT_TRACK_CFG_S *app_ipcam_Ai_Object_Track_Param_Get(void);
CVI_BOOL app_ipcam_Ai_Object_Track_Pause_Get(void);
CVI_VOID app_ipcam_Ai_Object_Track_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_Object_Track_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_BOOL flag);
int app_ipcam_Ai_Object_Track_Start(void);
int app_ipcam_Ai_Object_Track_Stop(void);
CVI_VOID app_ipcam_Ai_Object_Track_ObjDrawInfo_Get(TDLObject *pstAiObj);
CVI_VOID app_ipcam_Ai_Object_Track_DefaultBox_Get(int32_t box[4]);
APP_PARAM_OBJECT_TRACK_MODE app_ipcam_Ai_Object_Track_Mode_Get(void);
#endif

#ifdef KEYPOINT_HAND_GESTURE_SUPPORT
/* Keypoint Hand Gesture Detection function */
APP_PARAM_AI_KEYPOINT_HAND_GESTURE_CFG_S *app_ipcam_Ai_Keypoint_Hand_Gesture_Param_Get(void);
CVI_VOID app_ipcam_Ai_Keypoint_Hand_Gesture_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_Keypoint_Hand_Gesture_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_Keypoint_Hand_Gesture_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_Keypoint_Hand_Gesture_Pause_Get(void);
int app_ipcam_Ai_Keypoint_Hand_Gesture_Start(void);
int app_ipcam_Ai_Keypoint_Hand_Gesture_Stop(void);
int app_ipcam_Ai_Keypoint_Hand_Gesture_ObjDrawInfo_Get(TDLObject *pstAiObj);
CVI_S32 app_ipcam_Keypoint_Hand_Gesture_threshold_Set(float threshold);
CVI_S32 app_ipcam_Ai_Keypoint_Hand_Gesture_StatusGet(void);
#endif

#ifdef IMG_TXT_CLIP_SUPPORT
APP_PARAM_AI_IMG_TXT_CLIP_S *app_ipcam_Ai_Img_Txt_Clip_Param_Get(void);
CVI_BOOL app_ipcam_Ai_Img_Txt_Clip_Pause_Get(void);
CVI_VOID app_ipcam_Ai_Img_Txt_Clip_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_Img_Txt_Clip_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_Img_Txt_Clip_ProcStatus_Set(CVI_BOOL flag);
int app_ipcam_Ai_Img_Txt_Clip_Start(void);
int app_ipcam_Ai_Img_Txt_Clip_Stop(void);
CVI_VOID app_ipcam_Ai_Img_Txt_Clip_ObjDrawInfo_Get(TDLObject *pstAiObj);
#endif

#ifdef LPR_SUPPORT
/* License Plate Recognition function */
APP_PARAM_AI_LPR_CFG_S *app_ipcam_Ai_LPR_Param_Get(void);
CVI_VOID app_ipcam_Ai_LPR_ProcStatus_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_LPR_ProcStatus_Get(void);
CVI_VOID app_ipcam_Ai_LPR_Pause_Set(CVI_BOOL flag);
CVI_BOOL app_ipcam_Ai_LPR_Pause_Get(void);
int app_ipcam_Ai_LPR_Start(void);
int app_ipcam_Ai_LPR_Stop(void);
CVI_U32 app_ipcam_Ai_LPR_ProcFps_Get(void);
CVI_S32 app_ipcam_Ai_LPR_ProcTime_Get(void);
CVI_S32 app_ipcam_Ai_LPR_StatusGet(void);
CVI_S32 app_ipcam_Lpr_threshold_Set(float threshold);
#endif

/*****************************************************************
 *  The following API for command test used             S
 * **************************************************************/
int app_ipcam_CmdTask_Ai_PD_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
int app_ipcam_CmdTask_Ai_MD_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
#ifdef FACE_SUPPORT
int app_ipcam_CmdTask_Ai_FD_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
#endif
/*****************************************************************
 *  The above API for command test used                 E
 * **************************************************************/


#ifdef __cplusplus
}
#endif

#endif
