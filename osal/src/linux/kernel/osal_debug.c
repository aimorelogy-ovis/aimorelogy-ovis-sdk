#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include "osal.h"
#include "osal_inner.h"

void osal_panic(const char *fmt, const char *fun, int line, const char *cond)
{
    panic(fmt, fun, line, cond);
}
EXPORT_SYMBOL(osal_panic);

void osal_dump_stack(void)
{
    dump_stack();
}
EXPORT_SYMBOL(osal_dump_stack);

void osal_printk(const char *fmt, ...)
{
    va_list args;

    if (fmt == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
EXPORT_SYMBOL(osal_printk);

void osal_printf(const char *fmt, ...)
{
    va_list args;
    char buffer[256];

    if (fmt == NULL) {
        return;
    }

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    printk(KERN_INFO "%s", buffer);
}
EXPORT_SYMBOL(osal_printf);

void osal_printk_auto(const char *fmt, ...)
{
   va_list args;
    char buffer[256];

    if (fmt == NULL) {
        return;
    }

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    printk(KERN_INFO "%s", buffer);
}
EXPORT_SYMBOL(osal_printk_auto);
