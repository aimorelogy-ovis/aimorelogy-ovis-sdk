#include "ovis_manager.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	unsigned long value;
	char *end = NULL;
	unsigned short port = OVIS_DEFAULT_PORT;
	char error[256];

	setvbuf(stdout, NULL, _IOLBF, 0);
	if (argc >= 5 && strcmp(argv[1], "--run-logged") == 0 &&
	    strcmp(argv[3], "--") == 0)
		return startup_run_logged(argv[2], &argv[4]);

	if (argc == 2 && strcmp(argv[1], "--prepare-config") == 0) {
		if (config_ensure_runtime(error, sizeof(error)) == 0)
			return 0;
		fprintf(stderr, "runtime config unavailable: %s\n", error);
		return 1;
	}
	if (argc == 2 && strcmp(argv[1], "--usb-provision") == 0)
		return usb_provision_run();
	if (argc == 3 && strcmp(argv[1], "-p") == 0) {
		errno = 0;
		value = strtoul(argv[2], &end, 10);
		if (errno != 0 || end == argv[2] || *end != '\0' || value == 0 || value > 65535) {
			fprintf(stderr, "invalid port: %s\n", argv[2]);
			return 2;
		}
		port = (unsigned short)value;
	} else if (argc != 1) {
		fprintf(stderr, "Usage: %s [-p port | --prepare-config | --usb-provision | --run-logged file -- command ...]\n",
			argv[0]);
		return 2;
	}

	signal(SIGPIPE, SIG_IGN);
	if (config_ensure_runtime(error, sizeof(error)) != 0)
		fprintf(stderr, "runtime config unavailable: %s\n", error);

	return http_server_run(port);
}
