#ifndef __OSAL_DEBUG_H__
#define __OSAL_DEBUG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OSAL_BUG() \
    do {           \
    } while (1)

#define OSAL_ASSERT(expr)                       \
    do {                                        \
        if (!(expr)) {                          \
            osal_printk("\nASSERT failed at:\n" \
                "  >Condition: %s\n",           \
                #expr);                         \
            OSAL_BUG();                         \
        }                                       \
    } while (0)


#define OSAL_BUG_ON(expr)                                                  \
    do {                                                                   \
        if (expr) {                                                        \
            osal_printk("BUG: failure at %d/%s()!\n", __LINE__, __func__); \
            OSAL_BUG();                                                    \
        }                                                                  \
    } while (0)


/**
 * @ingroup osal_debug
 * @brief Log printing function.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void osal_printk(const char *fmt, ...) __attribute__((__format__ (__printf__, 1, 2)));

/**
 * @ingroup osal_debug
 * @brief Log printing function.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void osal_printf(const char *fmt, ...) __attribute__((__format__ (__printf__, 1, 2)));

/**
 * @ingroup osal_debug
 * @brief Kernel panic function.
 *
 * @par Description:
 * Stack function that prints kernel panics.
 * After this function is called and stack information is printed, the system will fail to respond.
 *
 * @par Support System:
 * linux.
 */
void osal_panic(const char *fmt, const char *fun, int line, const char *cond);

/**
 * @ingroup osal_debug
 * @brief Kernel backtrace function.
 *
 * @par Description:
 * Backtrace function that prints task call stack information traced from the running task.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void osal_dump_stack(void);
/**
 * @ingroup osal_debug
 * @brief Choose printk function according to the configuration.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void osal_printk_auto(const char *fmt, ...) __attribute__((__format__ (__printf__, 1, 2)));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* __OSAL_DEBUG_H__ */
