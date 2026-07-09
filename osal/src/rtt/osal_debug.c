#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

void osal_panic(const char *fmt, const char *fun, int line, const char *cond)
{
	RT_UNUSED(fmt);
	RT_UNUSED(fun);
	RT_UNUSED(line);
	RT_UNUSED(cond);
	osal_log("rt-thread not support osal_panic\n");
}
// EXPORT_SYMBOL(osal_panic);

void osal_dump_stack(void)
{
	rt_ubase_t stack_start, stack_end;
	rt_ubase_t *stack_ptr;
	rt_thread_t thread = rt_thread_self();

	stack_start = (rt_ubase_t)thread->stack_addr;
	stack_end = stack_start + thread->stack_size;

	stack_ptr = (rt_ubase_t *)thread->sp;
#ifdef RT_USING_SMART
	rt_kprintf("Stack dump for thread_Id: %d\n", thread->tid);
#endif
	rt_kprintf(
		"Stack start: 0x%08x, Stack end: 0x%08x, Current SP: 0x%08x\n",
		stack_start, stack_end, (rt_ubase_t)stack_ptr);

	for (rt_ubase_t addr = (rt_ubase_t)stack_ptr; addr < stack_end;
	     addr += sizeof(rt_ubase_t)) {
		rt_kprintf("0x%08x: 0x%08x\n", addr, *(rt_ubase_t *)addr);
	}
}
// EXPORT_SYMBOL(osal_dump_stack);

void osal_printk(const char *fmt, ...)
{
	va_list args;
	char buffer[256];

	if (fmt == NULL) {
		return;
	}

	va_start(args, fmt);
	rt_vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	rt_kprintf("%s", buffer);
}
// EXPORT_SYMBOL(osal_printk);

void osal_printf(const char *fmt, ...)
{
	va_list args;
	char buffer[256];

	if (fmt == NULL) {
		return;
	}

	va_start(args, fmt);
	rt_vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	rt_kprintf("%s", buffer);
}

void osal_printk_auto(const char *fmt, ...)
{
	va_list args;
	char buffer[256];

	if (fmt == NULL) {
		return;
	}

	va_start(args, fmt);
	rt_vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	rt_kprintf("%s", buffer);
}
