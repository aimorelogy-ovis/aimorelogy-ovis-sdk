#include "ovis_manager.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *action_name(enum service_action action)
{
	switch (action) {
	case SERVICE_START: return "start";
	case SERVICE_STOP: return "stop";
	case SERVICE_RESTART: return "restart";
	case SERVICE_STATUS: return "status";
	default: return NULL;
	}
}

int service_run_action(enum service_action action, char *output, size_t size)
{
	const char *name = action_name(action);
	int pipefd[2];
	int status;
	pid_t pid;
	ssize_t total = 0;
	ssize_t count;

	if (name == NULL || output == NULL || size == 0)
		return -1;
	output[0] = '\0';
	if (pipe(pipefd) != 0)
		return -1;

	pid = fork();
	if (pid == 0) {
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[1]);
		execl(OVIS_SERVICE_SCRIPT, OVIS_SERVICE_SCRIPT, name, (char *)NULL);
		_exit(127);
	}
	close(pipefd[1]);
	if (pid < 0) {
		close(pipefd[0]);
		return -1;
	}
	while ((count = read(pipefd[0], output + total, size - 1 - (size_t)total)) > 0) {
		total += count;
		if ((size_t)total == size - 1)
			break;
	}
	output[total] = '\0';
	close(pipefd[0]);
	if (waitpid(pid, &status, 0) < 0)
		return -1;
	if (!WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
}

int service_get_status(char *json, size_t size)
{
	char output[512];
	char escaped[1024];
	char pid_text[32] = "";
	FILE *file;
	int rc;

	rc = service_run_action(SERVICE_STATUS, output, sizeof(output));
	file = fopen(OVIS_PID_FILE, "r");
	if (file != NULL) {
		if (fgets(pid_text, sizeof(pid_text), file) != NULL)
			pid_text[strcspn(pid_text, "\r\n")] = '\0';
		fclose(file);
	}
	if (pid_text[0] != '\0') {
		size_t i;
		for (i = 0; pid_text[i] != '\0'; i++) {
			if (pid_text[i] < '0' || pid_text[i] > '9') {
				pid_text[0] = '\0';
				break;
			}
		}
	}
	json_escape(output, escaped, sizeof(escaped));
	snprintf(json, size, "{\"running\":%s,\"pid\":%s,\"detail\":\"%s\"}",
		rc == 0 ? "true" : "false", pid_text[0] ? pid_text : "null", escaped);
	return 0;
}
