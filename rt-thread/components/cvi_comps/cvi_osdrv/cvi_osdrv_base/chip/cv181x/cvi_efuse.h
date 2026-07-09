#ifndef __CVI_EFUSE_H__
#define __CVI_EFUSE_H__

#include "cvi_type.h"
#include "rtos_types.h"
#include <stdint.h>

CVI_S64 cvi_efuse_read_from_shadow(CVI_U32 addr);
int cvi_efuse_write(CVI_U32 addr, CVI_U32 value);

int cvi_efuse_read_buf(u32 addr, void *buf, size_t buf_size);

#endif /* __CVI_EFUSE_H__ */
