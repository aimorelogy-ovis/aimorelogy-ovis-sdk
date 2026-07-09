#ifndef __PLATFORM_MIPI_H__
#define __PLATFORM_MIPI_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sensor_cfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */



/**
 * @brief Set the MIPI reset signal.
 *
 * This function sets the MIPI reset signal based on the provided device number and reset value.
 *
 * @param devno Device number.
 * @param reset Reset value.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_mipi_SetMipiReset(CVI_S32 devno, CVI_U32 reset);

/**
 * @brief Set the sensor clock.
 *
 * This function enables or disables the sensor clock for the specified device.
 *
 * @param devno Device number.
 * @param enable Enable flag (non-zero to enable, zero to disable).
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_mipi_SetSensorClock(CVI_S32 devno, CVI_U32 enable);

/**
 * @brief Set the sensor reset parameters.
 *
 * This function configures the sensor reset parameters including port, pin, polarity, and enable state.
 *
 * @param devno Device number.
 * @param reset_port Reset port.
 * @param reset_pin Reset pin.
 * @param reset_pol Reset polarity.
 * @param reset_enable Reset enable flag (non-zero to enable, zero to disable).
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_mipi_SetSensorReset(CVI_S32 devno, CVI_U32 reset_port, CVI_U32 reset_pin, CVI_U32 reset_pol, CVI_U32 reset_enable);

/**
 * @brief Set the MIPI device attributes.
 *
 * This function configures the MIPI device attributes using the provided attribute structure.
 *
 * @param ViPipe Virtual pipeline identifier.
 * @param devAttr Pointer to the device attribute structure.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_mipi_SetMipiAttr(CVI_S32 ViPipe, const CVI_VOID *devAttr);

/**
 * @brief Set the clock edge for MIPI communication.
 *
 * This function configures which clock edge (rising or falling) is used for MIPI communication based on the provided flag.
 *
 * @param devno Device number.
 * @param is_up If non-zero, use rising edge; if zero, use falling edge.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_mipi_SetClkEdge(CVI_S32 devno, CVI_U32 is_up);

/**
 * @brief Set the sensor master clock configuration.
 *
 * This function sets the master clock (MCLK) configuration for the sensor based on the provided MCLK PLL settings.
 *
 * @param mclk Pointer to the MCLK PLL configuration structure.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_mipi_SetSnsMclk(struct mclk_pll_s *mclk);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__PLATFORM_SENSOR_H__ */
