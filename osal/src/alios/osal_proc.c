#include <aos/kernel.h>
#include <aos/cli.h>
#include <stdio.h>
#include <stdarg.h>

#include "osal.h"
#include "osal_inner.h"

static osal_mutex g_mutex_proc;
static struct osal_list_head g_list;


static void osal_remove_all_proc_entry(void)
{
    osal_proc_entry *sproc = NULL;

    osal_mutex_lock(&g_mutex_proc);
    if (!osal_list_empty(&g_list)) {
		sproc = osal_list_first_entry(&g_list, osal_proc_entry, node);
        osal_list_del_init(&sproc->node);
		aos_free(sproc);
    }
    osal_mutex_unlock(&g_mutex_proc);
}

static osal_proc_entry *osal_proc_find_node(const char *name, const struct osal_list_head *list)
{
    osal_proc_entry *node = NULL;

    if (list == NULL || name == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return NULL;
    }
    osal_list_for_each_entry(node, list, node) {
        if (osal_strcmp(node->name, name) == 0) {
            return node;
        }
    }

    return NULL;
}

static int osal_proc_cmd_call(osal_proc_entry *entry, unsigned int argc, char (*argv)[PROC_CMD_SINGEL_LENGTH_MAX])
{
    unsigned int i;

    for (i = 0; i < entry->cmd_cnt; i++) {
        if (osal_strcmp(entry->cmd_list[i].name, argv[0]) == 0) {
            if (entry->cmd_list[i].handler != NULL) {
                return entry->cmd_list[i].handler(argc, argv, entry->private_data);
            }
        }
    }

    return -1;
}

static int osal_proc_write(osal_proc_entry *entry, int argc, char **argv)
{
    int i, ret = -1;
	int len, total_len = 0;
	int index = 0;
	char *buf;
    char argv_tmp[PROC_CMD_NUM_MAX][PROC_CMD_SINGEL_LENGTH_MAX];

    osal_memset(&argv_tmp[0][0], 0, PROC_CMD_NUM_MAX * PROC_CMD_SINGEL_LENGTH_MAX);

	for (i = 0; i < argc; i++) {
		len = osal_strlen(argv[i]) + 1;
		if (len > PROC_CMD_SINGEL_LENGTH_MAX) {
			osal_log("cmd(%s) is to large!\n", argv[i]);
		}
		osal_strncpy(argv_tmp[i], argv[i], len);
		total_len += len;
	}

    ret = osal_proc_cmd_call(entry, argc, argv_tmp);
    if (ret != 0) {
        osal_log("cmd is not find or param is wrong!\n");
        osal_strncpy(argv_tmp[0], "help", osal_strlen("help"));
        osal_proc_cmd_call(entry, 1, argv_tmp);
    }

    if (entry->write != NULL) {
		buf = aos_calloc(1, total_len);
		if (buf == NULL) {
			osal_log("aos_calloc failed!\n");
			return -1;
		}

		for (i = 0; i < argc; i++) {
			len = osal_strlen(argv[i]);
			osal_strncpy(buf + index, argv[i], len);
			index += len + 1;
		}

        ret =  entry->write(entry, buf, total_len, 0);
		aos_free(buf);
    }

    return ret;
}

osal_proc_entry *osal_create_proc_entry(const char *name, osal_proc_entry *parent)
{
    osal_proc_entry *sentry = NULL;

    if (name == NULL) {
        return NULL;
    }

    sentry = aos_malloc(sizeof(osal_proc_entry));
    if (sentry == NULL) {
        osal_log("aos_malloc failed!\n");
        return NULL;
    }

    osal_memset(sentry, 0, sizeof(osal_proc_entry));

    osal_strncpy(sentry->name, name, sizeof(name) - 1);

    osal_mutex_lock(&g_mutex_proc);
    osal_list_add_tail(&sentry->node, &g_list);
    osal_mutex_unlock(&g_mutex_proc);
    return sentry;

}

void osal_remove_proc_entry(const char *name, osal_proc_entry *parent)
{
    osal_proc_entry *sproc = NULL;

    if (osal_list_empty(&g_list) != 0) {
        return;
    }
    if (name == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    osal_mutex_lock(&g_mutex_proc);
    sproc = osal_proc_find_node(name, &g_list);
    if (sproc == NULL) {
		osal_mutex_unlock(&g_mutex_proc);
        osal_log("name: %s not found!\n", name);
        return;
    }
    osal_list_del(&(sproc->node));
    osal_mutex_unlock(&g_mutex_proc);
    aos_free(sproc);
}

void osal_seq_printf(void *seqfile, const char *fmt, ...)
{
    va_list args;

    osal_unused(seqfile);

    if (fmt == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}


static void osal_proc_help(void)
{
	printf(
		"\n"
		"*************************************************\n"
		"Usage: proc module [args]\n"
		"1)module: \n"
		"	sys log vb vi_dbg vi vpss vo venc gdc rgn\n"
		"*)Example:\n"
		"	e.g : proc vpss\n"
		"	e.g : proc vpss help\n"
		"*************************************************\n"
		"\n");
}

static void osal_proc_handler(char *outbuf, int len, int argc, char **argv)
{
    osal_proc_entry *sproc = NULL;
	char *name = NULL;

    osal_unused(outbuf);
    osal_unused(len);

	if (argc < 2) {
        osal_proc_help();
		return;
	}
	name = argv[1];

    osal_mutex_lock(&g_mutex_proc);
    sproc = osal_proc_find_node(name, &g_list);
    if (sproc == NULL) {
		osal_mutex_unlock(&g_mutex_proc);
        osal_log("name: %s not found!\n", name);
		osal_proc_help();
        return;
    }
    osal_mutex_unlock(&g_mutex_proc);

	if (argc > 2) {
		osal_proc_write(sproc, argc - 2, argv + 2);
		return;
	}

    if (sproc->open != NULL) {
        sproc->open(sproc);
    }
    if (sproc->read != NULL) {
        sproc->read(sproc);
    }
}

static struct cli_command proc_cmd = {
	.name = "proc",
	.help = "media proc info",
	.function = osal_proc_handler,
};

void osal_proc_init(const char *name)
{
    int ret;
    osal_unused(name);

    ret = osal_mutex_init(&g_mutex_proc);
    if (ret != 0) {
        osal_log("workqueue mutex init failed! ret = %#x.\n", ret);
        return;
    }
    osal_mutex_lock(&g_mutex_proc);
    OSAL_INIT_LIST_HEAD(&g_list);
    osal_mutex_unlock(&g_mutex_proc);

	aos_cli_register_command(&proc_cmd);
}

void osal_proc_exit(const char *name)
{
    osal_unused(name);
	osal_remove_all_proc_entry();
	aos_cli_unregister_command(&proc_cmd);

    osal_mutex_lock(&g_mutex_proc);
    g_list.prev = NULL;
    g_list.next = NULL;
    osal_mutex_unlock(&g_mutex_proc);
    osal_mutex_destroy(&g_mutex_proc);
}

