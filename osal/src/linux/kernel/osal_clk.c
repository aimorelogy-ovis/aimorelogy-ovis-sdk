#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include "osal.h"
#include "osal_inner.h"


int osal_clk_register(osal_clk_ctrl_info *info)
{
	info = info;
	return 0;
}

void osal_clk_unregister(osal_clk_ctrl_info *info)
{
	info = info;
}

osal_clk *osal_clk_get(void *device, const char *name)
{
	struct clk *clock = NULL;
	osal_clk *clk;

	clock = devm_clk_get((struct device *)device, name);
	if (IS_ERR(clock)) {
		osal_log("devm_clk_get fail ret %d!\n", (int)PTR_ERR(clock));
		return NULL;
	}

	clk = (osal_clk *)kmalloc(sizeof(osal_clk), GFP_KERNEL);
	if (clk == NULL) {
		devm_clk_put((struct device *)device, clock);
		osal_log("kmalloc error!\n");
		return NULL;
	}
	clk->clk = clock;

	return clk;
}
EXPORT_SYMBOL(osal_clk_get);

void osal_clk_put(void *device, osal_clk *clk)
{
	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}

	devm_clk_put((struct device *)device, (struct clk *)clk->clk);
	clk->clk = NULL;
	kfree(clk);
}
EXPORT_SYMBOL(osal_clk_put);

void osal_clk_prepare(osal_clk *clk)
{
	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	clk_prepare((struct clk *)clk->clk);
}
EXPORT_SYMBOL(osal_clk_prepare);

void osal_clk_unprepare(osal_clk *clk)
{
	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	clk_unprepare((struct clk *)clk->clk);
}
EXPORT_SYMBOL(osal_clk_unprepare);

void osal_clk_enable(osal_clk *clk)
{
	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	clk_enable((struct clk *)clk->clk);
}
EXPORT_SYMBOL(osal_clk_enable);

void osal_clk_disable(osal_clk *clk)
{
	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	clk_disable((struct clk *)clk->clk);
}
EXPORT_SYMBOL(osal_clk_disable);

void osal_clk_prepare_enable(osal_clk *clk)
{
	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	clk_prepare_enable((struct clk *)clk->clk);
}
EXPORT_SYMBOL(osal_clk_prepare_enable);

void osal_clk_disable_unprepare(osal_clk *clk)
{
	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	clk_disable_unprepare((struct clk *)clk->clk);
}
EXPORT_SYMBOL(osal_clk_disable_unprepare);

bool osal_clk_is_enabled(osal_clk *clk)
{
	return __clk_is_enabled((struct clk *)clk->clk);
}
EXPORT_SYMBOL(osal_clk_is_enabled);

