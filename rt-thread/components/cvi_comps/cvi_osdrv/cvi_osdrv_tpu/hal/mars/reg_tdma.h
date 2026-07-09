/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: reg_tdma.h
 * Description: tdma register define header file
 */

#ifndef __REG_TDMA_H__
#define __REG_TDMA_H__
#ifdef __cplusplus
extern "C" {
#endif

#define TDMA_DESC_REG_BYTES           (0x40)
#define TDMA_ENGINE_DESCRIPTOR_NUM    (TDMA_DESC_REG_BYTES >> 2)
#define TDMA_NUM_BASE_REGS            (0x8)

//backward compatible?
#define GDMA_TYPE_f32           0
#define GDMA_TYPE_f16           1
#define GDMA_TYPE_i32           2
#define GDMA_TYPE_i16           3
#define GDMA_TYPE_i8            4
#define GDMA_TYPE_i4            5
#define GDMA_TYPE_i2            6
#define GDMA_TYPE_i1            7
#define LAST_GDMA_TYPE_i1       8


//tdma control define
#define TDMA_ENGINE_BASE_ADDR   0x0C100000  //base related virual address
#define TDMA_CTRL               (0x0)
#define TDMA_DES_BASE           (0x4)
#define TDMA_INT_MASK           (0x8)
#define TDMA_SYNC_STATUS        (0xC)
#define TDMA_CMD_ACCP0          (0x10)
#define TDMA_CMD_ACCP1          (0x14)
#define TDMA_CMD_ACCP2          (0x18)
#define TDMA_CMD_ACCP3          (0x1C)
#define TDMA_CMD_ACCP4          (0x20)
#define TDMA_CMD_ACCP5          (0x24)
#define TDMA_CMD_ACCP6          (0x28)
#define TDMA_CMD_ACCP7          (0x2C)
#define TDMA_CMD_ACCP8          (0x30)
#define TDMA_CMD_ACCP9          (0x34)
#define TDMA_CMD_ACCP10         (0x38)
#define TDMA_CMD_ACCP11         (0x3C)
#define TDMA_CMD_ACCP12         (0x40)
#define TDMA_CMD_ACCP13         (0x44)
#define TDMA_CMD_ACCP14         (0x48)
#define TDMA_CMD_ACCP15         (0x4C)
#define TDMA_ARRAYBASE0_L       (0x70)
#define TDMA_ARRAYBASE1_L       (0x74)
#define TDMA_ARRAYBASE2_L       (0x78)
#define TDMA_ARRAYBASE3_L       (0x7C)
#define TDMA_ARRAYBASE4_L       (0x80)
#define TDMA_ARRAYBASE5_L       (0x84)
#define TDMA_ARRAYBASE6_L       (0x88)
#define TDMA_ARRAYBASE7_L       (0x8C)
#define TDMA_ARRAYBASE0_H       (0x90)
#define TDMA_ARRAYBASE1_H       (0x94)
#define TDMA_DEBUG_MODE         (0xA0)
#define TDMA_DCM_DISABLE        (0xA4)
#define TDMA_STATUS             (0xEC)


#define TDMA_CTRL_ENABLE_BIT        0
#define TDMA_CTRL_MODESEL_BIT       1
#define TDMA_CTRL_RESET_SYNCID_BIT  2
#define TDMA_CTRL_FORCE_1ARRAY      5
#define TDMA_CTRL_FORCE_2ARRAY      6
#define TDMA_CTRL_BURSTLEN_BIT      8
#define TDMA_CTRL_64BYTE_ALIGN_EN   10
#define TDMA_CTRL_INTRA_CMD_OFF     13
#define TDMA_CTRL_DESNUM_BIT        16



#ifdef __cplusplus
}
#endif
#endif /* REG_TDMA_H */

