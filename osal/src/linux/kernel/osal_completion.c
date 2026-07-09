#include <linux/slab.h>
#include <linux/completion.h>
#include "osal.h"
#include "osal_inner.h"

int osal_completion_init(osal_completion *com)
{
    if (com == NULL || com->completion != NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    com->completion = kmalloc(sizeof(struct completion), GFP_KERNEL);
    if (com->completion == NULL) {
        osal_log("kmalloc failed!\n");
        return OSAL_FAILURE;
    }
    init_completion(com->completion);
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_completion_init);

void osal_completion_reinit(osal_completion *com)
{
    if (com == NULL || com->completion == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    reinit_completion(com->completion);
}
EXPORT_SYMBOL(osal_completion_reinit);

void osal_complete(osal_completion *com)
{
    if (com == NULL || com->completion == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    complete(com->completion);
}
EXPORT_SYMBOL(osal_complete);

void osal_wait_for_completion(osal_completion *com)
{
    if (com == NULL || com->completion == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    wait_for_completion(com->completion);
}
EXPORT_SYMBOL(osal_wait_for_completion);

/*
 * Return: 0 if timed out, and positive (at least 1, or number of jiffies left
 * till timeout) if completed.
 */
unsigned long osal_wait_for_completion_timeout(osal_completion *com, unsigned long timeout)
{
    if (com == NULL || com->completion == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    return wait_for_completion_timeout(com->completion, timeout) ? OSAL_SUCCESS : OSAL_FAILURE;
}
EXPORT_SYMBOL(osal_wait_for_completion_timeout);

void osal_complete_all(osal_completion *com)
{
    if (com == NULL || com->completion == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    complete_all(com->completion);
}
EXPORT_SYMBOL(osal_complete_all);

void osal_complete_destory(osal_completion *com)
{
    if (com == NULL || com->completion == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    kfree(com->completion);
    com->completion = NULL;
}
EXPORT_SYMBOL(osal_complete_destory);
