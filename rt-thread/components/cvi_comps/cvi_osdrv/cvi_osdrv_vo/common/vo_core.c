#include <vo_core.h>
#include <base_cb.h>

#include "scaler.h"
#include "vo_common.h"

#define CVI_VO_IRQ_NAME            "sc"
#define CVI_VO_DEV_NAME            "cvi-vo"
#define CVI_VO_CLASS_NAME          "cvi-vo"

static struct cvi_vo_dev *gVdev;

#if 0
static long vo_core_ioctl(struct file *filp, u_int cmd, u_long arg)
{
	long ret = 0;

	ret = vo_ioctl(filp, cmd, arg);
	return ret;
}
#endif

static int vo_core_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg)
{
	return vo_cb(dev, caller, cmd, arg);
}

static int vo_core_rm_cb(void)
{
	return base_rm_module_cb(E_MODULE_VO);
}

static int vo_core_register_cb(struct cvi_vo_dev *dev)
{
	struct base_m_cb_info reg_cb;

	reg_cb.module_id	= E_MODULE_VO;
	reg_cb.dev		= (void *)dev;
	reg_cb.cb		= vo_core_cb;

	return base_reg_module_cb(&reg_cb);
}

int vo_core_init(void)
{
	int ret = 0;

	gVdev = malloc(sizeof(struct cvi_vo_dev));
	memset(gVdev, 0, sizeof(struct cvi_vo_dev));
	if (!gVdev) {
		CVI_TRACE_VO(CVI_VO_ERR, "fail to malloc!\n");
		return -ENOMEM;
	}
	ret = vo_create_instance(gVdev);
	if (ret) {
		CVI_TRACE_VO(CVI_VO_ERR, "Failed to create instance, err %d\n", ret);
		return ret;
	}

	ret = vo_core_register_cb(gVdev);
	if (ret) {
		CVI_TRACE_VO(CVI_VO_ERR, "Failed to register vo cb, err %d\n", ret);
		return ret;
	}

	return ret;
}

int vo_core_deinit(void)
{
	int ret = 0;
	struct cvi_vo_dev *dev = gVdev;

	ret = vo_destroy_instance(dev);
	if (ret) {
		CVI_TRACE_VO(CVI_VO_ERR, "Failed to destroy vo instance, err %d\n", ret);
		goto err_destroy_instance;
	}

	ret = vo_core_rm_cb();
	if (ret) {
		CVI_TRACE_VO(CVI_VO_ERR, "Failed to rm vo cb, err %d\n", ret);
	}

err_destroy_instance:

	return ret;
}


