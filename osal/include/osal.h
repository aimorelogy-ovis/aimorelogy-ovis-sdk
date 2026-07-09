#ifndef __OSAL_H__
#define __OSAL_H__

#include "osal_types.h"
#include "osal_def.h"
#include "osal_list.h"
#include "osal_errno.h"
#include "module/osal_atomic.h"
#include "module/osal_debug.h"
#include "module/osal_interrupt.h"
#include "module/osal_mutex.h"
#include "module/osal_task.h"
#include "module/osal_addr.h"
#include "module/osal_barrier.h"
#include "module/osal_device.h"
#include "module/osal_fileops.h"
#include "module/osal_math.h"
#include "module/osal_proc.h"
#include "module/osal_semaphore.h"
#include "module/osal_spinlock.h"
#include "module/osal_string.h"
#include "module/osal_timer.h"
#include "module/osal_rtc.h"
#include "module/osal_wait.h"
#include "module/osal_workqueue.h"
#include "module/osal_bitmap.h"
#include "module/osal_cache.h"
#include "module/osal_completion.h"
#include "module/osal_delaywork.h"
#include "module/osal_drvbox.h"
#include "module/osal_event.h"
#include "module/osal_msgqueue.h"
#include "module/osal_rwlock.h"
#include "module/osal_hash.h"
#include "module/osal_clk.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OSAL_SUCCESS 0
#define OSAL_FAILURE (-1)

#define ERESTARTSYS 512

/* dts */
int osal_platform_driver_register(void *drv);
void osal_platform_driver_unregister(void *drv);
void *osal_platform_get_resource_byname(void *dev, unsigned int type, const char *name);
void *osal_platform_get_resource(void *dev, unsigned int type, unsigned int num);
int osal_platform_get_irq(void *dev, unsigned int num);
int osal_platform_get_irq_byname(void *dev, const char *name);

#define osal_module_driver(osal_driver, osal_register, osal_unregister, ...) \
    static int __init osal_driver##_init(void)                               \
    {                                                                        \
        return osal_register(&(osal_driver));                                \
    }                                                                        \
    module_init(osal_driver##_init);                                         \
    static void __exit osal_driver##_exit(void)                              \
    {                                                                        \
        osal_unregister(&(osal_driver));                                     \
    }                                                                        \
    module_exit(osal_driver##_exit);

#define osal_module_platform_driver(platform_driver)                   \
    osal_module_driver(platform_driver, osal_platform_driver_register, \
        osal_platform_driver_unregister)

#ifndef MODULE
#define osal_setup_str_param(param, value)                                   \
static int __init parse_kern_str_##param(char *line)                         \
{                                                                            \
    strncpy((value), line, sizeof(value) - 1); \
    return 0;                                                                \
}                                                                            \
__setup(#param"=", parse_kern_str_##param)

#define osal_setup_num_param(param, value)                  \
static int __init parse_kern_num_##param(char *line)        \
{                                                           \
    (value) = (typeof(value))simple_strtol(line, NULL, 10); \
    return 0;                                               \
}                                                           \
__setup(#param"=", parse_kern_num_##param)
#endif

/* reboot */
struct osal_notifier_block {
    int (*notifier_call)(struct osal_notifier_block *nb, unsigned long action, void *data);
    void *notifier_block;
};

int osal_register_reboot_notifier(struct osal_notifier_block *ob);
int osal_unregister_reboot_notifier(struct osal_notifier_block *nb);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* __OSAL_H__ */
