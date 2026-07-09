
#ifndef __DAEMON_UTILS_H__
#define __DAEMON_UTILS_H__

#include "daemon_base.h"
#include "sensor_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

int dump_video_frame(FILE *fp, void *pframe);
// ini_path: first call set valid path, subsequence call can be NULL
SENSOR_CFG_S *get_sensor_cfg(char *ini_path);

#ifdef __cplusplus
}
#endif
#endif
