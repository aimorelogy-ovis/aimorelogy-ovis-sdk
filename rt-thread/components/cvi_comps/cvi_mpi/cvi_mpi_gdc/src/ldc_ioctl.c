#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "ldc_ioctl.h"
#include "list.h"
#include "ldc_platform.h"

CVI_S32 cvi_gdc_begin_job(CVI_S32 fd, struct gdc_handle_data *cfg)
{
    return ldc_ioctl(CVI_LDC_BEGIN_JOB, (void *)cfg);
}

CVI_S32 cvi_gdc_end_job(CVI_S32 fd, struct gdc_handle_data *cfg)
{
    return ldc_ioctl(CVI_LDC_END_JOB, (void *)cfg);
}

CVI_S32 cvi_gdc_cancel_job(CVI_S32 fd, struct gdc_handle_data *cfg)
{
    return ldc_ioctl(CVI_LDC_CANCEL_JOB, (void *)cfg);
}

CVI_S32 cvi_gdc_add_rotation_task(CVI_S32 fd, struct gdc_task_attr *attr)
{
    return ldc_ioctl(CVI_LDC_ADD_ROT_TASK, (void *)attr);
}

CVI_S32 cvi_gdc_add_ldc_task(CVI_S32 fd, struct gdc_task_attr *attr)
{
    return ldc_ioctl(CVI_LDC_ADD_LDC_TASK, (void *)attr);
}

CVI_S32 cvi_gdc_set_chn_buf_wrap(CVI_S32 fd, const struct ldc_buf_wrap_cfg *cfg)
{
    return ldc_ioctl(CVI_LDC_SET_BUF_WRAP, (void *)cfg);
}

CVI_S32 cvi_gdc_get_chn_buf_wrap(CVI_S32 fd, struct ldc_buf_wrap_cfg *cfg)
{
    return ldc_ioctl(CVI_LDC_GET_BUF_WRAP, (void *)cfg);
}
