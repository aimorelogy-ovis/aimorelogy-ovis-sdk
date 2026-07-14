/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_uvc_gadget.h
 * Description:
 *   UVC gadget
 */

#ifndef __CVI_UVC_GADGET_H__
#define __CVI_UVC_GADGET_H__

#include <sys/types.h>

#include "uvc.h"
#include "cvi_uvc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t UVC_GADGET_Init(const CVI_UVC_DEVICE_CAP_S *pstDevCaps, u_int32_t u32MaxFrameSize);

int32_t UVC_GADGET_DeviceOpen(const char *pDevPath);

int32_t UVC_GADGET_DeviceClose(void);

int32_t UVC_GADGET_DeviceCheck(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_UVC_GADGET_H__ */