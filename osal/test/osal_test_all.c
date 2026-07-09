#include <osal_test.h>

// define test case structure
typedef struct {
    const char *name;
    void (*test_fn)(void);
} osal_test_case_t;

// test case array
static const osal_test_case_t test_cases[] = {
    {"addr", osal_addr_test},
    {"atomic", osal_atomic_test},
    {"atomic_multi", osal_atomic_multi_test},
    {"bitmap", osal_bitmap_test},
    {"completion", osal_completion_test},
    {"completion_multi", osal_completion_multi_test},
    {"delaywork", osal_delaywork_test},
    {"hash", osal_hash_test},
    {"interrupt", osal_interrupt_test},
    {"math", osal_math_test},
    {"mutex", osal_mutex_test},
    {"proc", osal_proc_test},
    {"rwlock", osal_rwlock_test},
    {"rwlock_multi", osal_rwlock_multi_test},
    {"semaphore", osal_semaphore_test},
    {"spinlock", osal_spinlock_test},
    {"string", osal_string_test},
    {"task", osal_task_test},
    {"timer", osal_timer_test},
    {"wait", osal_wait_test},
    {"workqueue", osal_workqueue_test},
    {"cache", osal_cache_test},
    {"fileops", osal_fileops_test}
};

#ifndef __KERNEL__
// RT-Thread os code
void osal_test_all(void)
{
    osal_test_log("Starting OSAL All Tests...\n");

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        osal_test_log("Running %s Tests...\n", test_cases[i].name);
        test_cases[i].test_fn();
    }

    osal_test_log("All Tests Completed.\n");
}

OSAL_CMD_EXPORT(osal_test_all, Test all operations);

#elif defined(__ALIOS__)


#else
// Linux os code

#include <linux/kmod.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/delay.h>

// find and execute test function by name
static void run_test_by_name(const char *test_name)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(test_cases); i++) {
        if (strncmp(test_name, test_cases[i].name, strlen(test_cases[i].name)) == 0) {
            test_cases[i].test_fn();
            osal_test_log("Finish test: %s\n", test_cases[i].name);
            return;
        }
    }
    osal_test_log("Test case '%s' not found.\n", test_name);
}

// run all tests and generate test report
static void run_all_tests(void)
{
    size_t i;
    char cmd[384];
    char start_pattern[128];
    char end_pattern[128];
    int ret;
    char *envp[] = { "HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
    char *final_argv[4];
    char *argv[4];
    char *init_argv[] = { "/bin/sh", "-c", "echo \"OSAL Test Report\" > /tmp/osal_test_report.txt", NULL };

    ret = call_usermodehelper(init_argv[0], init_argv, envp, UMH_WAIT_EXEC);
    if (ret) {
        osal_test_log("Failed to initialize report file, error: %d. Aborting tests.\n", ret);
        return;
    }

    osal_test_log("==== OSAL TEST BATCH START ====\n");

    for (i = 0; i < ARRAY_SIZE(test_cases); i++) {
        const char *test_name = test_cases[i].name;

        osal_test_log("Running %s test...\n", test_name);
        test_cases[i].test_fn();
        osal_test_log("Finish test: %s\n", test_name);

        msleep(10);

        snprintf(start_pattern, sizeof(start_pattern), "/Running %s test\\.\\.\\./", test_name);
        snprintf(end_pattern, sizeof(end_pattern), "/Finish test: %s/", test_name);

        snprintf(cmd, sizeof(cmd), "dmesg | sed -n '%s,%sp' >> /tmp/osal_test_report.txt",
                 start_pattern, end_pattern);

        argv[0] = "/bin/sh";
        argv[1] = "-c";
        argv[2] = cmd; /* Point to the dynamically generated command string */
        argv[3] = NULL;

        ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
        if (ret) {
            osal_test_log("Failed to append report for %s, error: %d\n", test_name, ret);
        }
    }

    osal_test_log("==== OSAL TEST BATCH END ====\n");

    final_argv[0] = "/bin/sh";
    final_argv[1] = "-c";
    final_argv[2] = "echo \"\n==== OSAL Test Report End ====\" >> /tmp/osal_test_report.txt";
    final_argv[3] = NULL;
    msleep(100);
    call_usermodehelper(final_argv[0], final_argv, envp, UMH_WAIT_EXEC);
    osal_test_log("Finished all tests. Report generated: /tmp/osal_test_report.txt\n");
}

// Linux os code

// proc file write operation function declaration
static ssize_t osal_test_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos);

// proc file operation structure
static const struct proc_ops osal_test_fops = {
    .proc_write = osal_test_write,
};

#define PROC_NAME "osal_test"

// handle proc file write operation
static ssize_t osal_test_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos) {
    char input[64];

    if (count > sizeof(input) - 1)
        count = sizeof(input) - 1;

    if (copy_from_user(input, buffer, count))
        return -EFAULT;

    input[count] = '\0'; // ensure string ends with null

    printk(KERN_INFO "Input received: %s\n", input);

    // if input "test_all", run all test cases and generate test report
    if (strncmp(input, "test_all", strlen("test_all")) == 0) {
        run_all_tests();
        return count;
    }

    // otherwise, try to run a single test case
    run_test_by_name(input);
    return count;
}

// module initialization function
static int __init osal_test_init(void) {
    struct proc_dir_entry *entry = proc_create(PROC_NAME, 0666, NULL, &osal_test_fops);

    osal_test_log("OSAL test module loaded.\n");

    if (!entry) {
        osal_test_log("Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }
    return 0;
}

// module exit function
static void __exit osal_test_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    osal_test_log("OSAL test module unloaded.\n");
}

module_init(osal_test_init);
module_exit(osal_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OSAL_TEST");
#endif
