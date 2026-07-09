#include "cvi_debug.h"
#include "cvi_sys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>

static void proc_log(int32_t argc, char **argv)
{
    const CVI_CHAR *str;
    CVI_BOOL bAll = CVI_FALSE;
    int32_t lever = 0;

    if (log_levels == NULL)
        return;

    if (argc == 3) {
        lever = atoi(argv[2]);
        if (lever < CVI_DBG_EMERG || lever > CVI_DBG_DEBUG) {
            printf("log lever err, range[%d-%d]\n", CVI_DBG_EMERG, CVI_DBG_DEBUG);
            return;
        }
        if (strncmp(argv[1], "ALL", strlen("ALL")) == 0)
            bAll = CVI_TRUE;
        for (CVI_S32 i = 0; i < CVI_ID_BUTT; ++i) {
            if (bAll) {
                log_levels[i] = lever;
                continue;
            }
            str = CVI_SYS_GetModName(i);
            if (strncmp(argv[1], str, strlen(str)) == 0) {
                log_levels[i] = lever;
                break;
            }
        }
    }

    for (CVI_S32 i = 0; i < CVI_ID_BUTT; ++i) {
        printf("%-10s(%2d)\n", CVI_SYS_GetModName(i), log_levels[i]);
    }
}

MSH_CMD_EXPORT(proc_log, log level info);
