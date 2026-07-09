#ifndef __OSAL_CLK_H__
#define __OSAL_CLK_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct osal_clk_ {
    void *clk;
} osal_clk;

typedef struct osal_clk_ctrl_info_ {
	char name[32];
	unsigned long reg_addr;
	unsigned int bit;
} osal_clk_ctrl_info;


/**
 * @ingroup osal_clk
 * @brief This API is register clock control info.
 *
 * @par Description:
 * This API is register clock control info.
 *
 * @param info [in] clock control info.
 *
 * @return OSAL_FAILURE/OSAL_SUCCESS.
 *
 * @par Support System:
 * alios
 */
int osal_clk_register(osal_clk_ctrl_info *info);

/**
 * @ingroup osal_clk
 * @brief This API is unregister clock control info.
 *
 * @par Description:
 * This API is unregister clock control info.
 *
 * @param info [in] clock control info.
 *
 * @par Support System:
 * alios
 */
void osal_clk_unregister(osal_clk_ctrl_info *info);

/**
 * @ingroup osal_clk
 * @brief This API is get clock.
 *
 * @par Description:
 * This API is get clock.
 *
 * @param device [in] linux device, alios is NULL.
 * @param name [in] clk name.
 * @return clk
 *
 * @par Support System:
 * linux alios
 */
osal_clk *osal_clk_get(void *device, const char *name);

/**
 * @ingroup osal_clk
 * @brief This API is free clock source.
 *
 * @par Description:
 * This API is free clock source.
 *
 * @param device [in] linux device, alios is NULL.
 * @param clk [in] clk.
 *
 * @par Support System:
 * linux alios
 */
void osal_clk_put(void *device, osal_clk *clk);

/**
 * @ingroup osal_clk
 * @brief prepare clock
 *
 * @par Description:
 * prepare clock
 *
 * @param clk [in] clk.
 *
 * @par Support System:
 * linux alios.
 */
void osal_clk_prepare(osal_clk *clk);

/**
 * @ingroup osal_clk
 * @brief unprepare clock
 *
 * @par Description:
 * unprepare clock
 *
 * @param clk [in] clk.
 *
 * @par Support System:
 * linux alios.
 */
void osal_clk_unprepare(osal_clk *clk);

/**
 * @ingroup osal_clk
 * @brief enable clock
 *
 * @par Description:
 * enable clock
 *
 * @param clk [in] clk.
 *
 * @par Support System:
 * linux alios.
 */
void osal_clk_enable(osal_clk *clk);

/**
 * @ingroup osal_clk
 * @brief disable clock
 *
 * @par Description:
 * disable clock
 *
 * @param clk [in] clk.
 *
 * @par Support System:
 * linux alios.
 */
void osal_clk_disable(osal_clk *clk);

/**
 * @ingroup osal_clk
 * @brief prepare and enable clock
 *
 * @par Description:
 * prepare and enable clock
 *
 * @param clk [in] clk.
 *
 * @par Support System:
 * linux alios.
 */
void osal_clk_prepare_enable(osal_clk *clk);

/**
 * @ingroup osal_clk
 * @brief disable and unprepare clock
 *
 * @par Description:
 * disable and unprepare clock
 *
 * @param clk [in] clk.
 *
 * @par Support System:
 * linux alios.
 */
void osal_clk_disable_unprepare(osal_clk *clk);

/**
 * @ingroup osal_clk
 * @brief Determine whether it is enabled
 *
 * @par Description:
 * Determine whether it is enabled
 *
 * @param clk [in] clk.
 *
 * @par Support System:
 * linux alios.
 */
bool osal_clk_is_enabled(osal_clk *clk);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* __OSAL_CLK_H__ */
