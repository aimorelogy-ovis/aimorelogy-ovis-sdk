#ifndef __HAL_SPI_H__
#define __HAL_SPI_H__

#include <stdint.h>

#include "hal_debug.h"
#include "hal_def.h"
#include "hal_dma.h"

#define spi_log printf
#define spi_err printf
// #define SPI_DEBUG
#ifdef SPI_DEBUG
#define spi_dbg printf
#else
#define spi_dbg(...)
#endif

enum transfer_type {
    POLL_TRAN = 0,
    IRQ_TRAN,
    DMA_TRAN,
};

typedef enum {
    SPI_MASTER,  ///< SPI Master (Output on MOSI, Input on MISO); arg = Bus Speed in bps
    SPI_SLAVE,   ///< SPI Slave  (Output on MISO, Input on MOSI)
} hal_spi_mode_t;

typedef enum {
    SPI_EVENT_SEND_COMPLETE,  ///< Data Send completed. Occurs after call to csi_spi_send_async to
                              ///< indicate that all the data has been send over
    SPI_EVENT_RECEIVE_COMPLETE,  ///< Data Receive completed. Occurs after call to
                                 ///< csi_spi_receive_async to indicate that all the data has been
                                 ///< received
    SPI_EVENT_SEND_RECEIVE_COMPLETE,  ///< Data Send_receive completed. Occurs after call to
                                      ///< csi_spi_send_receive_async to indicate that all the data
                                      ///< has been send_received
    SPI_EVENT_ERROR_OVERFLOW,         ///< Data overflow: Receive overflow
    SPI_EVENT_ERROR_UNDERFLOW,        ///< Data underflow: Transmit underflow
    SPI_EVENT_ERROR  ///< Master Mode Fault (SS deactivated when Master).Occurs in master mode when
                     ///< Slave Select is deactivated and indicates Master Mode Fault
} hal_spi_event_t;

typedef enum {
    SPI_FORMAT_CPOL0_CPHA0 = 0,  ///< Clock Polarity 0, Clock Phase 0
    SPI_FORMAT_CPOL0_CPHA1,      ///< Clock Polarity 0, Clock Phase 1
    SPI_FORMAT_CPOL1_CPHA0,      ///< Clock Polarity 1, Clock Phase 0
    SPI_FORMAT_CPOL1_CPHA1,      ///< Clock Polarity 1, Clock Phase 1
} hal_spi_cp_format_t;

typedef enum {
    SPI_FRAME_LEN_4 = 4,
    SPI_FRAME_LEN_5,
    SPI_FRAME_LEN_6,
    SPI_FRAME_LEN_7,
    SPI_FRAME_LEN_8,
    SPI_FRAME_LEN_9,
    SPI_FRAME_LEN_10,
    SPI_FRAME_LEN_11,
    SPI_FRAME_LEN_12,
    SPI_FRAME_LEN_13,
    SPI_FRAME_LEN_14,
    SPI_FRAME_LEN_15,
    SPI_FRAME_LEN_16
} hal_spi_frame_len_t;

typedef struct {
    uint8_t readable;
    uint8_t writeable;
    uint8_t error;
} hal_spi_state_t;

typedef struct _device_spi hal_spi_t;
struct _device_spi {
    struct rt_spi_bus spi_bus;
    char             *device_name;
    uint8_t          *tx_data;  ///< Output data buf
    uint32_t          tx_size;  ///< Output data size specified by user
    uint8_t          *rx_data;  ///< Input  data buf
    uint32_t          rx_size;  ///< Input  data size specified by user
    void (*callback)(hal_spi_t *spi, hal_spi_event_t event,
                     void *arg);  ///< User callback ,signaled by driver event
    int (*send)(hal_spi_t *spi, const void *data, uint32_t size);  ///< The send_async func
    int (*receive)(hal_spi_t *spi, void *data, uint32_t size);     ///< The receive_async func
    int (*send_receive)(hal_spi_t *spi, const void *data_out, void *data_in,
                        uint32_t size);  ///< The send_receive_async func
    void           *arg;                 ///< User private param ,passed to user callback
    hal_spi_state_t state;               ///< Peripheral state
    hal_dma_ch_t   *tx_dma;
    hal_dma_ch_t   *rx_dma;
    void           *priv;
};

void     hal_spi_init(hal_spi_t *spi, uint32_t idx);
void     hal_spi_uninit(hal_spi_t *spi);
int      hal_spi_attach_callback(hal_spi_t *spi, void *callback, void *arg);
void     hal_spi_detach_callback(hal_spi_t *spi);
int      hal_spi_mode(hal_spi_t *spi, hal_spi_mode_t mode);
int      hal_spi_cp_format(hal_spi_t *spi, hal_spi_cp_format_t format);
uint32_t hal_spi_baud(hal_spi_t *spi, uint32_t baud);
int      hal_spi_frame_len(hal_spi_t *spi, hal_spi_frame_len_t length);
void     hal_spi_select_slave(hal_spi_t *spi, uint32_t slave_num);
int32_t  hal_spi_send_receive(hal_spi_t *spi, const void *data_out, void *data_in, uint32_t size,
                              uint32_t timeout);
int32_t  hal_spi_send_receive_async(hal_spi_t *spi, const void *data_out, void *data_in,
                                    uint32_t size);
int32_t  hal_spi_send_receive_dma(hal_spi_t *spi, const void *data_out, void *data_in,
                                  uint32_t size);
int      hal_spi_link_dma(hal_spi_t *spi, hal_dma_ch_t *tx_dma, hal_dma_ch_t *rx_dma);

#endif