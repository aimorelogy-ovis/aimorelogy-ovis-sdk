#include <rtthread.h>
#include <string.h>
#include "osal.h"
#include "osal_inner.h"

#define MAX_IRQ_NAME_LEN 32

typedef struct irq_info_ {
	unsigned int irq;
	char name[MAX_IRQ_NAME_LEN];
	void *dev;
	struct irq_info *next;
}irq_info;

static irq_info *irq_list = NULL;
static int timer_counter = 0;

void *osal_irq_get_private_dev(void *param_dev)
{
	if (param_dev == NULL) {
		osal_log("Invalid parameter\n");
		return NULL;
	}

	return param_dev;
}
// EXPORT_SYMBOL(osal_irq_get_private_dev);

int osal_irq_request(unsigned int irq, osal_irq_handler handler, osal_irq_handler thread_fn,
	const char *name, void *dev)
{
	if (handler == NULL || name == NULL || dev == NULL) {
		osal_log("osal_irq_request: Invalid parameter\n");
		return OSAL_FAILURE;
	}

	rt_hw_interrupt_mask(irq);

	if (rt_hw_interrupt_install(irq, (rt_isr_handler_t)handler, dev, name) != RT_EOK) {
		osal_log("osal_irq_request: Failed to install interrupt handler\n");
		rt_hw_interrupt_umask(irq);
		return OSAL_FAILURE;
	}

	rt_hw_interrupt_umask(irq);

	irq_info *new_info = (irq_info *)rt_malloc(sizeof(irq_info));
	if (new_info == NULL) {
		osal_log("osal_irq_request: Failed to allocate memory for irq_info\n");
		return OSAL_FAILURE;
	}

	new_info->irq = irq;
	strncpy(new_info->name, name, MAX_IRQ_NAME_LEN - 1);
	new_info->name[MAX_IRQ_NAME_LEN - 1] = '\0';
	new_info->dev = dev;
	new_info->next = (struct irq_info *)irq_list;
	irq_list = new_info;
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_irq_request);

void osal_irq_free(unsigned int irq, void *dev)
{
	if (dev == NULL) {
		osal_log("Invalid parameter\n");
		return;
	}

	rt_hw_interrupt_mask(irq);

	rt_hw_interrupt_uninstall(irq, (rt_isr_handler_t)NULL, dev);

	irq_info *prev = NULL;
	irq_info *current = irq_list;
	while (current != NULL) {
		if (current->irq == irq && current->dev == dev) {
			if (prev == NULL) {
				irq_list = (struct irq_info_ *)current->next;
			} else {
				prev->next = current->next;
			}
			rt_free(current);
			break;
		}
		prev = current;
		current = (struct irq_info_ *)current->next;
	}

	rt_hw_interrupt_umask(irq);
}
// EXPORT_SYMBOL(osal_irq_free);

int osal_irq_set_priority(unsigned int irq, unsigned short priority)
{
	//rtt have no priority api
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_irq_set_priority);

int osal_irq_set_affinity(unsigned int irq, const char *name, int cpu_mask)
{
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_irq_set_affinity);

unsigned int osal_irq_lock(void)
{
	return rt_hw_interrupt_disable();
}
// EXPORT_SYMBOL(osal_irq_lock);

unsigned int osal_irq_unlock(void)
{
	rt_hw_interrupt_enable(0);
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_irq_unlock);

void osal_irq_restore(unsigned int irq_status)
{
	rt_hw_interrupt_enable(irq_status);
}
// EXPORT_SYMBOL(osal_irq_restore);

unsigned int osal_irq_clear(unsigned int vector)
{
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_irq_clear);

int osal_in_interrupt(void)
{
	return rt_interrupt_get_nest() > 0;
}
// EXPORT_SYMBOL(osal_in_interrupt);

void osal_irq_enable(unsigned int irq)
{
	rt_hw_interrupt_umask(irq);
}
// EXPORT_SYMBOL(osal_irq_enable);

/**
 * @par Description:
 * Enables and Disables are nested.
 * This function waits for any pending IRQ handlers for this interrupt to complete before returning.
 * If you use this function while holding a resource the IRQ handler may need you will deadlock.
 * This function may be called - with care - from IRQ context.
 */
void osal_irq_disable(unsigned int irq)
{
	rt_hw_interrupt_mask(irq);
}
// EXPORT_SYMBOL(osal_irq_disable);

/* tasklet is running only on one CPU simultaneously */
int osal_tasklet_init(osal_tasklet *tasklet)
{
	if (tasklet == NULL || tasklet->tasklet != NULL) {
		osal_log("init tasklet is NULL ! \n");
		return OSAL_FAILURE;
	}
	if (tasklet->tasklet != NULL) {
		osal_tasklet_kill(tasklet);
	}

    char timer_name[16];
    rt_snprintf(timer_name, sizeof(timer_name), "int_timer_%d", timer_counter++);

    tasklet->tasklet = rt_timer_create(
        timer_name,
        (void (*)(void*))tasklet->handler,
        (void *)tasklet->data,
        RT_TICK_PER_SECOND,
        RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT
    );
	if (tasklet->tasklet == NULL) {
		osal_log("Failed to initialize timer\n");
		return OSAL_FAILURE;
	}
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_tasklet_init);

/*
 * tasklet update should be called after tasklet init
 **/
int osal_tasklet_update(osal_tasklet *tasklet)
{
	if (tasklet == NULL || tasklet->tasklet == NULL) {
		osal_log("tasklet setdata input is NULL\n");
		return OSAL_FAILURE;
	}
	rt_timer_stop((rt_timer_t)tasklet->tasklet);
	((rt_timer_t)tasklet->tasklet)->timeout_func = (void (*)(void*))tasklet->handler;
	((rt_timer_t)tasklet->tasklet)->parameter = (void*)tasklet->data;

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_tasklet_update);

/*
 * add tasklet to tasklet_soc_vec and start tasklet
 **/
int osal_tasklet_schedule(osal_tasklet *tasklet)
{
	if (tasklet == NULL || tasklet->tasklet == NULL) {
		osal_log("schedule tasklet is NULL ! \n");
		return OSAL_FAILURE;
	}

	rt_timer_start((rt_timer_t)tasklet->tasklet);

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_tasklet_schedule);

/* kill tasklet */
int osal_tasklet_kill(osal_tasklet *tasklet)
{
	if (tasklet == NULL || tasklet->tasklet == NULL) {
		osal_log("kill tasklet is NULL \n");
		return OSAL_FAILURE;
	}

	rt_timer_stop((rt_timer_t)tasklet->tasklet);
	rt_timer_delete((rt_timer_t)tasklet->tasklet);
	tasklet->tasklet = NULL;

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_tasklet_kill);
