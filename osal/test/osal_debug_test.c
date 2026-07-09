#include <osal_test.h>

void test_osal_printk(void)
{
	osal_printk("%s:%d should print osal_printk\n", __func__, __LINE__);
}

void test_osal_printf(void)
{
	osal_printf("%s:%d should print osal_printf\n", __func__, __LINE__);
}

void test_osal_dump_stack(void)
{
	osal_dump_stack();
}

void osal_debug_test(void)
{
	test_osal_printk();
	test_osal_printf();
	test_osal_dump_stack();
}
OSAL_CMD_EXPORT(osal_debug_test, Test debug operations);
