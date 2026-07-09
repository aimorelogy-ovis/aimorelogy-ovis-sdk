#ifndef __VI_ATOMIC_H__
#define __VI_ATOMIC_H__

// #include "k_api.h"
// #include "k_atomic.h"
#include "rtthread.h"

static inline void atomic_set(rt_atomic_t *v, int i)
{
    rt_atomic_store(v, i);
}

static inline int atomic_read(const rt_atomic_t *v)
{
    return rt_atomic_load(v);
}

static inline void atomic_inc(rt_atomic_t *v)
{
    rt_atomic_add(v, 1);
}

static inline int atomic_cmpxchg(rt_atomic_t *v, int64_t old, int new_value)
{
    rt_atomic_compare_exchange_strong(v, (rt_atomic_t *)&old, new_value);
    return old;
}

static inline void atomic_dec(rt_atomic_t *v)
{
    rt_atomic_sub(v, 1);
}
#if 0
static inline int atomic_sub_return(int value, atomic_t *v)
{
    CPSR_ALLOC();
    int ret;

    RHINO_CPU_INTRPT_DISABLE();

    *v -= value;
    ret = *v;

    RHINO_CPU_INTRPT_ENABLE();

    return ret;
}

static inline int atomic_add_return(int value, atomic_t *v)
{
    CPSR_ALLOC();
    int ret;

    RHINO_CPU_INTRPT_DISABLE();

    *v += value;
    ret = *v;

    RHINO_CPU_INTRPT_ENABLE();

    return ret;
}
#endif
#endif
