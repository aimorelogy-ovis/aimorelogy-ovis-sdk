#include <aos/kernel.h>
#include <k_api.h>
#include "k_atomic.h"
#include "mmio.h"
#include "osal.h"
#include "osal_inner.h"
#include "osal_list.h"


#undef BIT
#define BIT(nr)      ((1ULL) << (nr))

struct clk_item {
	osal_clk_ctrl_info ctrl;
	atomic_t prepare_cnt;
	atomic_t enable_cnt;
	kspinlock_t lock;
	struct osal_list_head list;
};


static OSAL_LIST_HEAD(g_clk_list);
kspinlock_t g_clk_spinlock;


int osal_clk_register(osal_clk_ctrl_info *info)
{
	struct clk_item *item;

	if (info->bit >= 32) {
        osal_log("bit error.\n");
        return OSAL_FAILURE;
	}
	if (!info->name[0]) {
        osal_log("name error.\n");
        return OSAL_FAILURE;
	}

	item = (struct clk_item *)aos_malloc(sizeof(struct clk_item));
	if (item == NULL) {
        osal_log("aos_malloc failed.\n");
        return OSAL_FAILURE;
	}
	item->ctrl = *info;
	rhino_atomic_set(&item->prepare_cnt, 0);
	rhino_atomic_set(&item->enable_cnt, 0);
	krhino_spin_lock_init(&item->lock);

	krhino_spin_lock(&g_clk_spinlock);
	osal_list_add_tail(&item->list, &g_clk_list);
	krhino_spin_unlock(&g_clk_spinlock);

	return OSAL_SUCCESS;
}

void osal_clk_unregister(osal_clk_ctrl_info *info)
{
	struct clk_item *item = NULL;
	int is_find = 0;

	krhino_spin_lock(&g_clk_spinlock);
	osal_list_for_each_entry(item, &g_clk_list, list) {
		if (!strncmp(item->ctrl.name, info->name, 32)) {
			osal_list_del_init(&item->list);
			is_find = 1;
			break;
		}
	}
	krhino_spin_unlock(&g_clk_spinlock);

	if (is_find)
		aos_free(item);
}

osal_clk *osal_clk_get(void *device, const char *name)
{
	struct clk_item *item = NULL;
	int is_find = 0;
	osal_clk *clk;

	krhino_spin_lock(&g_clk_spinlock);
	osal_list_for_each_entry(item, &g_clk_list, list) {
		if (!strncmp(item->ctrl.name, name, 32)) {
			is_find = 1;
			break;
		}
	}
	krhino_spin_unlock(&g_clk_spinlock);

	if (!is_find)
		return NULL;

	clk = (osal_clk *)aos_malloc(sizeof(osal_clk));
	if (clk == NULL) {
		osal_log("aos_malloc error!\n");
		return NULL;
	}
	clk->clk = item;

	return clk;
}

void osal_clk_put(void *device, osal_clk *clk)
{
	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}

	clk->clk = NULL;
	aos_free(clk);
}

void osal_clk_prepare(osal_clk *clk)
{
	struct clk_item *item = NULL;

	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	item = (struct clk_item *)clk->clk;
	//TODO: prepare
	rhino_atomic_inc(&item->prepare_cnt);
}

void osal_clk_unprepare(osal_clk *clk)
{
	struct clk_item *item = NULL;

	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	item = (struct clk_item *)clk->clk;
	//TODO: unprepare

	if (rhino_atomic_get(&item->prepare_cnt) == 0)
		return;

	rhino_atomic_dec(&item->prepare_cnt);
}

void osal_clk_enable(osal_clk *clk)
{
	struct clk_item *item = NULL;
	unsigned long flags;
	unsigned long reg_addr;
	unsigned int bit;

	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	item = (struct clk_item *)clk->clk;
	reg_addr= item->ctrl.reg_addr;
	bit = item->ctrl.bit;

	if (rhino_atomic_get(&item->prepare_cnt) == 0) {
		osal_log("clk:%s unprepare! caller: %p\n", item->ctrl.name, __builtin_return_address(0));
	}

	krhino_spin_lock_irq_save(&item->lock, flags);
	if (!(mmio_read_32(reg_addr) & BIT(bit))) {
		mmio_clrsetbits_32(reg_addr, BIT(bit), BIT(bit));
	}
	rhino_atomic_inc(&item->enable_cnt);
	krhino_spin_unlock_irq_restore(&item->lock, flags);
}

void osal_clk_disable(osal_clk *clk)
{
	struct clk_item *item = NULL;
	unsigned long flags;
	unsigned long reg_addr;
	unsigned int bit, value;

	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	item = (struct clk_item *)clk->clk;
	reg_addr= item->ctrl.reg_addr;
	bit = item->ctrl.bit;

	if (rhino_atomic_get(&item->prepare_cnt) == 0) {
		osal_log("clk:%s unprepare! caller: %p\n", item->ctrl.name, __builtin_return_address(0));
	}

	krhino_spin_lock_irq_save(&item->lock, flags);
	value = rhino_atomic_get(&item->enable_cnt);
	if (value == 0) {
		krhino_spin_unlock_irq_restore(&item->lock, flags);
		return;
	} else if (value == 1) {
		mmio_clrsetbits_32(reg_addr, BIT(bit), 0);
	}
	rhino_atomic_dec(&item->enable_cnt);
	krhino_spin_unlock_irq_restore(&item->lock, flags);
}

void osal_clk_prepare_enable(osal_clk *clk)
{
	struct clk_item *item = NULL;
	unsigned long flags;
	unsigned long reg_addr;
	unsigned int bit;

	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	item = (struct clk_item *)clk->clk;
	reg_addr= item->ctrl.reg_addr;
	bit = item->ctrl.bit;

	krhino_spin_lock_irq_save(&item->lock, flags);
	rhino_atomic_inc(&item->prepare_cnt);
	rhino_atomic_inc(&item->enable_cnt);
	if (!(mmio_read_32(reg_addr) & BIT(bit))) {
		mmio_clrsetbits_32(reg_addr, BIT(bit), BIT(bit));
	}
	krhino_spin_unlock_irq_restore(&item->lock, flags);
}

void osal_clk_disable_unprepare(osal_clk *clk)
{
	struct clk_item *item = NULL;
	unsigned long flags;
	unsigned long reg_addr;
	unsigned int bit, value;

	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return;
	}
	item = (struct clk_item *)clk->clk;
	reg_addr= item->ctrl.reg_addr;
	bit = item->ctrl.bit;

	krhino_spin_lock_irq_save(&item->lock, flags);
	value = rhino_atomic_get(&item->enable_cnt);
	if (value == 0) {
	} else if (value == 1) {
		mmio_clrsetbits_32(reg_addr, BIT(bit), 0);
		rhino_atomic_dec(&item->enable_cnt);
	} else {
		rhino_atomic_dec(&item->enable_cnt);
	}

	if (rhino_atomic_get(&item->prepare_cnt) > 0)
		rhino_atomic_dec(&item->prepare_cnt);
	krhino_spin_unlock_irq_restore(&item->lock, flags);
}

bool osal_clk_is_enabled(osal_clk *clk)
{
	struct clk_item *item = NULL;
	unsigned long reg_addr;
	unsigned int bit;
	bool ret = false;

	if (clk == NULL || clk->clk == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return false;
	}
	item = (struct clk_item *)clk->clk;
	reg_addr= item->ctrl.reg_addr;
	bit = item->ctrl.bit;

	if (rhino_atomic_get(&item->enable_cnt) > 0) {
		ret = true;
	} else {
		if ((mmio_read_32(reg_addr) & BIT(bit)))
			ret = true;
	}

	return ret;
}

