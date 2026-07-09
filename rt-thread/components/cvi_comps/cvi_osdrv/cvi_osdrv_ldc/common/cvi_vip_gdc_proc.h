/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_vip_gdc_proc.h
 * Description:
 */

#ifndef _CVI_VIP_GDC_PROC_H_
#define _CVI_VIP_GDC_PROC_H_

#ifdef __cplusplus
extern "C" {
#endif
int get_gdc_proc(char *outbuf);
int gdc_proc_init(void *shm);
int gdc_proc_remove(void);

#ifdef __cplusplus
}
#endif

#endif /* _CVI_VIP_GDC_PROC_H_ */

