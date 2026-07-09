#ifndef __VI_COMMON_H__
#define __VI_COMMON_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "rtos_types.h"
// #include <aos/types.h>
#include "cvi_reg.h"
#include <vip_atomic.h>
#include <vip_spinlock.h>
#include <errno.h>
#include "cvi_debug.h"
#include "vi_debug.h"

extern int vi_dump_reg;

#define ISP_BASE_ADDR 0x0A000000
#define ISP_TOP_INT 24

#if 0
#define _reg_read(addr) readl((void __iomem *)addr)
//#define _reg_write(addr, data) writel(data, (void __iomem *)addr)
#define _reg_write(addr, data) \
	{ \
		writel(data, (void __iomem *)addr); \
		if (vi_dump_reg) \
			pr_info("MWriteS32 %#x %#x\n", (u32)(addr), (u32)(data)); \
	}
#endif

#define MIN(a, b) (((a) < (b))?(a):(b))
#define MAX(a, b) (((a) > (b))?(a):(b))
#define VI_64_ALIGN(x) (((x) + 0x3F) & ~0x3F)   // for 64byte alignment
#define VI_256_ALIGN(x) (((x) + 0xFF) & ~0xFF)   // for 256byte alignment
#define VI_ALIGN(x) (((x) + 0xF) & ~0xF)   // for 16byte alignment
#define VI_256_ALIGN(x) (((x) + 0xFF) & ~0xFF)   // for 256byte alignment
#define ISP_ALIGN(x, y) (((x) + (y - 1)) & ~(y - 1))   // for any bytes alignment
#define UPPER(x, y) (((x) + ((1 << (y)) - 1)) >> (y))   // for alignment
#define CEIL(x, y) (((x) + ((1 << (y)))) >> (y))   // for alignment

extern CVI_S32 log_levels[CVI_ID_BUTT];

#define vi_pr(level, fmt, ...)             \
	do {                                                   \
		CVI_S32 LogLevel = (log_levels == NULL) ? CONFIG_CVI_LOG_TRACE_LEVEL : log_levels[CVI_ID_VI];      \
		if (level <= LogLevel) {							\
			rt_kprintf("%s:%d(): " fmt, __func__, __LINE__, ##__VA_ARGS__);		\
		}										\
	} while (0)


enum vi_msg_pri {
	VI_ERR		= 0,
	VI_WARN		= 1,
	VI_NOTICE	= 2,
	VI_INFO		= 4,
	VI_DBG		= 7,
};

struct vi_rect {
	u16 x;
	u16 y;
	u16 w;
	u16 h;
};


//void _reg_write_mask(uintptr_t addr, u32 mask, u32 data);
int vip_sys_cif_cb(unsigned int cmd, void *arg);
int vip_sys_cmm_cb_i2c(unsigned int cmd, void *arg);
void vip_sys_reg_write_mask(uintptr_t addr, u32 mask, u32 data);

#if 0
extern bool __clk_is_enabled(struct clk *clk);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VI_COMMON_H__ */
