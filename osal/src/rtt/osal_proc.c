#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

typedef struct {
	char *buffer;
	size_t size;
	size_t len;
} proc_info;

static struct osal_list_head proc_list_head;
static struct rt_mutex proc_mutex;

int osal_proc_open(struct osal_proc_dir_entry *entry)
{
	if (entry == NULL || entry->open == NULL) {
		osal_log("Invalid proc entry or open function\n");
		return OSAL_FAILURE;
	}

	return entry->open(entry);
}

int seq_read(struct osal_proc_dir_entry *entry)
{
	if (entry == NULL || entry->read == NULL) {
		osal_log("Invalid proc entry or read function\n");
		return OSAL_FAILURE;
	}

	return entry->read(entry);
}

int osal_proc_write(struct osal_proc_dir_entry *entry, const char *buf, int count, long long *pos)
{
	if (entry == NULL || entry->write == NULL) {
		osal_log("Invalid proc entry or write function\n");
		return OSAL_FAILURE;
	}

	return entry->write(entry, buf, count, pos);
}

void osal_remove_proc_entry(const char *name, osal_proc_entry *parent)
{
	struct osal_list_head *pos, *n;
	osal_proc_entry *entry;

	rt_mutex_take(&proc_mutex, RT_WAITING_FOREVER);

	for (pos = proc_list_head.next; pos != &proc_list_head; pos = n) {
		n = pos->next;
		entry = rt_container_of(pos, osal_proc_entry, node);
		if (rt_strcmp(entry->name, name) == 0) {
			osal_list_del(&entry->node);
			if (entry->private_data) {
				rt_free(entry->private_data);
			}
			rt_free(entry);
			rt_mutex_release(&proc_mutex);
			return;
		}
	}

	rt_mutex_release(&proc_mutex);

	osal_log("Proc entry %s not found\n", name);
}
// EXPORT_SYMBOL(osal_remove_proc_entry);

osal_proc_entry *osal_create_proc_entry(const char *name, osal_proc_entry *parent)
{
	osal_proc_entry *entry = rt_malloc(sizeof(osal_proc_entry));
	if (entry == NULL) {
		osal_log("Failed to allocate memory for proc entry\n");
		return NULL;
	}

	rt_memset(entry, 0, sizeof(osal_proc_entry));
	rt_strncpy(entry->name, name, OSAL_PROC_NAME_LENGTH - 1);
	entry->name[OSAL_PROC_NAME_LENGTH - 1] = '\0';

	rt_mutex_take(&proc_mutex, RT_WAITING_FOREVER);

	OSAL_INIT_LIST_HEAD(&entry->node);
	osal_list_add(&entry->node, &proc_list_head);

	rt_mutex_release(&proc_mutex);

	return entry;
}
// EXPORT_SYMBOL(osal_create_proc_entry);

void osal_seq_printf(void *seqfile, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	proc_info *sf = (proc_info *)seqfile;
	if (sf == NULL || sf->buffer == NULL) {
		osal_log("Invalid seqfile\n");
		return;
	}

	int len = rt_vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	if (sf->len + len + 1 > sf->size) {
		osal_log("Buffer overflow\n");
		return;
	}

	va_start(args, fmt);
	rt_vsnprintf(sf->buffer + sf->len, len + 1, fmt, args);
	va_end(args);

	sf->len += len;
}
// EXPORT_SYMBOL(osal_seq_printf);

void osal_proc_init(const char *name)
{
	osal_proc_entry *entry = osal_create_proc_entry(name, NULL);
	if (entry == NULL) {
		osal_log("Failed to create proc entry for %s\n", name);
		return;
	}

	entry->private_data = rt_malloc(4096);
	if (entry->private_data == NULL) {
		osal_log("Failed to allocate memory for %s\n", name);
		osal_remove_proc_entry(name, NULL);
		return;
	}

	osal_log("Initialized proc entry for %s\n", name);
}
// EXPORT_SYMBOL(osal_proc_init);

void osal_proc_exit(const char *name)
{
	osal_remove_proc_entry(name, NULL);
	osal_log("Exited proc entry for %s\n", name);
}
// EXPORT_SYMBOL(osal_proc_exit);

int osal_proc_mutex_init(void)
{
	if (rt_mutex_init(&proc_mutex, "proc_mutex", RT_IPC_FLAG_FIFO) != RT_EOK) {
		osal_log("proc_mutex init failed, return\n");
		return -RT_ERROR;
	}
	OSAL_INIT_LIST_HEAD(&proc_list_head);
	return RT_EOK;
}
INIT_PREV_EXPORT(osal_proc_mutex_init);