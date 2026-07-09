#include <aos/kernel.h>
#include <stdio.h>
#include <stdarg.h>
#include "k_api.h"
#include "osal.h"

#include "osal_inner.h"

void osal_printf(const char *fmt, ...)
{
    va_list args;

    if (fmt == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void osal_dump_stack(void)
{
    aos_task_t current_task = aos_task_self();
    if (current_task == NULL) {
        osal_log("Failed to get current task. caller: %p\n", __builtin_return_address(0));
        return;
    }

    void *stack_start = ((ktask_t *)(current_task))->task_stack_base;
    size_t stack_size = ((ktask_t *)(current_task))->stack_size;
    void *stack_ptr = ((ktask_t *)(current_task))->task_stack;

    printf("Stack dump for thread: %s\n", ((ktask_t *)(current_task))->task_name);
    printf("Stack start: %p, Stack end: %p, Current SP: %p\n",
           stack_start,
           (void *)((uintptr_t)stack_start + stack_size * sizeof(cpu_stack_t)),
           stack_ptr);

    for (uintptr_t addr = (uintptr_t)stack_ptr; addr < (uintptr_t)stack_start + stack_size * sizeof(cpu_stack_t); addr += sizeof(uintptr_t))
    {
        printf("0x%08lx: 0x%08lx\n", addr, *(uintptr_t *)addr);
    }
}

void osal_printk(const char *fmt, ...)
{
    va_list args;

    if (fmt == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

#if CONFIG_LOG_TO_DMESG
#include "dmesg.h"
#endif
void osal_printk_auto(const char *fmt, ...)
{
    va_list args;

    if (fmt == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    va_start(args, fmt);
#if CONFIG_LOG_TO_DMESG
    vprintk_func(fmt, args);
#else
    vprintf(fmt, args);
#endif
    va_end(args);
}

