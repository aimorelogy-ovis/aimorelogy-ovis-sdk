#include <aos/kernel.h>
#include "osal.h"

#include "osal_inner.h"

#define RISCV_FENCE(p, s) \
	__asm__ __volatile__ ("fence " #p "," #s : : : "memory")

void osal_mb(void)
{
#if defined(__riscv)
    RISCV_FENCE(iorw,iorw);
#endif
}

void osal_rmb(void)
{
#if defined(__riscv)
    RISCV_FENCE(ir,ir);
#endif
}

void osal_wmb(void)
{
#if defined(__riscv)
    RISCV_FENCE(ow,ow);
#endif
}

void osal_smp_mb(void)
{
#if defined(__riscv)
    RISCV_FENCE(rw,rw);
#endif
}

void osal_smp_rmb(void)
{
#if defined(__riscv)
    RISCV_FENCE(r,r);
#endif
}

void osal_smp_wmb(void)
{
#if defined(__riscv)
    RISCV_FENCE(w,w);
#endif
}

void osal_isb(void)
{
#if defined(__CC_ARM)
    OS_ISB();
#endif
}

void osal_dsb(void)
{
#if defined(__CC_ARM)
    OS_DSB();
#endif
}

void osal_dmb(void)
{
#if defined(__CC_ARM)
    OS_DMB();
#endif
}
