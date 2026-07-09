/*
 * Template Test Tool
 *
 * This is a test tool template demonstrating basic program structure.
 * Modify this according to your needs when creating a new test tool.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

/* Program version */
#define PROG_VERSION "1.0.0"

/* Configuration structure */
typedef struct {
	char *input_file;
	char *output_file;
	int verbose;
	int loop_count;
} config_t;

/* Print usage information */
static void print_usage(const char *prog_name)
{
	printf("Usage: %s [OPTIONS]\n", prog_name);
	printf("\n");
	printf("Template test tool - modify this for your own test tool\n");
	printf("\n");
	printf("Options:\n");
	printf("  -i, --input <file>     Input file path\n");
	printf("  -o, --output <file>    Output file path\n");
	printf("  -l, --loop <count>     Loop count (default: 1)\n");
	printf("  -v, --verbose          Enable verbose output\n");
	printf("  -V, --version          Show version information\n");
	printf("  -h, --help             Show this help message\n");
	printf("\n");
	printf("Example:\n");
	printf("  %s -i input.bin -o output.bin -l 10 -v\n", prog_name);
	printf("\n");
}

/* Print version information */
static void print_version(void)
{
	printf("Template Test Tool v%s\n", PROG_VERSION);
	printf("Build Date: %s %s\n", __DATE__, __TIME__);
}

/* Initialize configuration */
static void init_config(config_t *cfg)
{
	memset(cfg, 0, sizeof(config_t));
	cfg->loop_count = 1;
	cfg->verbose = 0;
}

/* Cleanup configuration */
static void cleanup_config(config_t *cfg)
{
	if (cfg->input_file) {
		free(cfg->input_file);
		cfg->input_file = NULL;
	}
	if (cfg->output_file) {
		free(cfg->output_file);
		cfg->output_file = NULL;
	}
}

/* Parse command line arguments */
static int parse_args(int argc, char *argv[], config_t *cfg)
{
	int opt;
	static struct option long_options[] = {
		{"input",   required_argument, 0, 'i'},
		{"output",  required_argument, 0, 'o'},
		{"loop",    required_argument, 0, 'l'},
		{"verbose", no_argument,       0, 'v'},
		{"version", no_argument,       0, 'V'},
		{"help",    no_argument,       0, 'h'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "i:o:l:vVh", long_options, NULL)) != -1) {
		switch (opt) {
		case 'i':
			cfg->input_file = strdup(optarg);
			break;
		case 'o':
			cfg->output_file = strdup(optarg);
			break;
		case 'l':
			cfg->loop_count = atoi(optarg);
			if (cfg->loop_count <= 0) {
				fprintf(stderr, "Error: loop count must be positive\n");
				return -1;
			}
			break;
		case 'v':
			cfg->verbose = 1;
			break;
		case 'V':
			print_version();
			return 1;
		case 'h':
			print_usage(argv[0]);
			return 1;
		default:
			print_usage(argv[0]);
			return -1;
		}
	}

	return 0;
}

/* Main test execution logic */
static int run_test(config_t *cfg)
{
	int i;

	if (cfg->verbose) {
		printf("========================================\n");
		printf("Test Configuration:\n");
		printf("  Input file:  %s\n", cfg->input_file ? cfg->input_file : "(none)");
		printf("  Output file: %s\n", cfg->output_file ? cfg->output_file : "(none)");
		printf("  Loop count:  %d\n", cfg->loop_count);
		printf("========================================\n\n");
	}

	/* Main test loop */
	for (i = 0; i < cfg->loop_count; i++) {
		if (cfg->verbose) {
			printf("[%d/%d] Running test iteration...\n", i + 1, cfg->loop_count);
		}

		/*
		 * TODO: Add your test logic here
		 * For example:
		 * - Read input file
		 * - Execute algorithm/processing
		 * - Write output file
		 * - Validate results
		 */

		/* Simulate some processing */
		usleep(100000); /* 100ms */

		if (cfg->verbose) {
			printf("[%d/%d] Test iteration completed\n", i + 1, cfg->loop_count);
		}
	}

	printf("\nAll tests completed successfully!\n");
	printf("Total iterations: %d\n", cfg->loop_count);

	return 0;
}

/* Main function */
int main(int argc, char *argv[])
{
	config_t cfg;
	int ret;

	/* Initialize configuration */
	init_config(&cfg);

	/* Parse command line arguments */
	ret = parse_args(argc, argv, &cfg);
	if (ret != 0) {
		cleanup_config(&cfg);
		return (ret < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	/* Run test */
	ret = run_test(&cfg);

	/* Cleanup resources */
	cleanup_config(&cfg);

	return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
