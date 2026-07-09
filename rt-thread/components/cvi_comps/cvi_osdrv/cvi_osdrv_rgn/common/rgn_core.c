#include <stdlib.h>
#include <errno.h>
#include <rgn_core.h>
#include <base_cb.h>

#define CVI_RGN_DEV_NAME            "cvi-rgn"

static struct cvi_rgn_dev *gRdev;

static int rgn_core_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg)
{
	return rgn_cb(dev, caller, cmd, arg);
}

static int rgn_core_rm_cb(void)
{
	return base_rm_module_cb(E_MODULE_RGN);
}

static int rgn_core_register_cb(struct cvi_rgn_dev *dev)
{
	struct base_m_cb_info reg_cb;

	reg_cb.module_id	= E_MODULE_RGN;
	reg_cb.dev		= (void *)dev;
	reg_cb.cb		= rgn_core_cb;

	return base_reg_module_cb(&reg_cb);
}

// static int cvi_rgn_probe(struct platform_device *pdev)
int rgn_core_init(void)
{
	int ret = 0;

	gRdev = malloc(sizeof(struct cvi_rgn_dev));
	if (!gRdev) {
		CVI_TRACE_RGN(RGN_ERR, "Failed to malloc size(%d)!\n", (int)sizeof(struct cvi_rgn_dev));
		return -ENOMEM;
	}

	/* initialize locks */
	spin_lock_init(&gRdev->lock);
	pthread_mutex_init(&gRdev->mutex, NULL);

	ret = rgn_create_instance(gRdev);
	if (ret) {
		CVI_TRACE_RGN(RGN_ERR, "Failed to create instance, err %d\n", ret);
		goto err_create_instance;
	}

	if (rgn_core_register_cb(gRdev)) {
		CVI_TRACE_RGN(RGN_ERR, "Failed to register rgn cb, err %d\n", ret);
		goto err_create_instance;
	}

	CVI_TRACE_RGN(RGN_INFO, "done with ret(%d).\n", ret);
	return ret;

err_create_instance:
	CVI_TRACE_RGN(RGN_INFO, "failed with rc(%d).\n", ret);
	return ret;
}

int rgn_core_deinit(void)
{
	int ret = 0;

	ret = rgn_destroy_instance(gRdev);
	if (ret) {
		CVI_TRACE_RGN(RGN_ERR, "Failed to destroy instance, err %d\n", ret);
		goto err_destroy_instance;
	}

	if (rgn_core_rm_cb()) {
		CVI_TRACE_RGN(RGN_ERR, "Failed to rm rgn cb, err %d\n", ret);
	}

err_destroy_instance:
	CVI_TRACE_RGN(RGN_INFO, "%s -\n", __func__);

	return ret;
}

