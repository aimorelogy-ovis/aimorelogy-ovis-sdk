
#ifndef __DAEMON_PIPE_H__
#define __DAEMON_PIPE_H__

#include "daemon_base.h"
#include "daemon_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

int create_pipe(daemon_pipe_cfg_t *p_cfg);
int destroy_pipe(daemon_pipe_cfg_t *p_cfg);

#ifdef __cplusplus
}
#endif
#endif
