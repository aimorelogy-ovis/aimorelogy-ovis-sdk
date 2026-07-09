#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_fs.h>

#define SD_TEST_FILE "/mnt/sd/test_file.bin"
#define TEST_BLOCK_SIZE 512
#define TEST_BLOCK_COUNT 10

static void sd_test_read_write(void)
{
    int fd = -1;
    rt_uint8_t *write_buf = RT_NULL;
    rt_uint8_t *read_buf = RT_NULL;
    rt_size_t written, readlen;
    int i, result = 0;

    write_buf = rt_malloc(TEST_BLOCK_SIZE * TEST_BLOCK_COUNT);
    read_buf = rt_malloc(TEST_BLOCK_SIZE * TEST_BLOCK_COUNT);

    rt_kprintf("\n--- Starting SD Card R/W Test ---\n");
    if (!write_buf || !read_buf) {
        rt_kprintf("[FAIL] Memory allocation failed!\n");
        goto exit;
    }

    for (i = 0; i < TEST_BLOCK_SIZE * TEST_BLOCK_COUNT; i++) {
        write_buf[i] = i & 0xFF;
    }

    fd = open(SD_TEST_FILE, O_WRONLY | O_CREAT);
    if (fd < 0) {
        rt_kprintf("[FAIL] Failed to open file for writing\n");
        result = -1;
        goto exit;
    }

    written = write(fd, write_buf, TEST_BLOCK_SIZE * TEST_BLOCK_COUNT);
    if (written != TEST_BLOCK_SIZE * TEST_BLOCK_COUNT) {
        rt_kprintf("[FAIL] Write failed! Written: %d, Expected: %d\n", 
              written, TEST_BLOCK_SIZE * TEST_BLOCK_COUNT);
        result = -1;
        close(fd);
        goto exit;
    }
    close(fd);

    fd = open(SD_TEST_FILE, O_RDONLY);
    if (fd < 0) {
        rt_kprintf("[FAIL] Failed to open file for reading\n");
        result = -1;
        goto exit;
    }

    readlen = read(fd, read_buf, TEST_BLOCK_SIZE * TEST_BLOCK_COUNT);
    if (readlen != TEST_BLOCK_SIZE * TEST_BLOCK_COUNT) {
        rt_kprintf("[FAIL] Read failed! Read: %d, Expected: %d\n", 
            readlen, TEST_BLOCK_SIZE * TEST_BLOCK_COUNT);
        result = -1;
        close(fd);
        goto exit;
    }
    close(fd);

    for (i = 0; i < TEST_BLOCK_SIZE * TEST_BLOCK_COUNT; i++) {
        if (read_buf[i] != write_buf[i]) {
            rt_kprintf("[FAIL] Data mismatch at position %d: %02X vs %02X\n", 
                  i, read_buf[i], write_buf[i]);
            result = -1;
            break;
        }
    }

    //delete test file
    unlink(SD_TEST_FILE);

exit:
    if (write_buf) rt_free(write_buf);
    if (read_buf) rt_free(read_buf);

    if (result == 0) {
        rt_kprintf("[PASS] SD card read/write test!\n");
    } else {
        rt_kprintf("[FAIL] SD card read/write test!\n");
    }
    rt_kprintf("\n--- END SD Card R/W Test ---\n");
}

static void sd_test_performance(void)
{
    int fd = -1;
    rt_uint8_t *buf = RT_NULL;
    rt_tick_t start, end;
    rt_size_t size = 1024 * 1024; // 1MB
    float time_sec, speed;
    char print_buf[128] = {0};
    
    rt_kprintf("\n--- Starting SD Card Performance Test ---\n");
    buf = rt_malloc(size);
    if (!buf) {
        rt_kprintf("[FAIL] Memory allocation failed!\n");
        return;
    }

    fd = open(SD_TEST_FILE, O_WRONLY | O_CREAT);
    if (fd < 0) {
        rt_kprintf("[FAIL] Failed to open file for writing\n");
        goto exit;
    }
    
    start = rt_tick_get();
    rt_size_t written = write(fd, buf, size);
    end = rt_tick_get();
    close(fd);
    
    if (written != size) {
        rt_kprintf("[FAIL] Write performance test failed\n");
        goto exit;
    }
    
    time_sec = (end - start) * 1.0 / RT_TICK_PER_SECOND;
    speed = size / (time_sec * 1024 * 1024); // MB/s

    snprintf(print_buf, sizeof(print_buf), "Write speed: %.2f MB/s", speed);
    rt_kprintf("%s\n", print_buf);

    fd = open(SD_TEST_FILE, O_RDONLY);
    if (fd < 0) {
        rt_kprintf("[FAIL] Failed to open file for reading\n");
        goto exit;
    }
    
    start = rt_tick_get();
    rt_size_t readlen = read(fd, buf, size);
    end = rt_tick_get();
    close(fd);
    
    if (readlen != size) {
        rt_kprintf("[FAIL] Read performance test failed\n");
        goto exit;
    }
    
    time_sec = (end - start) * 1.0 / RT_TICK_PER_SECOND;
    speed = size / (time_sec * 1024 * 1024); // MB/s
    memset(print_buf, 0, sizeof(print_buf));
    snprintf(print_buf, sizeof(print_buf), "Read speed: %.2f MB/s", speed);
    rt_kprintf("%s\n", print_buf);

exit:
    if (buf) rt_free(buf);
    unlink(SD_TEST_FILE);
    rt_kprintf("\n--- END SD Card Performance Test ---\n");
}

static void sd_card_test(int argc, char *argv[])
{
    // rt_kprintf("Unmounting SD card...\n");
    // if (dfs_unmount("/")) {
    //     rt_kprintf("Failed to unmount SD card!");
    //     return;
    // }
    // rt_kprintf("SD card Unmounted successfully");

    // rt_kprintf("Mounting SD card...\n");
    // if (dfs_mount("sd0", "/", "elm", 0, 0) != 0) {
    //     rt_kprintf("Failed to mount SD card!");
    //     return;
    // }
    
    // rt_kprintf("SD card mounted successfully");
    
    // read/write test
    sd_test_read_write();
    
    // performance test
    if (argc > 1 && rt_strcmp(argv[1], "perf") == 0) {
        sd_test_performance();
    }
}

MSH_CMD_EXPORT(sd_card_test, Test SD card functionality or performance);
