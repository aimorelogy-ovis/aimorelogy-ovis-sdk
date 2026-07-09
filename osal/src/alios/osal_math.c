#include <aos/kernel.h>
#include <aos/debug.h>
#include <stdint.h>
#include "osal_inner.h"
#include "osal.h"

/* the result of u64/u32. */
unsigned long long osal_div_u64(unsigned long long dividend, unsigned int divisor)
{
    if (divisor == 0) {
        osal_log("divisor is zero! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    return dividend / divisor;
}

/* the result of s64/s32. */
long long osal_div_s64(long long dividend, int divisor)
{
    if (divisor == 0) {
        osal_log("divisor is zero! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    return dividend / divisor;
}

/* the result of u64/u64. */
unsigned long long osal_div64_u64(unsigned long long dividend, unsigned long long divisor)
{
    if (divisor == 0) {
        osal_log("divisor is zero! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    return dividend / divisor;
}

/* the result of s64/s64. */
long long osal_div64_s64(long long dividend, long long divisor)
{
    if (divisor == 0) {
        osal_log("divisor is zero! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    return dividend / divisor;
}

/* the remainder of u64/u32. */
unsigned long long osal_div_u64_rem(unsigned long long dividend, unsigned int divisor)
{
    if (divisor == 0) {
        osal_log("divisor is zero! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    return dividend % divisor;
}

/* the remainder of s64/s32. */
long long osal_div_s64_rem(long long dividend, int divisor)
{
    if (divisor == 0) {
        osal_log("divisor is zero! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    return dividend % divisor;
}

/* the remainder of u64/u64. */
unsigned long long osal_div64_u64_rem(unsigned long long dividend, unsigned long long divisor)
{
    if (divisor == 0) {
        osal_log("divisor is zero! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    return dividend % divisor;
}

#define LCG_A 1664525
#define LCG_C 1013904223
#define LCG_M 4294967296ULL // 2^32
static uint32_t lcg_state = 1;
unsigned int osal_get_random_int(void)
{
    lcg_state = (LCG_A * lcg_state + LCG_C) % LCG_M;
    return (uint32_t)lcg_state;
}