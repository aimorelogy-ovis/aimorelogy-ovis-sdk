
#ifndef _LOG_H_
#define _LOG_H_

#include "rtos_types.h"
#include <stdio.h>

enum
{
    NONE = 0,
    ERR,
    WARN,
    INFO,
    TRACE,
    MAX_LOG_LEVEL
};
enum
{
    LOG_HAS_DAY_NAME   = 1,    /**< Include day name [default: no]         */
    LOG_HAS_YEAR       = 2,    /**< Include year digit [no]                */
    LOG_HAS_MONTH      = 4,    /**< Include month [no]                     */
    LOG_HAS_DAY_OF_MON = 8,    /**< Include day of month [no]              */
    LOG_HAS_TIME       = 16,   /**< Include time [yes]                     */
    LOG_HAS_MICRO_SEC  = 32,   /**< Include microseconds [yes]             */
    LOG_HAS_FILE       = 64,   /**< Include sender in the log [yes]        */
    LOG_HAS_NEWLINE    = 128,  /**< Terminate each call with newline [yes] */
    LOG_HAS_CR         = 256,  /**< Include carriage return [no]           */
    LOG_HAS_SPACE      = 512,  /**< Include two spaces before log [yes]    */
    LOG_HAS_COLOR      = 1024, /**< Colorize logs [yes on win32]           */
    LOG_HAS_LEVEL_TEXT = 2048  /**< Include level text string [no]         */
};
enum
{
    TERM_COLOR_R      = 2, /**< Red            */
    TERM_COLOR_G      = 4, /**< Green          */
    TERM_COLOR_B      = 1, /**< Blue.          */
    TERM_COLOR_BRIGHT = 8  /**< Bright mask.   */
};

#define MAX_PRINT_LENGTH 512

#define JLOG(level, msg, ...) printf("%s(), %d, " msg "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)


#if defined(__cplusplus)
}
#endif

#endif //#ifndef _LOG_H_
