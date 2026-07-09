#include <rtthread.h>
#include "osal.h"

void osal_mb(void)
{
	__sync_synchronize();
}
// EXPORT_SYMBOL(osal_mb);
void osal_rmb(void)
{
	__sync_synchronize();
}
// EXPORT_SYMBOL(osal_rmb);
void osal_wmb(void)
{
	__sync_synchronize();
}
// EXPORT_SYMBOL(osal_wmb);
void osal_smp_mb(void)
{
	__sync_synchronize();
}
// EXPORT_SYMBOL(osal_smp_mb);
void osal_smp_rmb(void)
{
	__sync_synchronize();
}
// EXPORT_SYMBOL(osal_smp_rmb);
void osal_smp_wmb(void)
{
	__sync_synchronize();
}
// EXPORT_SYMBOL(osal_smp_wmb);
void osal_isb(void)
{
	__asm__ volatile ("" : : : "memory");
}
// EXPORT_SYMBOL(osal_isb);
void osal_dsb(void)
{
	__asm__ volatile ("" : : : "memory");
}
// EXPORT_SYMBOL(osal_dsb);
void osal_dmb(void)
{
	__asm__ volatile ("" : : : "memory");
}
// EXPORT_SYMBOL(osal_dmb);
