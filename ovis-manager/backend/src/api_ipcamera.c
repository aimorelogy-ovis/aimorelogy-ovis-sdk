#include "ovis_manager.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_TASKS 16

enum task_state { TASK_EMPTY, TASK_RUNNING, TASK_SUCCEEDED, TASK_FAILED };

struct service_task {
	unsigned long id;
	enum service_action action;
	enum task_state state;
	int result;
	char output[512];
};

static struct service_task tasks[MAX_TASKS];
static unsigned long next_task_id = 1;
static pthread_mutex_t tasks_lock = PTHREAD_MUTEX_INITIALIZER;

static void *task_worker(void *arg)
{
	struct service_task *task = arg;
	enum service_action action = task->action;
	char output[512];
	int rc;
	char result[32];

	rc = service_run_action(action, output, sizeof(output));
	pthread_mutex_lock(&tasks_lock);
	task->result = rc;
	snprintf(task->output, sizeof(task->output), "%s", output);
	task->state = rc == 0 ? TASK_SUCCEEDED : TASK_FAILED;
	pthread_mutex_unlock(&tasks_lock);
	snprintf(result, sizeof(result), "%d", rc);
	audit_log(action == SERVICE_START ? "ipcamera.start" :
		action == SERVICE_STOP ? "ipcamera.stop" : "ipcamera.restart", result);
	return NULL;
}

unsigned long task_submit(enum service_action action)
{
	struct service_task *task;
	pthread_t thread;
	unsigned long id;
	size_t index;

	pthread_mutex_lock(&tasks_lock);
	id = next_task_id++;
	task = NULL;
	for (index = 0; index < MAX_TASKS; index++) {
		if (tasks[index].state != TASK_RUNNING) {
			task = &tasks[index];
			break;
		}
	}
	if (task == NULL) {
		pthread_mutex_unlock(&tasks_lock);
		return 0;
	}
	memset(task, 0, sizeof(*task));
	task->id = id;
	task->action = action;
	task->state = TASK_RUNNING;
	if (pthread_create(&thread, NULL, task_worker, task) != 0) {
		task->state = TASK_FAILED;
		strcpy(task->output, "unable to create task");
		pthread_mutex_unlock(&tasks_lock);
		return 0;
	}
	pthread_detach(thread);
	pthread_mutex_unlock(&tasks_lock);
	return id;
}

int task_get(unsigned long id, char *json, size_t size)
{
	struct service_task snapshot;
	struct service_task *task = NULL;
	const char *state;
	char escaped[1024];
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
	state = snapshot.state == TASK_RUNNING ? "running" :
		snapshot.state == TASK_SUCCEEDED ? "succeeded" : "failed";
	json_escape(snapshot.output, escaped, sizeof(escaped));
	snprintf(json, size, "{\"id\":%lu,\"state\":\"%s\",\"result\":%d,\"detail\":\"%s\"}",
		id, state, snapshot.result, escaped);
	return 0;
}
