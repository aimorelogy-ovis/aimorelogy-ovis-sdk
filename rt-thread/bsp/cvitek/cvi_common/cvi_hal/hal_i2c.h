#ifndef __CVI_HAL_I2C_H__
#define __CVI_HAL_I2C_H__

#include <rtthread.h>
#include <drivers/dev_i2c.h>

#define CVI_I2C_I2C0 "i2c0"
#define CVI_I2C_I2C1 "i2c1"
#define CVI_I2C_I2C2 "i2c2"
#define CVI_I2C_I2C3 "i2c3"
#define CVI_I2C_I2C4 "i2c4"

#define CVI_I2C_WR          RT_I2C_WR          /*!< i2c wirte flag */
#define CVI_I2C_RD          RT_I2C_RD          /*!< i2c read flag  */
#define CVI_I2C_ADDR_10BIT  RT_I2C_ADDR_10BIT  /*!< this is a ten bit chip address */
#define CVI_I2C_NO_START    RT_I2C_NO_START    /*!< do not generate START condition */
#define CVI_I2C_IGNORE_NACK RT_I2C_IGNORE_NACK /*!< ignore NACK from slave */
#define CVI_I2C_NO_READ_ACK RT_I2C_NO_READ_ACK /* when I2C reading, we do not ACK */
#define CVI_I2C_NO_STOP     RT_I2C_NO_STOP     /*!< do not generate STOP condition */

#define CVI_I2C_DEV_CTRL_10BIT     RT_I2C_DEV_CTRL_10BIT
#define CVI_I2C_DEV_CTRL_ADDR      RT_I2C_DEV_CTRL_ADDR
#define CVI_I2C_DEV_CTRL_TIMEOUT   RT_I2C_DEV_CTRL_TIMEOUT
#define CVI_I2C_DEV_CTRL_RW        RT_I2C_DEV_CTRL_RW
#define CVI_I2C_DEV_CTRL_CLK       RT_I2C_DEV_CTRL_CLK
#define CVI_I2C_DEV_CTRL_UNLOCK    RT_I2C_DEV_CTRL_UNLOCK
#define CVI_I2C_DEV_CTRL_GET_STATE RT_I2C_DEV_CTRL_GET_STATE
#define CVI_I2C_DEV_CTRL_GET_MODE  RT_I2C_DEV_CTRL_GET_MODE
#define CVI_I2C_DEV_CTRL_GET_ERROR RT_I2C_DEV_CTRL_GET_ERROR


#define CVI_I2C_SPEED_STD  100 * 1000      //100 KHz
#define CVI_I2C_SPEED_FAST 400 * 1000      //400 KHz
#define CVI_I2C_SPEED_HIGH 4 * 1000 * 1000 //4 MHz

/*
 *	struct rt_i2c_msg {
 *		rt_uint16_t addr;
 *		rt_uint16_t flags;
 *		rt_uint16_t len;
 *		rt_uint8_t  *buf;
 *	};
 */
typedef struct rt_i2c_msg cvi_i2c_msg_t;

/**
 * @brief CVI I2C Master transfer
 *
 * @param name the I2C bus name. ex: CVI_I2C_I2C0, CVI_I2C_I2C1...
 * @param msgs the pointer  of the I2C transfer msgs
 * @param num the  number of the msgs
 *
 * @return the actual length of transmitted
 */
rt_ssize_t cvi_i2c_master_transfer(const char      *name,
                                   cvi_i2c_msg_t msgs[],
                                   uint32_t         count);

/**
 * @brief CVI I2C Control
 *
 * @param bus the I2C bus device
 * @param cmd the I2C control command
 * @param args the I2C control arguments
 *
 * @return rt_err_t error code
 */
rt_err_t cvi_i2c_control(const char *name,
                         int         cmd,
                         void       *args);

/**
 * @brief init cvi i2c
 */
int cvi_i2c_init(void);

#endif
