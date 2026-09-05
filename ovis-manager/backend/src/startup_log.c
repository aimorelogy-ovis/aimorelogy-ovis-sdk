#include "ovis_manager.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STARTUP_LOG_LIMIT (256 * 1024)

static void capture_log(int input, const char *path)
{
	char previous[512];
	char buffer[4096];
	struct stat st;
	ssize_t count;
	size_t size = 0;
	FILE *file = NULL;

	if (snprintf(previous, sizeof(previous), "%s.1", path) >= (int)sizeof(previous))
		_exit(1);
	if (stat(path, &st) == 0)
		size = (size_t)st.st_size;
	if (stat(previous, &st) == 0 && st.st_size > STARTUP_LOG_LIMIT)
		unlink(previous);
	if (size > STARTUP_LOG_LIMIT) {
		file = fopen(path, "w");
		size = 0;
	}
	while ((count = read(input, buffer, sizeof(buffer))) != 0) {
		if (count < 0) {
			if (errno == EINTR) continue;
			break;
		}
		if (size + (size_t)count > STARTUP_LOG_LIMIT) {
			if (file != NULL) fclose(file);
			file = NULL;
			rename(path, previous);
			file = fopen(path, "w");
			size = 0;
		} else if (file == NULL) {
			file = fopen(path, "a");
		}
		/* Drain even on a logging error so the application cannot block on logs. */
		if (file != NULL) {
			fwrite(buffer, 1, (size_t)count, file);
			fflush(file);
		}
		size += (size_t)count;
	}
	if (file != NULL) fclose(file);
	close(input);
	_exit(0);
}

int startup_run_logged(const char *path, char **command)
{
	int descriptors[2];
	pid_t logger;

	if (pipe(descriptors) != 0)
		return 1;
	logger = fork();
	if (logger < 0) {
		close(descriptors[0]);
		close(descriptors[1]);
		return 1;
	}
	if (logger == 0) {
		close(descriptors[1]);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		capture_log(descriptors[0], path);
	}
	close(descriptors[0]);
	if (dup2(descriptors[1], STDOUT_FILENO) < 0 ||
	    dup2(descriptors[1], STDERR_FILENO) < 0) {
		close(descriptors[1]);
		return 1;
	}
	close(descriptors[1]);
	execvp(command[0], command);
	perror("exec logged service");
	return 1;
}
