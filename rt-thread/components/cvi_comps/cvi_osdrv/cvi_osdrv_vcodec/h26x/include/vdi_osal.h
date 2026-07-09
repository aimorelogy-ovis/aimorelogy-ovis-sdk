//------------------------------------------------------------------------------
// File: log.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef _VDI_OSAL_H_
#define _VDI_OSAL_H_
#include "rtos_types.h"
//#include "printf.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
// #include <syslog.h>
#include <string.h>

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define VLOG(level, msg, ...) printf(msg, ##__VA_ARGS__)

#ifndef G_TEST
#define CVI_VC_MSG_ENABLE
#endif

#define USE_SWAP_264_FW

#define CVI_MASK_ERR    0x1
#define CVI_MASK_WARN   0x2
#define CVI_MASK_INFO   0x4
#define CVI_MASK_FLOW   0x8
#define CVI_MASK_DBG    0x10
#define CVI_MASK_INTR   0x20
#define CVI_MASK_MCU    0x40
#define CVI_MASK_MEM    0x80
#define CVI_MASK_BS     0x100
#define CVI_MASK_SRC    0x200
#define CVI_MASK_IF     0x400
#define CVI_MASK_LOCK   0x800
#define CVI_MASK_PERF   0x1000
#define CVI_MASK_CFG    0x2000
#define CVI_MASK_RC     0x4000
#define CVI_MASK_TRACE  0x8000
#define CVI_MASK_DISP   0x10000
#define CVI_MASK_MOTMAP 0x20000
#define CVI_MASK_UBR    0x40000
#define CVI_MASK_RQ     0x80000
#define CVI_MASK_CVRC   0x100000
#define CVI_MASK_AR     0x200000
#define CVI_MASK_REG    0x80000000


#define CVI_MASK_CURR ((0x0) | CVI_MASK_ERR)

#ifdef CVI_VC_MSG_ENABLE
extern int dbg_mask;
#define CVI_PRNT(msg, ...) \
    printf(msg, ##__VA_ARGS__)
#define CVI_FUNC_COND(FLAG, FUNC) \
    do {                          \
        if (dbg_mask & (FLAG))    \
        {                         \
            FUNC;                 \
        }                         \
    } while (0)
#define CVI_VC_ERR(msg, ...)                                                  \
    do {                                                                      \
        if (dbg_mask & CVI_MASK_ERR)                                          \
        {                                                                     \
            printf("[ERR] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#define CVI_VC_WARN(msg, ...)                                                  \
    do {                                                                       \
        if (dbg_mask & CVI_MASK_WARN)                                          \
        {                                                                      \
            printf("[WARN] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#define CVI_VC_INFO(msg, ...)                                                  \
    do {                                                                       \
        if (dbg_mask & CVI_MASK_INFO)                                          \
        {                                                                      \
            printf("[INFO] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#define CVI_VC_FLOW(msg, ...)                                                  \
    do {                                                                       \
        if (dbg_mask & CVI_MASK_FLOW)                                          \
        {                                                                      \
            printf("[FLOW] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#define CVI_VC_DBG(msg, ...)                                                  \
    do {                                                                      \
        if (dbg_mask & CVI_MASK_DBG)                                          \
        {                                                                     \
            printf("[DBG] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#define CVI_VC_INTR(msg, ...)                                                  \
    do {                                                                       \
        if (dbg_mask & CVI_MASK_INTR)                                          \
        {                                                                      \
            printf("[INTR] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#define CVI_VC_MCU(msg, ...)                                                  \
    do {                                                                      \
        if (dbg_mask & CVI_MASK_MCU)                                          \
        {                                                                     \
            printf("[MCU] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#define CVI_VC_MEM(msg, ...)                                                  \
    do {                                                                      \
        if (dbg_mask & CVI_MASK_MEM)                                          \
        {                                                                     \
            printf("[MEM] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#define CVI_VC_BS(msg, ...)                                                  \
    do {                                                                     \
        if (dbg_mask & CVI_MASK_BS)                                          \
        {                                                                    \
            printf("[BS] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                    \
    } while (0)
#define CVI_VC_SRC(msg, ...)                                                  \
    do {                                                                      \
        if (dbg_mask & CVI_MASK_SRC)                                          \
        {                                                                     \
            printf("[SRC] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#define CVI_VC_IF(msg, ...)                                                  \
    do {                                                                     \
        if (dbg_mask & CVI_MASK_IF)                                          \
        {                                                                    \
            printf("[IF] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                    \
    } while (0)
#define CVI_VC_LOCK(msg, ...)                                                  \
    do {                                                                       \
        if (dbg_mask & CVI_MASK_LOCK)                                          \
        {                                                                      \
            printf("[LOCK] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#define CVI_VC_PERF(msg, ...)                                                  \
    do {                                                                       \
        if (dbg_mask & CVI_MASK_PERF)                                          \
        {                                                                      \
            printf("[PERF] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#define CVI_VC_CFG(msg, ...)                                                  \
    do {                                                                      \
        if (dbg_mask & CVI_MASK_CFG)                                          \
        {                                                                     \
            printf("[CFG] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#define CVI_VC_RC(msg, ...)                                                  \
    do {                                                                     \
        if (dbg_mask & CVI_MASK_RC)                                          \
        {                                                                    \
            printf("[RC] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                    \
    } while (0)
#define CVI_VC_TRACE(msg, ...)                                                  \
    do {                                                                        \
        if (dbg_mask & CVI_MASK_TRACE)                                          \
        {                                                                       \
            printf("[TRACE] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                       \
    } while (0)
#define CVI_VC_DISP(msg, ...)                                                  \
    do {                                                                       \
        if (dbg_mask & CVI_MASK_DISP)                                          \
        {                                                                      \
            printf("[DISP] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)
#define CVI_VC_MOTMAP(msg, ...)         \
    do {                                \
        if (dbg_mask & CVI_MASK_MOTMAP) \
        {                               \
            printf(msg, ##__VA_ARGS__); \
        }                               \
    } while (0)
#define CVI_VC_UBR(msg, ...)                                                  \
    do {                                                                      \
        if (dbg_mask & CVI_MASK_UBR)                                          \
        {                                                                     \
            printf("[UBR] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)
#define CVI_VC_RQ(msg, ...)                                                  \
    do {                                                                     \
        if (dbg_mask & CVI_MASK_RQ)                                          \
        {                                                                    \
            printf("[RQ] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                    \
    } while (0)
#define CVI_VC_CVRC(msg, ...)                                  \
    do {                                                       \
        if (dbg_mask & CVI_MASK_CVRC)                          \
        {                                                      \
            printf("[CVRC] %s = %d, " msg, __func__, __LINE__, \
                   ##__VA_ARGS__);                             \
        }                                                      \
    } while (0)
#define CVI_VC_AR(msg, ...)                                  \
    do {                                                     \
        if (dbg_mask & CVI_MASK_AR)                          \
        {                                                    \
            printf("[AR] %s = %d, " msg, __func__, __LINE__, \
                   ##__VA_ARGS__);                           \
        }                                                    \
    } while (0)
#define CVI_VC_REG(msg, ...)                                                  \
    do {                                                                      \
        if (dbg_mask & CVI_MASK_REG)                                          \
        {                                                                     \
            printf("[REG] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)

#else
#define CVI_FUNC_COND(FLAG, FUNC)
#define CVI_VC_ERR(msg, ...)                                                  \
    do {                                                                      \
        if (dbg_mask & CVI_MASK_ERR)                                          \
        {                                                                     \
            printf("[ERR] %s = %d, " msg, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                     \
    } while (0)

#define CVI_VC_WARN(msg, ...)
#define CVI_VC_INFO(msg, ...)
#define CVI_VC_FLOW(msg, ...)
#define CVI_VC_DBG(msg, ...)
#define CVI_VC_INTR(msg, ...)
#define CVI_VC_MCU(msg, ...)
#define CVI_VC_MEM(msg, ...)
#define CVI_VC_BS(msg, ...)
#define CVI_VC_SRC(msg, ...)
#define CVI_VC_IF(msg, ...)
#define CVI_VC_LOCK(msg, ...)
#define CVI_VC_PERF(msg, ...)
#define CVI_VC_CFG(msg, ...)
#define CVI_VC_RC(msg, ...)
#define CVI_VC_TRACE(msg, ...)
#define CVI_VC_DISP(msg, ...)
#define CVI_VC_MOTMAP(msg, ...)
#define CVI_VC_UBR(msg, ...)
#define CVI_VC_RQ(msg, ...)
#define CVI_VC_CVRC(msg, ...)
#define CVI_VC_REG(msg, ...)
#endif

#define VPU_DRAM_SIZE          (0x180000)
#define VPU_DRAM_PHYSICAL_BASE (0x80080000)

#define VDI_SRAM_BASE_ADDR 0x00
#define VDI_SRAM_SIZE      0x25000

//#define SUPPORT_INTERRUPT
#define VPU_TO_32BIT(x) ((x) & 0xFFFFFFFF)
#define VPU_TO_64BIT(x) ((x) | 0x100000000)
#define ENABLE_RC_LIB   1
#define CFG_MEM         0
#define DIRECT_YUV      0
#define SRC_YUV_CYCLIC  0
#define ES_CYCLIC       0
#define FFMPEG_EN       0
#define MAX_TRANSRATE   51000000

#if CFG_MEM
typedef struct _DRAM_CFG_
{
    unsigned long pucCodeAddr;
    int           iCodeSize;
    unsigned long pucVpuDramAddr;
    int           iVpuDramSize;
    unsigned long pucSrcYuvAddr;
    int           iSrcYuvSize;
} DRAM_CFG;

extern DRAM_CFG dramCfg;
#endif

#define TO_VIR(addr, vb) ((addr) - ((vb)->phys_addr) + ((vb)->virt_addr))

enum
{
    NONE = 0,
    INFO,
    WARN,
    ERR,
    TRACE,
    MAX_LOG_LEVEL
};

#ifndef stdout
#define stdout (void *)1
#endif
#ifndef stderr
#define stderr (void *)1
#endif

#if defined(__cplusplus)
extern "C" {
#endif

// math
int math_div(int number, int denom);
int math_modulo(int number, int denom);

#if defined(__cplusplus)
}
#endif

#endif //#ifndef _VDI_OSAL_H_
