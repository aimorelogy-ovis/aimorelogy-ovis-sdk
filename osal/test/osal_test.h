#ifndef __OSAL_TEST_H__
#define __OSAL_TEST_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "osal.h"


#ifndef __KERNEL__
#include <rtthread.h>
#include <stdio.h>
#include <dfs_posix.h>
#include <string.h>
#include <time.h>
#define OSAL_CMD_EXPORT(...) MSH_CMD_EXPORT(__VA_ARGS__)
#elif defined(__ALIOS__)
#define OSAL_CMD_EXPORT(...) ALIOS_CLI_CMD_REGISTER(__VA_ARGS__)
#else
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#define OSAL_CMD_EXPORT(...)
#endif

#define osal_test_log(fmt, ...) osal_printk("[%s:%d]:" fmt, __func__, __LINE__, ##__VA_ARGS__)

void osal_addr_test(void);
void osal_atomic_test(void);
void osal_atomic_multi_test(void);
void osal_bitmap_test(void);
void osal_completion_test(void);
void osal_completion_multi_test(void);
void osal_delaywork_test(void);
void osal_hash_test(void);
void osal_math_test(void);
void osal_interrupt_test(void);
void osal_mutex_test(void);
void osal_proc_test(void);
void osal_rwlock_test(void);
void osal_rwlock_multi_test(void);
void osal_semaphore_test(void);
void osal_spinlock_test(void);
void osal_string_test(void);
void osal_task_test(void);
void osal_timer_test(void);
void osal_wait_test(void);
void osal_workqueue_test(void);
void osal_fileops_test(void);
void osal_cache_test(void);
void osal_test_all(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* __OSAL_TEST_H__ */
