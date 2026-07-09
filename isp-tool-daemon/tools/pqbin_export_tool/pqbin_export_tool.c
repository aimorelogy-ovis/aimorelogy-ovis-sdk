
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <sys/types.h>

#include "cvi_isp.h"
#include "cvi_ispd2.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

static int g_main_loop_run = 1;

#define JSONRPC_PORT (5566)
#define DEFAULT_VI_PIPE (0)
#define MAX_VI_PIPES VI_MAX_PIPE_NUM
static int g_vi_pipes[MAX_VI_PIPES];
static int g_vi_pipe_count;

static void signal_handler(int signum)
{
	UNUSED(signum);
	printf("\nReceived signal, exiting gracefully...\n");
	g_main_loop_run = 0;
}

static void print_usage(const char *program_name)
{
	printf("Usage: %s [-i pipes] [-p port] [-h]\n", program_name);
	printf("  -i pipes       ViPipe list (comma-separated, e.g., 0,1,2,3 or 0,2,3; default: %d)\n", DEFAULT_VI_PIPE);
	printf("  -p port        JSONRPC port number (default: %d)\n", JSONRPC_PORT);
	printf("  -h             Show this help message\n");
}

int main(int argc, char **argv)
{
	int port = JSONRPC_PORT;
	int opt;
	int i;

	g_vi_pipes[0] = DEFAULT_VI_PIPE;
	g_vi_pipe_count = 1;

	while ((opt = getopt(argc, argv, "i:p:h")) != -1) {
		switch (opt) {
		case 'i':
			g_vi_pipe_count = 0;
			char *token = strtok(optarg, ",");

			while (token != NULL && g_vi_pipe_count < MAX_VI_PIPES) {
				g_vi_pipes[g_vi_pipe_count++] = atoi(token);
				token = strtok(NULL, ",");
			}
			if (g_vi_pipe_count == 0) {
				fprintf(stderr, "Error: Invalid pipe list\n");
				return 1;
			}
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'h':
			print_usage(argv[0]);
			return 0;
		default:
			print_usage(argv[0]);
			return 1;
		}
	}

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	for (i = 0; i < g_vi_pipe_count; i++) {
		printf("Initializing ViPipe %d...\n", g_vi_pipes[i]);
		CVI_ISP_MemInit(g_vi_pipes[i]);
	}

	isp_daemon2_init(port);

	g_main_loop_run = 1;
	while (g_main_loop_run) {
		sleep(1);
	}

	isp_daemon2_uninit();
	return 0;
}

CVI_S32 CVI_VI_QueryDevStatus(VI_PIPE ViPipe)
{
	for (int i = 0; i < g_vi_pipe_count; i++) {
		if (g_vi_pipes[i] == ViPipe) {
			return CVI_SUCCESS;
		}
	}
	return CVI_FAILURE;
}
