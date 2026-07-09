#ifndef __RGN_DEFINES_H__
#define __RGN_DEFINES_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "cvi_type.h"
#include "rtos_types.h"
#include <cvi_defines.h>
#include <vip_spinlock.h>
#include <pthread.h>

enum sclr_gop {
	SCL_GOP_SCD,
	SCL_GOP_SCV1,
	SCL_GOP_SCV2,
	SCL_GOP_SCV3,
	SCL_GOP_DISP,
	SCL_GOP_MAX,
};

struct sclr_size {
	u16 w;
	u16 h;
};

enum cvi_rgn_format {
	CVI_RGN_FMT_ARGB8888,
	CVI_RGN_FMT_ARGB4444,
	CVI_RGN_FMT_ARGB1555,
	CVI_RGN_FMT_256LUT,
	CVI_RGN_FMT_FONT,
	CVI_RGN_FMT_MAX
};

struct cvi_rect {
	__u32 left;
	__u32 top;
	__u32 width;
	__u32 height;
};

struct cvi_rgn_param {
	enum cvi_rgn_format fmt;
	struct cvi_rect rect;
	__u32 stride;
	__u64 phy_addr;
};

struct cvi_rgn_ex_cfg {
	struct cvi_rgn_param rgn_ex_param[RGN_EX_MAX_NUM_VPSS];
	__u8 num_of_rgn_ex;
	bool hscale_x2;
	bool vscale_x2;
	bool colorkey_en;
	__u32 colorkey;
};

struct cvi_vpss_rgnex_cfg {
	struct cvi_rgn_ex_cfg cfg;
	__u32 pixelformat;
	__u32 bytesperline[2];
	__u64 addr[3];
};

struct cvi_rgn_odec {
	__u8 enable;
	__u8 attached_ow;
	__u32 bso_sz;
};

struct cvi_rgn_cfg {
	struct cvi_rgn_param param[RGN_MAX_NUM_VPSS];
	struct cvi_rgn_odec odec;
	__u8 num_of_rgn;
	bool hscale_x2;
	bool vscale_x2;
	bool colorkey_en;
	__u32 colorkey;
};

struct cvi_rgn_lut_cfg {
	__u16 lut_length;
	__u16 *lut_addr;
	__u8 lut_layer;
	bool rgnex_en;
};

/**
 * struct cvi_rgn - RGN IP abstraction
 */
struct cvi_rgn_dev {
	spinlock_t		lock;
	spinlock_t		rdy_lock;
	pthread_mutex_t		mutex;
	bool			bind_fb;
};

#ifdef __cplusplus
}
#endif

#endif /* __RGN_DEFINES_H__ */
