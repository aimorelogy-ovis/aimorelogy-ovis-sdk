#ifndef __OSAL_TYPES_H__
#define __OSAL_TYPES_H__

#ifdef __KERNEL__
#include <linux/types.h>
#else

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;

typedef signed char             s8;
typedef short                   s16;
typedef int                     s32;
typedef long long               s64;

//typedef unsigned char           bool;

typedef unsigned char           __u8;
typedef unsigned short          __u16;
typedef unsigned int            __u32;
typedef unsigned long long      __u64;

typedef signed char             __s8;
typedef short                   __s16;
typedef int                     __s32;
typedef long long               __s64;

#endif


#endif // __OSAL_TYPES_H__
