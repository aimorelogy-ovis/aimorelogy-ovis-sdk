#if defined(ENABLE_FACE_AE) || defined(ENABLE_TEAISP_PQ)

#include <dlfcn.h>
#include "daemon_base.h"
#include "tdl_sdk_api.h"

static int tdl_sdk_lib_ref_count;
static tdl_sdk_api_t *tdl_sdk_api;

#ifndef TDL_SDK_LIB
#define TDL_SDK_LIB "NULL"
#endif
static void *dl;

#define LOAD_SYMBOL(dl, sym, type, fn)                                         \
	do {                                                                   \
		if (NULL == fn && NULL == (fn = (type)dlsym(dl, sym))) {       \
			clog_a("load symbol %s fail, %s\n", sym, dlerror());   \
			goto err;                                              \
		}                                                              \
	} while (0);

static int load_tdl_so(void)
{
	dl = dlopen(TDL_SDK_LIB, RTLD_LAZY);
	if (dl == NULL) {
		clog_a("dlopen [%s] failed: %s\n", TDL_SDK_LIB, dlerror());
		return -1;
	}

	LOAD_SYMBOL(dl, "TDL_CreateHandle", create_handle_t,
		    tdl_sdk_api->create_handle);
	LOAD_SYMBOL(dl, "TDL_DestroyHandle", destroy_handle_t,
		    tdl_sdk_api->destroy_handle);
	LOAD_SYMBOL(dl, "TDL_OpenModel", open_model_t,
		    tdl_sdk_api->open_model);
	LOAD_SYMBOL(dl, "TDL_CloseModel", close_model_t,
		    tdl_sdk_api->close_model);
	LOAD_SYMBOL(dl, "TDL_WrapFrame", wrap_vpss_frame_t,
		    tdl_sdk_api->wrap_vpss_frame);
	LOAD_SYMBOL(dl, "TDL_DestroyImage", free_vpss_frame_t,
		    tdl_sdk_api->free_vpss_frame);
	LOAD_SYMBOL(dl, "TDL_FaceDetection", face_detection_t,
		    tdl_sdk_api->face_detection);
	LOAD_SYMBOL(dl, "TDL_ReleaseFaceMeta", free_face_meta_t,
		    tdl_sdk_api->free_face_meta);
	LOAD_SYMBOL(dl, "TDL_IspClassification", isp_image_classification_t,
		    tdl_sdk_api->isp_image_classification);
	LOAD_SYMBOL(dl, "TDL_ReleaseClassMeta", free_class_meta_t,
		    tdl_sdk_api->free_class_meta);
	LOAD_SYMBOL(dl, "TDL_FaceDrawRect", service_face_draw_rect_t,
		    tdl_sdk_api->service_face_draw_rect);
	LOAD_SYMBOL(dl, "TDL_ObjectWriteText", service_object_write_text_t,
		    tdl_sdk_api->service_object_write_text);

	return 0;
err:
	dlclose(dl);
	dl = NULL;
	return -1;
}

static int unload_tdl_so(void)
{
	dlclose(dl);
	dl = NULL;
	return 0;
}

int load_tdl_sdk_lib(void)
{
	if (tdl_sdk_api == NULL) {
		tdl_sdk_api = (tdl_sdk_api_t *)calloc(1, sizeof(tdl_sdk_api_t));
		if (tdl_sdk_api == NULL) {
			clog_a("malloc failed\n");
			return -1;
		}
		load_tdl_so();
	}
	tdl_sdk_lib_ref_count++;
	return 0;
}

int unload_tdl_sdk_lib(void)
{
	if (tdl_sdk_lib_ref_count > 0) {
		tdl_sdk_lib_ref_count--;
	}
	if (tdl_sdk_lib_ref_count == 0) {
		unload_tdl_so();
		free(tdl_sdk_api);
		tdl_sdk_api = NULL;
	}
	return 0;
}

tdl_sdk_api_t *get_tdl_sdk_api(void)
{
	if (tdl_sdk_api == NULL) {
		clog_a("tdl_sdk_api is not initialized\n");
		return NULL;
	}
	return tdl_sdk_api;
}
#endif
