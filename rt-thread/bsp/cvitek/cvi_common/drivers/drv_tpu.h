#ifndef __CVI_TPU_INTERFACE_H__
#define __CVI_TPU_INTERFACE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "rtos_types.h"

void  cvi_tpu_init(void);
void  cvi_tpu_deinit(void);
void *cvi_tpu_open(void);
int   cvi_tpu_close(void);
int   cvi_tpu_ioctl(void *dev, unsigned int cmd, unsigned long arg);

#define TDMA_DESC_REG_BYTES        (0x40)
#define TDMA_ENGINE_DESCRIPTOR_NUM (TDMA_DESC_REG_BYTES >> 2)
#define TDMA_NUM_BASE_REGS         (0x8)

// backward compatible
#define GDMA_TYPE_f32     0
#define GDMA_TYPE_f16     1
#define GDMA_TYPE_i32     2
#define GDMA_TYPE_i16     3
#define GDMA_TYPE_i8      4
#define GDMA_TYPE_i4      5
#define GDMA_TYPE_i2      6
#define GDMA_TYPE_i1      7
#define LAST_GDMA_TYPE_i1 8

// tdma control define
#define TDMA_ENGINE_BASE_ADDR 0x0C100000  // base related virual address
#define TDMA_CTRL             (0x0)
#define TDMA_DES_BASE         (0x4)
#define TDMA_INT_MASK         (0x8)
#define TDMA_SYNC_STATUS      (0xC)
#define TDMA_CMD_ACCP0        (0x10)
#define TDMA_CMD_ACCP1        (0x14)
#define TDMA_CMD_ACCP2        (0x18)
#define TDMA_CMD_ACCP3        (0x1C)
#define TDMA_CMD_ACCP4        (0x20)
#define TDMA_CMD_ACCP5        (0x24)
#define TDMA_CMD_ACCP6        (0x28)
#define TDMA_CMD_ACCP7        (0x2C)
#define TDMA_CMD_ACCP8        (0x30)
#define TDMA_CMD_ACCP9        (0x34)
#define TDMA_CMD_ACCP10       (0x38)
#define TDMA_CMD_ACCP11       (0x3C)
#define TDMA_CMD_ACCP12       (0x40)
#define TDMA_CMD_ACCP13       (0x44)
#define TDMA_CMD_ACCP14       (0x48)
#define TDMA_CMD_ACCP15       (0x4C)
#define TDMA_ARRAYBASE0_L     (0x70)
#define TDMA_ARRAYBASE1_L     (0x74)
#define TDMA_ARRAYBASE2_L     (0x78)
#define TDMA_ARRAYBASE3_L     (0x7C)
#define TDMA_ARRAYBASE4_L     (0x80)
#define TDMA_ARRAYBASE5_L     (0x84)
#define TDMA_ARRAYBASE6_L     (0x88)
#define TDMA_ARRAYBASE7_L     (0x8C)
#define TDMA_ARRAYBASE0_H     (0x90)
#define TDMA_ARRAYBASE1_H     (0x94)
#define TDMA_DEBUG_MODE       (0xA0)
#define TDMA_DCM_DISABLE      (0xA4)
#define TDMA_STATUS           (0xEC)

#define TDMA_CTRL_ENABLE_BIT       0
#define TDMA_CTRL_MODESEL_BIT      1
#define TDMA_CTRL_RESET_SYNCID_BIT 2
#define TDMA_CTRL_FORCE_1ARRAY     5
#define TDMA_CTRL_FORCE_2ARRAY     6
#define TDMA_CTRL_BURSTLEN_BIT     8
#define TDMA_CTRL_64BYTE_ALIGN_EN  10
#define TDMA_CTRL_INTRA_CMD_OFF    13
#define TDMA_CTRL_DESNUM_BIT       16

#define BDC_ENGINE_CMD_ALIGNED_BIT 8

// base related virtual address
#define TIU_ENGINE_BASE_ADDR 0x0C101000
#define BD_CMD_BASE_ADDR     (0)
#define BD_CTRL_BASE_ADDR    (0x100)

// BD control bits base on BD_CTRL_BASE_ADDR
#define BD_TPU_EN       0   // TPU Enable bit
#define BD_LANE_NUM     22  // Lane number bit[29:22]
#define BD_DES_ADDR_VLD 30  // enable descriptor mode
#define BD_INTR_ENABLE  31  // TIU interrupt global enable

enum TIU_LANNUM {
    TIU_LANNUM_2  = 0x1,
    TIU_LANNUM_4  = 0x2,
    TIU_LANNUM_8  = 0x3,
    TIU_LANNUM_16 = 0x4,
    TIU_LANNUM_32 = 0x5,
    TIU_LANNUM_64 = 0x6,
};

struct cvi_cache_op_arg {
    uintptr_t          paddr;
    unsigned long long size;
    int                dma_fd;
};

struct cvi_submit_dma_arg {
    uintptr_t   *addr;
    unsigned int seq_no;
};

struct cvi_wait_dma_arg {
    unsigned int seq_no;
    int          ret;
};

struct cvi_pio_mode {
    unsigned long long cmdbuf;
    unsigned long long sz;
};

struct cvi_load_tee_arg {
    // normal domain
    unsigned long long cmdbuf_addr_ree;
    unsigned int       cmdbuf_len_ree;
    unsigned long long weight_addr_ree;
    unsigned int       weight_len_ree;
    unsigned long long neuron_addr_ree;

    // security domain
    unsigned long long dmabuf_addr_tee;
};

struct cvi_submit_tee_arg {
    unsigned long long dmabuf_tee_addr;
    unsigned long long gaddr_base2;
    unsigned long long gaddr_base3;
    unsigned long long gaddr_base4;
    unsigned long long gaddr_base5;
    unsigned long long gaddr_base6;
    unsigned long long gaddr_base7;
    unsigned int       seq_no;
};

struct cvi_unload_tee_arg {
    uintptr_t          addr;
    unsigned long long size;
};

struct cvi_tdma_copy_arg {
    uintptr_t    paddr_src;
    uintptr_t    paddr_dst;
    unsigned int h;
    unsigned int w_bytes;
    unsigned int stride_bytes_src;
    unsigned int stride_bytes_dst;
    unsigned int enable_2d;
    unsigned int leng_bytes;
    unsigned int seq_no;
};

struct cvi_tdma_wait_arg {
    unsigned int seq_no;
    int          ret;
};
/***************************************
 * cvi_tpu_ioctl ↑
 **************************************/

#define CVITPU_SUBMIT_DMABUF   0x01
#define CVITPU_DMABUF_FLUSH_FD 0x02
#define CVITPU_DMABUF_INVLD_FD 0x03
#define CVITPU_DMABUF_FLUSH    0x04
#define CVITPU_DMABUF_INVLD    0x05
#define CVITPU_WAIT_DMABUF     0x06
#define CVITPU_PIO_MODE        0x07

#define CVITPU_LOAD_TEE   0x08
#define CVITPU_SUBMIT_TEE 0x09
#define CVITPU_UNLOAD_TEE 0x0A
#define CVITPU_SUBMIT_PIO 0x0B
#define CVITPU_WAIT_PIO   0x0C

#ifdef __cplusplus
}
#endif
#endif  // __CVI_TPU_INTERFACE_H__