#ifndef __RGN_COMMON_H__
#define __RGN_COMMON_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "cvi_type.h"
#include "rtos_types.h"
#include "cvi_debug.h"

#define RGN_64_ALIGN(x) (((x) + 0x3F) & ~0x3F)   // for 64byte alignment
#define RGN_256_ALIGN(x) (((x) + 0xFF) & ~0xFF)   // for 256byte alignment
#define RGN_ALIGN(x) (((x) + 0xF) & ~0xF)   // for 16byte alignment
#define RGN_256_ALIGN(x) (((x) + 0xFF) & ~0xFF)   // for 256byte alignment
#ifndef ALIGN
#define ALIGN(x, y) (((x) + (y - 1)) & ~(y - 1))   // for any bytes alignment
#endif
#define UPPER(x, y) (((x) + ((1 << (y)) - 1)) >> (y))   // for alignment
#define CEIL(x, y) (((x) + ((1 << (y)))) >> (y))   // for alignment

#define RGN_ERR        3   /* error conditions                     */
#define RGN_WARN       4   /* warning conditions                   */
#define RGN_NOTICE     5   /* normal but significant condition     */
#define RGN_INFO       6   /* informational                        */
#define RGN_DEBUG      7   /* debug-level messages                 */


#define RGB888_2_ARGB1555(rgb888) \
	(BIT(15) | ((rgb888 & 0x00f80000) >> 9) | ((rgb888 & 0x0000f800) >> 6) | ((rgb888 & 0x000000f8) >> 3))

#define RGN_COLOR_DARK		0x8000
#define RGN_COLOR_BRIGHT	0xffff

enum RGN_OP {
	RGN_OP_UPDATE = 0,
	RGN_OP_INSERT,
	RGN_OP_REMOVE,
};

#ifdef __cplusplus
}
#endif

#endif /* __RGN_COMMON_H__ */
