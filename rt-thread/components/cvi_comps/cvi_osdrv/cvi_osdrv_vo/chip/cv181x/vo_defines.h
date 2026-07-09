/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __VO_DEFINES_H__
#define __VO_DEFINES_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "pthread.h"
#include "semaphore.h"
// #include "aos/kernel.h"
#include <vip_atomic.h>
#include <vip_spinlock.h>
#include <vip_list.h>
// #include <queue.h>
#include <vo_disp.h>
#include <vo_uapi.h>

enum E_VO_TH {
	E_VO_TH_DISP,
	E_VO_TH_DISP_ISR_TEST,
	E_VO_TH_MAX
};

struct vo_thread_attr {
	char				th_name[32];
	pthread_t			w_thread;
	atomic_t			thread_exit;
	sem_t				sem;
	u32					flag;
	void *(*th_handler)(void *arg);
};

struct cvi_vo_dev {
	// private data
	// struct device			*dev;
	// struct class			*vo_class;
	// dev_t				cdev_id;
	void						*reg_base[4];
	int							irq_num;

	struct vo_dv_timings		dv_timings;
	struct vo_rect				sink_rect;
	struct vo_rect				compose_out;
	struct vo_rect				crop_rect;
	enum cvi_disp_intf			disp_interface;
	bool						bgcolor_enable;
	void						*shared_mem;
	u8							align;
	bool						disp_online;
	u32							vid_caps;
	u32							frame_number;
	u8							num_rdy;
	spinlock_t					rdy_lock;
	struct list_head			rdy_queue;
	u32							seq_count;
	u8							chn_id;
	u32							bytesperline[3];
	u32							sizeimage[3];
	u32							colorspace;
	spinlock_t					qbuf_lock;
	struct list_head			qbuf_list[1];
	u8							qbuf_num[1];
	atomic_t					disp_streamon;
	struct vo_thread_attr		vo_th[E_VO_TH_MAX];
	u8							numOfPlanes;
};


#ifdef __cplusplus
}
#endif

#endif /* __VO_DEFINES_H__ */
