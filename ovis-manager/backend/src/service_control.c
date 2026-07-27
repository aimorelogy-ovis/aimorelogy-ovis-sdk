#include "ovis_manager.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
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

static int run_script_action(const char *script, const char *name,
	char *output, size_t size)
{
	int pipefd[2];
	int status;
	pid_t pid;
	ssize_t total = 0;
	ssize_t count;
	char buffer[512];

	if (script == NULL || name == NULL || output == NULL || size == 0)
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
		execl(script, script, name, (char *)NULL);
		_exit(127);
	}
	close(pipefd[1]);
	if (pid < 0) {
		close(pipefd[0]);
		return -1;
	}
	while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
		size_t available = size - 1 - (size_t)total;
		size_t copied = (size_t)count < available ? (size_t)count : available;

		if (copied > 0) {
			memcpy(output + total, buffer, copied);
			total += (ssize_t)copied;
		}
	}
	output[total] = '\0';
	close(pipefd[0]);
	if (waitpid(pid, &status, 0) < 0)
		return -1;
	if (!WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
}

int service_run_action(enum service_action action, char *output, size_t size)
{
	return run_script_action(OVIS_SERVICE_SCRIPT, action_name(action), output, size);
}

int usb_schedule_output_reboot(char *output, size_t size)
{
	return run_script_action(OVIS_USB_SERVICE_SCRIPT, "output-reboot", output, size);
}

int service_reload_overlay(char *output, size_t size)
{
	char pid_text[32];
	char *end;
	long parsed;
	FILE *file;

	if (output == NULL || size == 0)
		return -1;
	file = fopen(OVIS_PID_FILE, "r");
	if (file == NULL || fgets(pid_text, sizeof(pid_text), file) == NULL) {
		if (file != NULL)
			fclose(file);
		snprintf(output, size, "ipcamera PID unavailable");
		return -1;
	}
	fclose(file);
	errno = 0;
	parsed = strtol(pid_text, &end, 10);
	while (*end == '\r' || *end == '\n')
		end++;
	if (errno != 0 || parsed <= 1 || *end != '\0' ||
	    kill((pid_t)parsed, 0) != 0 || kill((pid_t)parsed, SIGUSR2) != 0) {
		snprintf(output, size, "ipcamera overlay reload failed");
		return -1;
	}
	snprintf(output, size, "ipcamera overlay reload requested");
	return 0;
}

int service_get_status(char *json, size_t size)
{
	char output[512];
	char escaped[1024];
	char pid_text[32] = "";
	FILE *file;
	int rc;
	int rtsp_enabled = 0;
	int uvc_enabled = 0;
	int gadget_present;

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
	config_get_output_flags(&rtsp_enabled, &uvc_enabled);
	gadget_present = access("/dev/video0", F_OK) == 0;
	snprintf(json, size, "{\"running\":%s,\"pid\":%s,\"detail\":\"%s\","
		"\"outputs\":{\"rtsp\":{\"enabled\":%s,\"running\":%s},"
		"\"uvc\":{\"enabled\":%s,\"gadget_present\":%s}}}",
		rc == 0 ? "true" : "false", pid_text[0] ? pid_text : "null", escaped,
		rtsp_enabled ? "true" : "false",
		(rc == 0 && rtsp_enabled) ? "true" : "false",
		uvc_enabled ? "true" : "false", gadget_present ? "true" : "false");
	return 0;
}
