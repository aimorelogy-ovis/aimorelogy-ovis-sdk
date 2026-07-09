#ifndef MODULES_VPU_INCLUDE_LDC_IOCTL_H_
#define MODULES_VPU_INCLUDE_LDC_IOCTL_H_

#include <sys/ioctl.h>

#include "ldc_uapi.h"

/* Configured from user  */
CVI_S32 gdc_init(CVI_S32 fd);
CVI_S32 gdc_deinit(CVI_S32 fd);

CVI_S32 gdc_begin_job(CVI_S32 fd, struct gdc_handle_data *cfg);
CVI_S32 gdc_end_job(CVI_S32 fd, struct gdc_handle_data *cfg);
CVI_S32 gdc_cancel_job(CVI_S32 fd, struct gdc_handle_data *cfg);
CVI_S32 gdc_add_rotation_task(CVI_S32 fd, struct gdc_task_attr *attr);
CVI_S32 gdc_add_ldc_task(CVI_S32 fd, struct gdc_task_attr *attr);
CVI_S32 gdc_set_job_identity(CVI_S32 fd, struct gdc_identity_attr *indentity);
CVI_S32 gdc_get_work_job(CVI_S32 fd, struct gdc_handle_data *cfg);
CVI_S32 gdc_get_chn_frm(CVI_S32 fd, struct gdc_chn_frm_cfg *cfg);
CVI_S32 gdc_suspend(CVI_S32 fd);
CVI_S32 gdc_resume(CVI_S32 fd);
CVI_S32 gdc_get_internal_chn_attr(CVI_S32 fd, struct ldc_internal_chn_attr *attr);
CVI_S32 gdc_set_internal_chn_ldc_cfg(CVI_S32 fd, struct ldc_internal_chn_ldc_cfg *cfg);
CVI_S32 gdc_attach_vbpool(CVI_S32 fd, struct ldc_vb_pool_cfg *cfg);
CVI_S32 gdc_detach_vbpool(CVI_S32 fd, struct ldc_vb_pool_cfg *cfg);

#endif /* MODULES_VPU_INCLUDE_LDC_IOCTL_H_ */
