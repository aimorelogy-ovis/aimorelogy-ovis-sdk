#ifndef __U_VI_EXT_H__
#define __U_VI_EXT_H__

#define ISP_SYNC_TSK_MAX_NUM        4
#define ISP_SYNC_TASK_ID_MAX_LENGTH 64

#include <vi_uapi.h>
#include <vip_list.h>
#include <pthread.h>
#include <cvi_comm_vi.h>

struct isp_sync_task_data {
	int vi_pipe;
	VI_SYNC_EVENT_E sync_event;
	void *data;
	int value;
};

struct isp_sync_task_node {
	__s32 (*isp_sync_tsk_call_back)(struct isp_sync_task_data *data);
	__u64 data;
	char *name;
	struct list_head list;
};

struct isp_sync_list_entry {
	__u32 num;
	struct list_head head;
};

struct isp_sync_tsk_ctx {
	int vi_pipe;
	struct isp_sync_list_entry work_list;
	struct pthread_mutex lock;
};

int isp_sync_task_process(int vi_pipe, struct isp_sync_task_data *data);
int isp_sync_task_register(int vi_pipe, struct isp_sync_task_node *new_node);
int isp_sync_task_unregister(int vi_pipe, struct isp_sync_task_node *del_node);
void sync_task_init(int vi_pipe);
void sync_task_exit(int vi_pipe);

#endif // __U_VI_EXT_H__
