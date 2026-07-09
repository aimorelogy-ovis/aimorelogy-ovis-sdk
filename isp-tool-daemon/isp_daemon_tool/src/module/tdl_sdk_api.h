
#ifndef __TDL_SDK_API_H_
#define __TDL_SDK_API_H_

#include "cvi_type.h"
#include "cvi_comm_video.h"
#include <c_apis/tdl_sdk.h>
#include <c_apis/tdl_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

#define topK 5
#define TDL_SUPPORTED_MODEL_FACE TDL_MODEL_SCRFD_DET_FACE
#define TDL_SUPPORTED_MODEL_CLASSIFICATION TDL_MODEL_CLS_ISP_SCENE

typedef TDLHandle (*create_handle_t)(const int32_t device_id);
typedef CVI_S32 (*destroy_handle_t)(TDLHandle handle);
typedef CVI_S32 (*open_model_t)(TDLHandle handle,
				TDLModel config,
				const char *filepath,
				const char *model_config_json);
typedef CVI_S32 (*close_model_t)(TDLHandle handle,
				TDLModel config);
typedef CVI_S32 (*face_detection_t)(TDLHandle handle,
				    TDLModel model_index,
					TDLImage *frame,
				    TDLFace *face_meta);
typedef CVI_S32 (*free_face_meta_t)(TDLFace *face_meta);
typedef CVI_S32 (*isp_image_classification_t)(TDLHandle handle,
					      TDLModel model_index,
						  TDLImage *frame,
						  TDLIspMeta *isp_meta,
					      TDLClass *obj_meta);
typedef CVI_S32 (*free_class_meta_t)(TDLClass *cls_meta);
typedef TDLImage (*wrap_vpss_frame_t)(void *vpss_frame, bool own_memory);
typedef CVI_S32 (*free_vpss_frame_t)(TDLImage image_handle);

typedef CVI_S32 (*service_face_draw_rect_t)(const TDLFace *meta,
					    VIDEO_FRAME_INFO_S *frame,
					    const bool drawText,
					    TDLBrush brush);
typedef CVI_S32 (*service_object_write_text_t)(char *name, int x, int y,
					       VIDEO_FRAME_INFO_S *frame,
					       float r, float g, float b);

typedef struct {
	create_handle_t create_handle;
	destroy_handle_t destroy_handle;
	open_model_t open_model;
	close_model_t close_model;
	face_detection_t face_detection;
	free_face_meta_t free_face_meta;
	isp_image_classification_t isp_image_classification;
	free_class_meta_t free_class_meta;
	wrap_vpss_frame_t wrap_vpss_frame;
	free_vpss_frame_t free_vpss_frame;

	service_face_draw_rect_t service_face_draw_rect;
	service_object_write_text_t service_object_write_text;
} tdl_sdk_api_t;

int load_tdl_sdk_lib(void);
int unload_tdl_sdk_lib(void);

tdl_sdk_api_t *get_tdl_sdk_api(void);

#ifdef __cplusplus
}
#endif
#endif // __TDL_SDK_API_H_
