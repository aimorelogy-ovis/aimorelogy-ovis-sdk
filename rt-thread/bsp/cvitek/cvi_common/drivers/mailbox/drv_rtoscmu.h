#ifndef __RTOS_COMMAND_QUEUE__
#define __RTOS_COMMAND_QUEUE__
#include <rtthread.h>
#ifdef __linux__
#include <linux/kernel.h>
#endif

#define NR_SYSTEM_CMD 20
#define NR_RTOS_CMD   127
#define NR_RTOS_IP    IP_LIMIT

#define uint8_t  rt_uint8_t
#define uint16_t rt_uint16_t
#define uint32_t rt_uint32_t
#define uint64_t rt_uint64_t

enum IP_TYPE {
    IP_ISP = 0,
    IP_VCODEC,
    IP_VIP,
    IP_VI,
    IP_RGN,
    IP_AUDIO,
    IP_SYSTEM,
    IP_CAMERA,
    IP_LIMIT,
};

enum SYS_CMD_ID {
    SYS_CMD_INFO_TRANS = 0x50,
    SYS_CMD_INFO_LINUX_INIT_DONE,
    SYS_CMD_INFO_RTOS_INIT_DONE,
    SYS_CMD_INFO_STOP_ISR,
    SYS_CMD_INFO_STOP_ISR_DONE,
    SYS_CMD_INFO_LINUX,
    SYS_CMD_INFO_RTOS,
    SYS_CMD_SYNC_TIME,
    SYS_CMD_INFO_DUMP_MSG,
    SYS_CMD_INFO_DUMP_EN,
    SYS_CMD_INFO_DUMP_DIS,
    SYS_CMD_INFO_DUMP_JPG,
    SYS_CMD_INFO_TRACE_SNAPSHOT_START,
    SYS_CMD_INFO_TRACE_SNAPSHOT_STOP,
    SYS_CMD_INFO_TRACE_STREAM_START,
    SYS_CMD_INFO_TRACE_STREAM_STOP,
    SYS_CMD_INFO_LIMIT,
};

struct valid_t {
    unsigned char linux_valid;
    unsigned char rtos_valid;
} __attribute__((packed));

typedef union resv_t {
    struct valid_t valid;
    unsigned short mstime; // 0 : noblock, -1 : block infinite
} resv_t;

typedef struct cmdqu_t cmdqu_t;
/* cmdqu size should be 8 bytes because of mailbox buffer size */
struct cmdqu_t {
    unsigned char ip_id;
    unsigned char cmd_id : 7;
    unsigned char block : 1;
    union resv_t  resv;
    unsigned int  param_ptr;
} __attribute__((packed)) __attribute__((aligned(0x8)));

#ifdef __linux__
/* keep those commands for ioctl system used */
enum SYSTEM_CMD_TYPE {
    CMDQU_SEND = 1,
    CMDQU_REQUEST,
    CMDQU_REQUEST_FREE,
    CMDQU_SEND_WAIT,
    CMDQU_SEND_WAKEUP,
    CMDQU_SYSTEM_LIMIT = NR_SYSTEM_CMD,
};

#define RTOS_CMDQU_DEV_NAME     "cvi-rtos-cmdqu"
#define RTOS_CMDQU_SEND         _IOW('r', CMDQU_SEND, unsigned long)
#define RTOS_CMDQU_REQUEST      _IOW('r', CMDQU_REQUEST, unsigned long)
#define RTOS_CMDQU_REQUEST_FREE _IOW('r', CMDQU_REQUEST_FREE, unsigned long)
#define RTOS_CMDQU_SEND_WAIT    _IOW('r', CMDQU_SEND_WAIT, unsigned long)
#define RTOS_CMDQU_SEND_WAKEUP  _IOW('r', CMDQU_SEND_WAKEUP, unsigned long)

int rtos_cmdqu_send(cmdqu_t *cmdq);
int rtos_cmdqu_send_wait(cmdqu_t *cmdq, int wait_cmd_id);
int request_rtos_irq(unsigned char ip_id, void *handler, const char *devname,
                     void *dev_id);
int free_rtos_irq(unsigned char ip_id);

#endif // end of __linux__








#ifndef __linux__
#include "rtos_types.h"
#else
#include <linux/kernel.h>

enum FAST_IMAGE_CMD_TYPE {
    FAST_SEND_STOP_REC = CMDQU_SYSTEM_LIMIT,
    FAST_SEND_QUERY_ISP_PADDR,
    FAST_SEND_QUERY_ISP_VADDR,
    FAST_SEND_QUERY_ISP_SIZE,
    FAST_SEND_QUERY_ISP_CTXT,
    FAST_SEND_QUERY_IMG_PADDR,
    FAST_SEND_QUERY_IMG_VADDR,
    FAST_SEND_QUERY_IMG_SIZE,
    FAST_SEND_QUERY_IMG_CTXT,
    FAST_SEND_QUERY_ENC_PADDR,
    FAST_SEND_QUERY_ENC_VADDR,
    FAST_SEND_QUERY_ENC_SIZE,
    FAST_SEND_QUERY_ENC_CTXT,
    FAST_SEND_QUERY_FREE_ISP_ION,
    FAST_SEND_QUERY_FREE_IMG_ION,
    FAST_SEND_QUERY_FREE_ENC_ION,
    FAST_SEND_QUERY_DUMP_MSG,
    FAST_SEND_QUERY_DUMP_MSG_INFO,
    FAST_SEND_QUERY_DUMP_EN,
    FAST_SEND_QUERY_DUMP_DIS,
    FAST_SEND_QUERY_DUMP_JPG,
    FAST_SEND_QUERY_DUMP_JPG_INFO,
    FAST_SEND_QUERY_TRACE_SNAPSHOT_START,
    FAST_SEND_QUERY_TRACE_SNAPSHOT_STOP,
    FAST_SEND_QUERY_TRACE_SNAPSHOT_DUMP,
    FAST_SEND_QUERY_TRACE_STREAM_START,
    FAST_SEND_QUERY_TRACE_STREAM_STOP,
    FAST_SEND_QUERY_TRACE_STREAM_DUMP,
    FAST_SEND_LIMIT,
};

#define FAST_IMAGE_DEV_NAME      "cvi-fast-image"
#define FAST_IMAGE_SEND_STOP_REC _IOW('r', FAST_SEND_STOP_REC, unsigned long)
#define FAST_IMAGE_QUERY_ISP_PADDR                                             \
    _IOW('r', FAST_SEND_QUERY_ISP_PADDR, unsigned long)
#define FAST_IMAGE_QUERY_ISP_VADDR                                             \
    _IOW('r', FAST_SEND_QUERY_ISP_VADDR, unsigned long)
#define FAST_IMAGE_QUERY_ISP_SIZE                                              \
    _IOW('r', FAST_SEND_QUERY_ISP_SIZE, unsigned long)
#define FAST_IMAGE_QUERY_ISP_CTXT                                              \
    _IOW('r', FAST_SEND_QUERY_ISP_CTXT, unsigned long)
#define FAST_IMAGE_QUERY_IMG_PADDR                                             \
    _IOW('r', FAST_SEND_QUERY_IMG_PADDR, unsigned long)
#define FAST_IMAGE_QUERY_IMG_VADDR                                             \
    _IOW('r', FAST_SEND_QUERY_IMG_VADDR, unsigned long)
#define FAST_IMAGE_QUERY_IMG_SIZE                                              \
    _IOW('r', FAST_SEND_QUERY_IMG_SIZE, unsigned long)
#define FAST_IMAGE_QUERY_IMG_CTXT                                              \
    _IOW('r', FAST_SEND_QUERY_IMG_CTXT, unsigned long)
#define FAST_IMAGE_QUERY_ENC_PADDR                                             \
    _IOW('r', FAST_SEND_QUERY_ENC_PADDR, unsigned long)
#define FAST_IMAGE_QUERY_ENC_VADDR                                             \
    _IOW('r', FAST_SEND_QUERY_ENC_VADDR, unsigned long)
#define FAST_IMAGE_QUERY_ENC_SIZE                                              \
    _IOW('r', FAST_SEND_QUERY_ENC_SIZE, unsigned long)
#define FAST_IMAGE_QUERY_ENC_CTXT                                              \
    _IOW('r', FAST_SEND_QUERY_ENC_CTXT, unsigned long)
#define FAST_IMAGE_QUERY_FREE_ISP_ION                                          \
    _IOW('r', FAST_SEND_QUERY_FREE_ISP_ION, unsigned long)
#define FAST_IMAGE_QUERY_FREE_IMG_ION                                          \
    _IOW('r', FAST_SEND_QUERY_FREE_IMG_ION, unsigned long)
#define FAST_IMAGE_QUERY_FREE_ENC_ION                                          \
    _IOW('r', FAST_SEND_QUERY_FREE_ENC_ION, unsigned long)
#define FAST_IMAGE_QUERY_DUMP_MSG                                              \
    _IOW('r', FAST_SEND_QUERY_DUMP_MSG, unsigned long)
#define FAST_IMAGE_QUERY_DUMP_MSG_INFO                                         \
    _IOW('r', FAST_SEND_QUERY_DUMP_MSG_INFO, unsigned long)
#define FAST_IMAGE_QUERY_DUMP_EN                                               \
    _IOW('r', FAST_SEND_QUERY_DUMP_EN, unsigned long)
#define FAST_IMAGE_QUERY_DUMP_DIS                                              \
    _IOW('r', FAST_SEND_QUERY_DUMP_DIS, unsigned long)
#define FAST_IMAGE_QUERY_DUMP_JPG                                              \
    _IOW('r', FAST_SEND_QUERY_DUMP_JPG, unsigned long)
#define FAST_IMAGE_QUERY_DUMP_JPG_INFO                                         \
    _IOW('r', FAST_SEND_QUERY_DUMP_JPG_INFO, unsigned long)
#define FAST_IMAGE_QUERY_TRACE_SNAPSHOT_START                                  \
    _IOW('r', FAST_SEND_QUERY_TRACE_SNAPSHOT_START, unsigned long)
#define FAST_IMAGE_QUERY_TRACE_SNAPSHOT_STOP                                   \
    _IOW('r', FAST_SEND_QUERY_TRACE_SNAPSHOT_STOP, unsigned long)
#define FAST_IMAGE_QUERY_TRACE_SNAPSHOT_DUMP                                   \
    _IOW('r', FAST_SEND_QUERY_TRACE_SNAPSHOT_DUMP, unsigned long)
#define FAST_IMAGE_QUERY_TRACE_STREAM_START                                    \
    _IOW('r', FAST_SEND_QUERY_TRACE_STREAM_START, unsigned long)
#define FAST_IMAGE_QUERY_TRACE_STREAM_STOP                                     \
    _IOW('r', FAST_SEND_QUERY_TRACE_STREAM_STOP, unsigned long)
#define FAST_IMAGE_QUERY_TRACE_STREAM_DUMP                                     \
    _IOW('r', FAST_SEND_QUERY_TRACE_STREAM_DUMP, unsigned long)

#endif // end of __linux__

#define C906_MAGIC_HEADER 0xA55AC906 // master cpu is c906
#define CA53_MAGIC_HEADER 0xA55ACA53 // master cpu is ca53

#ifdef __riscv
#define RTOS_MAGIC_HEADER C906_MAGIC_HEADER
#else
#define RTOS_MAGIC_HEADER CA53_MAGIC_HEADER
#endif

enum E_IMAGE_TYPE {
    E_FAST_NONE = 0,
    E_FAST_JEPG = 1,
    E_FAST_H264,
    E_FAST_H265,
};

enum _MUC_STATUS_E {
    MCU_STATUS_NONOS_INIT = 1,
    MCU_STATUS_NONOS_RUNNING,
    MCU_STATUS_NONOS_DONE,
    MCU_STATUS_RTOS_T1_INIT, // before linux running
    MCU_STATUS_RTOS_T1_RUNNING,
    MCU_STATUS_RTOS_T2_INIT, // after linux running
    MCU_STATUS_RTOS_T2_RUNNING,
    MCU_STATUS_LINUX_INIT,
    MCU_STATUS_LINUX_RUNNING,
};

enum DUMP_PRINT_SIZE_E {
    DUMP_PRINT_SZ_IDX_0K = 0,
    DUMP_PRINT_SZ_IDX_4K = 12, // 4096 = 1<<12
    DUMP_PRINT_SZ_IDX_8K,
    DUMP_PRINT_SZ_IDX_16K,
    DUMP_PRINT_SZ_IDX_32K,
    DUMP_PRINT_SZ_IDX_64K,
    DUMP_PRINT_SZ_IDX_128K,
    DUMP_PRINT_SZ_IDX_LIMIT,
};

#define ATTR __attribute__

#ifndef __packed
#define __packed ATTR((packed))
#endif

#ifndef __aligned
#define __aligned(x) ATTR((aligned(x)))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

/* this structure should be modified both fsbl & MCU & osdrv side */
struct transfer_config_t {
    uint32_t conf_magic;
    uint32_t conf_size; //conf_size exclude mcu_status & linux_status
    uint32_t isp_buffer_addr;
    uint32_t isp_buffer_size;
    uint32_t encode_img_addr;
    uint32_t encode_img_size;
    uint32_t encode_buf_addr;
    uint32_t encode_buf_size;
    uint8_t  dump_print_enable;
    uint8_t  dump_print_size_idx;
    uint16_t image_type;
    uint16_t checksum; // checksum exclude mcu_status & linux_status
    uint8_t  mcu_status;
    uint8_t  linux_status;
} __packed __aligned(0x40);

struct trace_snapshot_t {
    uint32_t ptr;
    uint16_t size;
    uint16_t type;
} __packed;

#endif // end of __RTOS_COMMAND_QUEUE__
