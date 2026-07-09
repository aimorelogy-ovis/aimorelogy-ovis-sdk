/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <rtthread.h>

#ifdef RT_USING_DFS

#include <dfs_fs.h>
#include <rtdbg.h>

#define DBG_TAG "app.filesystem"
#define DBG_LVL DBG_LOG

/**
 * @brief wait device ready in 50ms * 10 times
 *
 * @param devname: pointer to device name
 * @return 1: device ready, 0: device not ready
 */
static int wait_device_ready(const char* devname)
{
    for (int i = 0; i < 10; i++) {
        if (rt_device_find(devname) != RT_NULL) {
            return 1;
        }
        rt_thread_mdelay(50);
    }

    return 0;
}

/**
 * @brief mount sd card to '/'
 * @note only support ext and fat file system
 *
 * @param devname: pointer to device name
 */
static void sd_mount(const char* devname)
{
    if (!wait_device_ready(devname)) {
        LOG_W("Failed to find device: %s", devname);
        return;
    }

    if (dfs_mount(devname, "/", "ext", 0, 0) == RT_EOK) {
        LOG_I("device '%s' is mounted to '/' as EXT", devname);
    } else if (dfs_mount(devname, "/", "elm", 0, 0) == RT_EOK) {
        LOG_I("device '%s' is mounted to '/' as FAT", devname);
    } else {
        LOG_W("Failed to mount device '%s' to '/': %d\n", devname, rt_get_errno());
    }
}

/**
 * @brief mount sd0 to '/'
 *
 * @return RT_EOK: success, other: failed
 */
int mount_init(void)
{
#ifdef BSP_USING_SDH
    sd_mount("sd0");
#endif

    return RT_EOK;
}
INIT_ENV_EXPORT(mount_init);

#endif /* RT_USING_DFS */
