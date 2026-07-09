#include <osal_test.h>

#define TEST_NUMS 4

static int test_proc_open(struct osal_proc_dir_entry *entry)
{
	osal_test_log("Proc entry '%s' opened.\n", entry->name);
	return 0;
}

static int test_proc_read(struct osal_proc_dir_entry *entry)
{
	osal_test_log("Reading from proc entry '%s'.\n", entry->name);
	return 0;
}

static int test_proc_write(struct osal_proc_dir_entry *entry, const char *buf, int count, long long *pos)
{
	osal_test_log("Writing to proc entry '%s': %s\n", entry->name, buf);
	return count;
}

static int test_cmd_handler(unsigned int argc, char (*argv)[PROC_CMD_SINGEL_LENGTH_MAX], void *private_data)
{
	osal_test_log("Command handler called with %u arguments.\n", argc);
	for (unsigned int i = 0; i < argc; i++)
	{
		osal_test_log("Arg[%u]: %s\n", i, argv[i]);
	}
	return 0;
}

void osal_proc_test(void)
{
	int total_tests = TEST_NUMS;  // init, create parent, create child, command test
	int failed_index = 0;
	const char *failed_tests[TEST_NUMS] = {0};
	osal_proc_entry *parent_entry;	// declare parent entry
	osal_proc_entry *child_entry;	// declare child entry
	osal_proc_cmd cmd_list[1];		// declare command list
	char argv[2][PROC_CMD_SINGEL_LENGTH_MAX]; // declare argv array

	osal_test_log("Starting osal_proc test...\n");

	// test init
	osal_proc_init("root");

	// create parent entry test
	parent_entry = osal_create_proc_entry("parent", NULL);
	if (!parent_entry) {
		failed_tests[failed_index++] = "parent proc entry creation";
	} else {
		parent_entry->open = test_proc_open;
		parent_entry->read = test_proc_read;
		parent_entry->write = test_proc_write;
		// create child entry test
		child_entry = osal_create_proc_entry("child", parent_entry);
		if (!child_entry) {
			failed_tests[failed_index++] = "child proc entry creation";
		} else {
			child_entry->open = test_proc_open;
			child_entry->read = test_proc_read;
			child_entry->write = test_proc_write;

			// command test
			snprintf(cmd_list[0].name, OSAL_PROC_NAME_LENGTH, "test_cmd");
			cmd_list[0].handler = test_cmd_handler;

			child_entry->cmd_list = cmd_list;
			child_entry->cmd_cnt = 1;

			// initialize argv array
			snprintf(argv[0], PROC_CMD_SINGEL_LENGTH_MAX, "arg1");
			snprintf(argv[1], PROC_CMD_SINGEL_LENGTH_MAX, "arg2");
			if (cmd_list[0].handler(2, argv, NULL) != 0) {
				failed_tests[failed_index++] = "command handler test";
			}

			// clean up child entry
			osal_remove_proc_entry("child", parent_entry);
		}

		// clean up parent entry
		osal_remove_proc_entry("parent", NULL);
	}

	// clean up init
	osal_proc_exit("root");

	// output test results
	if (failed_index == 0) {
		osal_test_log("[success] All proc operation tests(init, create, read, write, cmd) passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}

	osal_test_log("osal_proc test completed.\n");
}
OSAL_CMD_EXPORT(osal_proc_test, Test proc operations);