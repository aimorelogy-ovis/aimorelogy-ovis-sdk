#include <linux/module.h>
#include <linux/platform_device.h>
#include "osal.h"
#include "osal_inner.h"


static int ext_drv_osal_init(void)
{
	return OSAL_SUCCESS;
}

static void ext_drv_osal_exit(void)
{
    return;
}

static int osal_modules_init(void)
{
    int ret;

    ret = ext_drv_osal_init();
    if (ret != 0) {
        osal_log("common init failed! ret = %#x.\n", ret);
        goto err_osal_exit;
    }
    return 0;

err_osal_exit:
    ext_drv_osal_exit();

    return -1;
}

static void osal_modules_exit(void)
{
    ext_drv_osal_exit();

    return;
}

module_init(osal_modules_init);
module_exit(osal_modules_exit);

MODULE_AUTHOR("OSAL");
MODULE_LICENSE("GPL");
