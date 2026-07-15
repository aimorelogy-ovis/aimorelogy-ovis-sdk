#include "ovis_manager.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_TASKS 16
#define CONFIG_RESTART_GRACE_MS 1000

enum task_state { TASK_EMPTY, TASK_QUEUED, TASK_RUNNING, TASK_SUCCEEDED, TASK_FAILED };
enum task_kind { TASK_SERVICE, TASK_CONFIG_APPLY, TASK_CONFIG_RESET };

struct service_task {
	unsigned long id;
	enum task_kind kind;
	enum service_action action;
	enum task_state state;
	int result;
	int progress;
	int rolled_back;
	char revision[33];
	char stage[64];
	char message[512];
};

static struct service_task tasks[MAX_TASKS];
static unsigned long next_task_id = 1;
static pthread_mutex_t tasks_lock = PTHREAD_MUTEX_INITIALIZER;

static void set_task_status(struct service_task *task, enum task_state state,
	const char *stage, int progress, const char *message)
{
	pthread_mutex_lock(&tasks_lock);
	task->state = state;
	task->progress = progress;
	snprintf(task->stage, sizeof(task->stage), "%s", stage);
	if (message != NULL)
		snprintf(task->message, sizeof(task->message), "%s", message);
	pthread_mutex_unlock(&tasks_lock);
}

static void wait_for_config_response(void)
{
	struct timespec remaining = {
		.tv_sec = CONFIG_RESTART_GRACE_MS / 1000,
		.tv_nsec = (CONFIG_RESTART_GRACE_MS % 1000) * 1000000L,
	};

	while (nanosleep(&remaining, &remaining) != 0 && errno == EINTR)
		;
}

static void *task_worker(void *arg)
{
	struct service_task *task = arg;
	enum task_kind kind = task->kind;
	enum service_action action = task->action;
	char revision[33];
	char output[512] = "";
	char result_text[32];
	int rolled_back = 0;
	int rc;

	snprintf(revision, sizeof(revision), "%s", task->revision);
	if (kind == TASK_SERVICE) {
		set_task_status(task, TASK_RUNNING, "controlling_ipcamera", 20,
			"正在执行视频服务操作");
		rc = service_run_action(action, output, sizeof(output));
	} else if (kind == TASK_CONFIG_APPLY) {
		wait_for_config_response();
		set_task_status(task, TASK_RUNNING, "restarting_ipcamera", 60,
			"正在重启视频服务");
		rc = config_apply_staged(revision, output, sizeof(output), &rolled_back);
	} else {
		wait_for_config_response();
		set_task_status(task, TASK_RUNNING, "restoring_defaults", 20,
			"正在恢复默认配置");
		rc = config_apply_defaults(output, sizeof(output), &rolled_back);
	}

	pthread_mutex_lock(&tasks_lock);
	task->result = rc;
	task->rolled_back = rolled_back;
	task->progress = rc == 0 ? 100 : task->progress;
	snprintf(task->stage, sizeof(task->stage), "%s", rc == 0 ? "completed" : "failed");
	snprintf(task->message, sizeof(task->message), "%s",
		output[0] != '\0' ? output : rc == 0 ? "操作成功" : "操作失败");
	task->state = rc == 0 ? TASK_SUCCEEDED : TASK_FAILED;
	pthread_mutex_unlock(&tasks_lock);

	if (kind == TASK_SERVICE) {
		snprintf(result_text, sizeof(result_text), "%d", rc);
		audit_log(action == SERVICE_START ? "ipcamera.start" :
			action == SERVICE_STOP ? "ipcamera.stop" : "ipcamera.restart",
			result_text);
	}
	return NULL;
}

static unsigned long submit_task(enum task_kind kind, enum service_action action,
	const char *revision)
{
	struct service_task *task = NULL;
	pthread_t thread;
	unsigned long id;
	size_t index;

	pthread_mutex_lock(&tasks_lock);
	for (index = 0; index < MAX_TASKS; index++) {
		if (tasks[index].state == TASK_QUEUED || tasks[index].state == TASK_RUNNING) {
			pthread_mutex_unlock(&tasks_lock);
			return 0;
		}
	}
	for (index = 0; index < MAX_TASKS; index++) {
		if (tasks[index].state != TASK_QUEUED && tasks[index].state != TASK_RUNNING) {
			task = &tasks[index];
			break;
		}
	}
	if (task == NULL) {
		pthread_mutex_unlock(&tasks_lock);
		return 0;
	}
	id = next_task_id++;
	memset(task, 0, sizeof(*task));
	task->id = id;
	task->kind = kind;
	task->action = action;
	task->state = TASK_QUEUED;
	task->progress = 0;
	snprintf(task->stage, sizeof(task->stage), "queued");
	snprintf(task->message, sizeof(task->message), "%s",
		kind == TASK_CONFIG_APPLY ? "配置已保存，设备即将重启" :
		kind == TASK_CONFIG_RESET ? "默认配置已准备，设备即将重启" : "任务已排队");
	if (revision != NULL)
		snprintf(task->revision, sizeof(task->revision), "%s", revision);
	if (pthread_create(&thread, NULL, task_worker, task) != 0) {
		task->state = TASK_FAILED;
		snprintf(task->stage, sizeof(task->stage), "failed");
		snprintf(task->message, sizeof(task->message), "无法创建任务线程");
		pthread_mutex_unlock(&tasks_lock);
		return 0;
	}
	pthread_detach(thread);
	pthread_mutex_unlock(&tasks_lock);
	return id;
}

unsigned long task_submit(enum service_action action)
{
	return submit_task(TASK_SERVICE, action, NULL);
}

unsigned long config_task_submit_apply(const char *revision)
{
	if (revision == NULL || revision[0] == '\0' || strlen(revision) > 32)
		return 0;
	return submit_task(TASK_CONFIG_APPLY, SERVICE_RESTART, revision);
}

unsigned long config_task_submit_reset(void)
{
	return submit_task(TASK_CONFIG_RESET, SERVICE_RESTART, NULL);
}

int task_get(unsigned long id, char *json, size_t size)
{
	struct service_task snapshot;
	struct service_task *task = NULL;
	const char *state;
	char escaped_message[1024];
	char escaped_stage[128];
	size_t index;

	pthread_mutex_lock(&tasks_lock);
	for (index = 0; index < MAX_TASKS; index++) {
		if (tasks[index].id == id) {
			task = &tasks[index];
			break;
		}
	}
	if (id == 0 || task == NULL) {
		pthread_mutex_unlock(&tasks_lock);
		return -1;
	}
	snapshot = *task;
	pthread_mutex_unlock(&tasks_lock);
	state = snapshot.state == TASK_QUEUED ? "queued" :
		snapshot.state == TASK_RUNNING ? "running" :
		snapshot.state == TASK_SUCCEEDED ? "succeeded" : "failed";
	json_escape(snapshot.message, escaped_message, sizeof(escaped_message));
	json_escape(snapshot.stage, escaped_stage, sizeof(escaped_stage));
	if (snprintf(json, size,
			"{\"id\":%lu,\"state\":\"%s\",\"stage\":\"%s\","
			"\"progress\":%d,\"message\":\"%s\",\"rolled_back\":%s}",
			id, state, escaped_stage, snapshot.progress, escaped_message,
			snapshot.rolled_back ? "true" : "false") >= (int)size)
		return -1;
	return 0;
}
