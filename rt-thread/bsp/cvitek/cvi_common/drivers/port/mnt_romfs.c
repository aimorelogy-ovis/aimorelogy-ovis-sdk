/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022/12/25     flyingcys    first version
 */
#include <rtthread.h>

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include "dfs_romfs.h"

#define DBG_TAG "app.filesystem"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static const struct romfs_dirent _romfs_root_mnt[] = {
    {ROMFS_DIRENT_DIR, "cfg", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "data", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "emmc", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "misc", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "sd", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "system", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "usb", RT_NULL, 0}
};

static const struct romfs_dirent _romfs_root[] = {
    {ROMFS_DIRENT_DIR, "dev", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "mnt", (rt_uint8_t *)_romfs_root_mnt, sizeof(_romfs_root_mnt)/sizeof(_romfs_root_mnt[0])}
};

const struct romfs_dirent romfs_root = {
    ROMFS_DIRENT_DIR, "/", (rt_uint8_t *)_romfs_root, sizeof(_romfs_root)/sizeof(_romfs_root[0])
};

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

static int sd_mount(const char* devname)
{
    if (!wait_device_ready(devname)) {
        LOG_W("Failed to find device: %s", devname);
        return -RT_EINVAL;
    }

    if (dfs_mount(devname, "/mnt/sd", "ext", 0, 0) == RT_EOK) {
        LOG_I("device '%s' is mounted to '/mnt/sd' as EXT", devname);
    } else if (dfs_mount(devname, "/mnt/sd", "elm", 0, 0) == RT_EOK) {
        LOG_I("device '%s' is mounted to '/mnt/sd' as FAT", devname);
    } else {
        LOG_W("Failed to mount device '%s' to '/mnt/sd': %d\n", devname, rt_get_errno());
        return -RT_ERROR;
    }
    return RT_EOK;
}

int mount_init(void)
{
    if(dfs_mount(RT_NULL, "/", "rom", 0, &romfs_root) != 0)
    {
        LOG_E("rom mount to '/' failed!");
    }

#ifdef BSP_USING_ON_CHIP_FLASH_FS
    struct rt_device *flash_dev = RT_NULL;

    /* 使用 filesystem 分区创建块设备，块设备名称为 filesystem */
    flash_dev = fal_blk_device_create("filesystem");
    if(flash_dev == RT_NULL)
    {
        LOG_E("Failed to create device.\n");
        return -RT_ERROR;
    }

    if (dfs_mount("filesystem", "/flash", "lfs", 0, 0) != 0)
    {
        LOG_I("file system initialization failed!\n");
        if(dfs_mkfs("lfs", "filesystem") == 0)
        {
            if (dfs_mount("filesystem", "/flash", "lfs", 0, 0) == 0)
            {
                LOG_I("mount to '/flash' success!");
            }
        }
    }
    else
    {
        LOG_I("mount to '/flash' success!");
    }
#endif

#ifdef BSP_USING_SDH
    int ret;
    ret = sd_mount("sd0");
    if (ret != RT_EOK) {
        LOG_D("sd0 mount failed: %d; Maybe SD do not have valid partition table!", ret);
        LOG_D("Try to mount without partition table");
        ret = sd_mount("sd");
        return ret;
    }
#endif

    return RT_EOK;
}
INIT_APP_EXPORT(mount_init);

#endif /* RT_USING_DFS */