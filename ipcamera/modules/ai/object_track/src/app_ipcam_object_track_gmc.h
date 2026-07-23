#ifndef __APP_IPCAM_OBJECT_TRACK_GMC_H__
#define __APP_IPCAM_OBJECT_TRACK_GMC_H__

#include "cvi_comm_video.h"
#include "cvi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_OBJECT_TRACK_GMC_GRID_WIDTH 96
#define APP_OBJECT_TRACK_GMC_GRID_HEIGHT 54

typedef struct APP_OBJECT_TRACK_GMC_RESULT_T {
    CVI_BOOL evaluated;
    CVI_BOOL valid;
    CVI_U64 frame_id;
    CVI_U32 frame_gap;
    CVI_FLOAT dx;
    CVI_FLOAT dy;
    CVI_FLOAT confidence;
    CVI_U32 sampled_points;
    CVI_U64 cache_us;
    CVI_U64 grid_us;
    CVI_U64 search_us;
    CVI_U64 total_us;
} APP_OBJECT_TRACK_GMC_RESULT_S;

typedef struct APP_OBJECT_TRACK_GMC_STATE_T {
    CVI_U8 previous[APP_OBJECT_TRACK_GMC_GRID_WIDTH *
                    APP_OBJECT_TRACK_GMC_GRID_HEIGHT];
    CVI_U8 current[APP_OBJECT_TRACK_GMC_GRID_WIDTH *
                   APP_OBJECT_TRACK_GMC_GRID_HEIGHT];
    CVI_BOOL has_previous;
    CVI_U64 previous_frame_id;
    CVI_S32 previous_exclusion[4];
} APP_OBJECT_TRACK_GMC_STATE_S;

CVI_VOID app_ipcam_ObjectTrackGmc_Reset(
    APP_OBJECT_TRACK_GMC_STATE_S *state);

CVI_S32 app_ipcam_ObjectTrackGmc_Process(
    APP_OBJECT_TRACK_GMC_STATE_S *state,
    const VIDEO_FRAME_INFO_S *frame,
    CVI_U64 frame_id,
    const CVI_S32 exclusion[4],
    APP_OBJECT_TRACK_GMC_RESULT_S *result);

#ifdef __cplusplus
}
#endif

#endif
