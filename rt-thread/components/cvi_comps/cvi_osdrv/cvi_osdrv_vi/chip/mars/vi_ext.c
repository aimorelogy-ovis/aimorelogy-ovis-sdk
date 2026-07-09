#include <vi_tun_cfg.h>
#include <vi_ext.h>
#include <vi_common.h>

/* isp sync task */
struct isp_sync_tsk_ctx g_isp_sync_tsk_ctx[ISP_PRERAW_VIRT_MAX];

#define isp_sync_tsk_get_ctx(dev) (&g_isp_sync_tsk_ctx[dev])

static void isp_sync_task_find_and_execute(int vi_pipe, struct list_head *head, struct isp_sync_task_data *data)
{
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct isp_sync_task_node *sync_tsk_node = NULL;

	if (!list_empty(head)) {
		list_for_each_safe(pos, next, head) {
			sync_tsk_node = clist_entry(pos, struct isp_sync_task_node, list);

			if (sync_tsk_node->isp_sync_tsk_call_back) {
				sync_tsk_node->isp_sync_tsk_call_back(data);
			}
		}
	}
}

static struct list_head *search_node(struct list_head *head, char *name)
{
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct isp_sync_task_node *sync_tsk_node = NULL;

	if (head == NULL) {
		vi_pr(VI_ERR, "head is null\n");
		return NULL;
	}
	list_for_each_safe(pos, next, head) {
		sync_tsk_node = clist_entry(pos, struct isp_sync_task_node, list);
		if (!strncmp(sync_tsk_node->name, name, ISP_SYNC_TASK_ID_MAX_LENGTH)) {
			return pos;
		}
	}

	return NULL;
}

int isp_sync_task_register(int vi_pipe, struct isp_sync_task_node *new_node)
{
	struct isp_sync_tsk_ctx *sync_tsk = isp_sync_tsk_get_ctx(vi_pipe);
	struct isp_sync_list_entry *list_entry_tmp = NULL;
	struct list_head *target_list = NULL;
	struct list_head *pos = NULL;

	if (new_node == NULL) {
		vi_pr(VI_ERR, "vi_pipe[%d] node is null\n", vi_pipe);
		return -1;
	}

	if (new_node->name == NULL) {
		vi_pr(VI_ERR, "vi_pipe[%d] node name is null\n", vi_pipe);
		return -1;
	}

	target_list = &sync_tsk->work_list.head;

	list_entry_tmp = clist_entry(target_list, struct isp_sync_list_entry, head);
	if (list_entry_tmp == NULL) {
		vi_pr(VI_ERR, "vi_pipe[%d] list is null\n", vi_pipe);
		return -1;
	}

	if (list_entry_tmp->num >= ISP_SYNC_TSK_MAX_NUM) {
		vi_pr(VI_ERR, "vi_pipe[%d] list num[%d] exceeds the maximum[%d]\n",
				vi_pipe, list_entry_tmp->num, ISP_SYNC_TSK_MAX_NUM);
		return -1;
	}

	pos = search_node(target_list, new_node->name);
	if (pos) {
		vi_pr(VI_ERR, "vi_pipe[%d] already register task[%s]\n", vi_pipe, new_node->name);
		return -1;
	}

	pthread_mutex_lock(&sync_tsk->lock);
	list_add_tail(&new_node->list, target_list);
	list_entry_tmp->num++;
	pthread_mutex_unlock(&sync_tsk->lock);

	vi_pr(VI_INFO, "vi_pipe[%d] list num[%d] name[%s]\n",
			vi_pipe, list_entry_tmp->num, new_node->name);
	return 0;
}

int isp_sync_task_unregister(int vi_pipe, struct isp_sync_task_node *del_node)
{
	struct isp_sync_tsk_ctx *sync_tsk = isp_sync_tsk_get_ctx(vi_pipe);
	struct isp_sync_list_entry *list_entry_tmp = NULL;
	struct list_head *target_list = NULL;
	struct list_head *pos = NULL;
	int del_success = -1;

	if (del_node == NULL) {
		vi_pr(VI_ERR, "vi_pipe[%d] node is null\n", vi_pipe);
		return -1;
	}

	if (del_node->name == NULL) {
		vi_pr(VI_ERR, "vi_pipe[%d] node name is null\n", vi_pipe);
		return -1;
	}

	target_list = &sync_tsk->work_list.head;

	list_entry_tmp = clist_entry(target_list, struct isp_sync_list_entry, head);
	if (list_entry_tmp == NULL) {
		vi_pr(VI_ERR, "vi_pipe[%d] list is null\n", vi_pipe);
		return -1;
	}

	pthread_mutex_lock(&sync_tsk->lock);
	pos = search_node(target_list, del_node->name);
	if (pos) {
		list_del(pos);
		if (list_entry_tmp->num > 0) {
			list_entry_tmp->num = list_entry_tmp->num - 1;
		}

		del_success = 0;
	}
	pthread_mutex_unlock(&sync_tsk->lock);

	vi_pr(VI_INFO, "vi_pipe[%d] del_success[%d] list num[%d]\n",
			vi_pipe, del_success, list_entry_tmp->num);
	return del_success;
}

int isp_sync_task_process(int vi_pipe, struct isp_sync_task_data *data)
{
	struct isp_sync_tsk_ctx *sync_tsk = isp_sync_tsk_get_ctx(vi_pipe);

	if (sync_tsk->work_list.num) {
		isp_sync_task_find_and_execute(vi_pipe, &sync_tsk->work_list.head, data);
	}

	return 0;
}

void sync_task_init(int vi_pipe)
{
	struct isp_sync_tsk_ctx *sync_tsk = isp_sync_tsk_get_ctx(vi_pipe);

	INIT_LIST_HEAD(&sync_tsk->work_list.head);

	sync_tsk->work_list.num = 0;
	pthread_mutex_destroy(&sync_tsk->lock);

	vi_pr(VI_INFO, "vi_pipe[%d]\n", vi_pipe);
}

void sync_task_exit(int vi_pipe)
{
	struct isp_sync_tsk_ctx *sync_tsk = isp_sync_tsk_get_ctx(vi_pipe);

	sync_tsk->work_list.num = 0;
	pthread_mutex_init(&sync_tsk->lock, NULL);

	vi_pr(VI_INFO, "vi_pipe[%d]\n", vi_pipe);
}
