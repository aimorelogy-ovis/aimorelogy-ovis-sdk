
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

static void signal_handler(int signo)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		g_main_loop_run = 0;
		break;
	default:
		break;
	}
}

static void register_signal_handler(void)
{
	struct sigaction sigaInst;

	sigaInst.sa_flags = 0;
	sigemptyset(&sigaInst.sa_mask);
	sigaddset(&sigaInst.sa_mask, SIGINT);
	sigaddset(&sigaInst.sa_mask, SIGTERM);

	sigaInst.sa_handler = signal_handler;
	sigaction(SIGINT, &sigaInst, NULL);
	sigaction(SIGTERM, &sigaInst, NULL);
}

#define JSONRPC_PORT (8888)
#define DEFAULT_VI_PIPE (0)

static void print_usage(const char *program_name)
{
	printf("Usage: %s [-i pipe] [-p port] [-h]\n", program_name);
	printf("  -i pipe        ViPipe (default: %d)\n", DEFAULT_VI_PIPE);
	printf("  -p port        JSONRPC port number (default: %d)\n", JSONRPC_PORT);
	printf("  -h             Show this help message\n");
}

int main(int argc, char **argv)
{
	int pipe = DEFAULT_VI_PIPE;
	int port = JSONRPC_PORT;
	int opt;

	while ((opt = getopt(argc, argv, "i:p:h")) != -1) {
		switch (opt) {
		case 'i':
			pipe = atoi(optarg);
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

	register_signal_handler();
	CVI_ISP_Client_Init(pipe);
	isp_daemon2_init(port);

	g_main_loop_run = 1;
	while (g_main_loop_run) {
		sleep(1);
	}

	isp_daemon2_uninit();
	CVI_ISP_Client_Exit(pipe);
	return 0;
}
