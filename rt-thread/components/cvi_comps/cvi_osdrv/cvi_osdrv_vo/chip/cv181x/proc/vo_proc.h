/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _VO_PROC_H_
#define _VO_PROC_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include <vo_defines.h>

int vo_proc_init(void *shm);
int vo_proc_remove(void);

#ifdef __cplusplus
}
#endif

#endif // _VO_PROC_H_
