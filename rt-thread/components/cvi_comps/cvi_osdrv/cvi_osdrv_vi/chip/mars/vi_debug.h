#ifndef __VI_DEBUG_H__
#define __VI_DEBUG_H__

#include "rtos_types.h"

struct printk_info {
	u64	seq;		/* sequence number */
	u64	ts_nsec;	/* timestamp in nanoseconds */
	u16	text_len;	/* length of text message */
	u8	facility;	/* syslog facility */
	u8	flags:5;	/* internal record flags */
	u8	level:3;	/* syslog level */
	u32	caller_id;	/* thread id or processor id */
};

struct printk_record {
	struct printk_info	*info;
	char			*text_buf;
	unsigned int		text_buf_size;
};

int vi_printk(const char *fmt, ...);
void dmesg_init(void);

#endif
