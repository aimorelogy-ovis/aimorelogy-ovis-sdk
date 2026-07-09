#ifndef __HAL_DMA_H__
#define __HAL_DMA_H__
// HAL define common dma structs, and common dma API
// The DMA API implementation is placed in the .c file of each IP
#include <stdint.h>
#include "hal_list.h"

#define dma_log printf
#define dma_err printf
//#define DMA_DEBUG
#ifdef DMA_DEBUG
#define dma_dbg rt_kprintf
#else
#define dma_dbg(...)
#endif

#define DMA_LLI_SIZE 28

typedef enum {
    CVI_I2S0_RX = 0,
    CVI_I2S0_TX,
    CVI_I2S1_RX,
    CVI_I2S1_TX,
    CVI_I2S2_RX,
    CVI_I2S2_TX,
    CVI_I2S3_RX,
    CVI_I2S3_TX,
    CVI_UART0_RX,
    CVI_UART0_TX,
    CVI_UART1_RX,
    CVI_UART1_TX,
    CVI_UART2_RX,
    CVI_UART2_TX,
    CVI_UART3_RX,
    CVI_UART3_TX,
    CVI_SPI0_RX,
    CVI_SPI0_TX,
    CVI_SPI1_RX,
    CVI_SPI1_TX,
    CVI_SPI2_RX,
    CVI_SPI2_TX,
    CVI_SPI3_RX,
    CVI_SPI3_TX,
    CVI_I2C0_RX,
    CVI_I2C0_TX,
    CVI_I2C1_RX,
    CVI_I2C1_TX,
    CVI_I2C2_RX,
    CVI_I2C2_TX,
    CVI_I2C3_RX,
    CVI_I2C3_TX,
    CVI_I2C4_RX,
    CVI_I2C4_TX,
    CVI_TDM0_RX,
    CVI_TDM0_TX,
    CVI_TDM1_RX,
    CVI_AUDSRC,
    CVI_SPI_NAND,
    CVI_SPI_NOR,
    CVI_UART4_RX,
    CVI_UART4_TX,
    CVI_SPI_NOR1,
} request_index_t;

struct dma_remap_item {
    request_index_t hs_id;
    uint8_t         channel_id;
};

typedef uint64_t dma_addr_t;

typedef enum { DMA_ADDR_INC = 0, DMA_ADDR_DEC, DMA_ADDR_CONSTANT } dma_addr_inc_t;

typedef enum {
    DMA_DATA_WIDTH_8_BITS = 0,
    DMA_DATA_WIDTH_16_BITS,
    DMA_DATA_WIDTH_32_BITS,
    DMA_DATA_WIDTH_64_BITS,
    DMA_DATA_WIDTH_128_BITS,
    DMA_DATA_WIDTH_512_BITS
} dma_data_width_t;

typedef enum {
    DMA_MEM2MEM = 0,
    DMA_MEM2DEV,
    DMA_DEV2MEM,
} dma_trans_dir_t;

typedef enum {
    DMA_EVENT_TRANSFER_DONE = 0,   ///< transfer complete
    DMA_EVENT_TRANSFER_HALF_DONE,  ///< transfer half done
    DMA_EVENT_TRANSFER_ERROR,      ///< transfer error
} dma_event_t;

typedef struct hal_dma_ch hal_dma_ch_t;

struct hal_dma_ch {
    void   *parents;
    uint8_t ctrl_id;
    uint8_t ch_id;
    void (*callback)(hal_dma_ch_t *dma_ch, dma_event_t event, void *arg);
    void    *arg;
    uint32_t lli_num;                 // lli buffer len
    uint32_t lli_count;               // lli data count
    int32_t  lli_w_p;                 // write position
    int32_t  lli_r_p;                 // read position
    void    *lli;                     // lli buffer
    uint32_t lli_loop_buf0;           // lli loop data
    uint32_t lli_loop_buf1;           // lli loop data
    uint8_t  lli_loop[DMA_LLI_SIZE];  // lli loop handle
    int16_t  etb_ch_id;
    slist_t  next;
};

typedef struct {
    dma_addr_inc_t   src_inc;        ///< source address increment
    dma_addr_inc_t   dst_inc;        ///< destination address increment
    dma_data_width_t src_tw;         ///< source transfer width in byte
    dma_data_width_t dst_tw;         ///< destination transfer width in byte
    dma_trans_dir_t  trans_dir;      ///< transfer direction
    uint16_t         handshake;      ///< handshake id
    uint16_t         group_len;      ///< group transaction length (unit: bytes)
    uint8_t          src_reload_en;  ///< 1:dma enable src addr auto reload, 0:disable
    uint8_t          dst_reload_en;  ///< 1:dma enable dst addr auto reload, 0:disable
    uint8_t          half_int_en;    ///< 1:dma enable half interrupt, 0: disable
    uint8_t          lli_src_en;     ///< 1:dma enable llp, 0 disable
    uint8_t          lli_dst_en;     ///< 1:dma enable llp, 0 disable
} hal_dma_ch_config_t;

typedef struct {
    char         *name;
    slist_t       head;
    uint32_t      alloc_status;
    uint32_t      ch_num;
    unsigned long reg_base;
    uint32_t      irq_num;
    void         *priv;
} hal_dma_t;

int  hal_dma_init(hal_dma_t *dma, uint32_t ctrl_id);
void hal_dma_deinit(hal_dma_t *dma, uint32_t ctrl_id);
int  hal_dma_channel_request(hal_dma_ch_t *hal_dma_ch, uint32_t ch_id, uint32_t ctrl_id);
void hal_dma_ch_stop(hal_dma_ch_t *hal_dma_ch);
void hal_dma_ch_free(hal_dma_ch_t *hal_dma_ch);
void hal_dma_ch_pause(hal_dma_ch_t *hal_dma_ch);
void hal_dma_ch_resume(hal_dma_ch_t *hal_dma_ch);
int  hal_dma_ch_attach_callback(hal_dma_ch_t *hal_dma_ch, void *callback, void *arg);
void hal_dma_ch_detach_callback(hal_dma_ch_t *hal_dma_ch);
int  hal_dma_ch_config(hal_dma_ch_t *hal_dma_ch, hal_dma_ch_config_t *config);
void hal_dma_ch_start(hal_dma_ch_t *hal_dma_ch, void *srcaddr, void *dstaddr, uint32_t length);
#endif