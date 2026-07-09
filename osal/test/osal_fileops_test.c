#include <osal_test.h>

#define BUF_LEN 80
#define FTRUNCATE_LEN 10

void osal_fileops_test(void)
{
    #ifndef __KERNEL__
    const char* filename = "/text.txt";
    #else
    const char* filename = "/tmp/text.txt";
    #endif
    void* file = NULL;
    void* file2 = NULL;
    int flags = OSAL_O_WRONLY | OSAL_O_CREAT;
    int mode = 0644;
    char buf[BUF_LEN] = "RT-Thread Programmer!";

    file = osal_klib_fopen(filename, flags, mode);
    if (file == NULL) {
        osal_test_log("[failed] osal_klib_fopen\n");
    }else{
        osal_test_log("[success] osal_klib_fopen\n");
    }
    if (osal_klib_fwrite(buf, sizeof(buf), file) < 0) {
        osal_test_log("[failed] osal_klib_fwrite\n");
    }else{
        osal_test_log("[success] osal_klib_fwrite\n");
    }

    osal_klib_fsync(file);
    osal_test_log("[success] osal_klib_fsync\n");

    osal_klib_fseek(0, SEEK_SET, file);
    osal_test_log("[success] osal_klib_fseek\n");

    osal_klib_fclose(file);
    osal_test_log("[success] osal_klib_fclose\n");
    file = NULL;
    flags = OSAL_O_RDWR;
    mode = 0;
    file2 = osal_klib_fopen(filename, flags, mode);

    memset(buf , 0, sizeof(char) * BUF_LEN);
    if (osal_klib_fread(buf, sizeof(buf), file2) < 0){
        osal_test_log("[failed] osal_klib_fread\n");
    }else {
        osal_test_log("[success] osal_klib_fread\n");
    }
    osal_test_log("Read from file test.txt : %s \n", buf);

    if (osal_klib_ftruncate(file2, FTRUNCATE_LEN) == OSAL_SUCCESS) {
        osal_test_log("[success] osal_klib_ftruncate\n");
    }else{
        osal_test_log("[failed] osal_klib_ftruncate\n");
    }

    osal_klib_fseek(0, SEEK_SET, file2);

    memset(buf , 0, sizeof(char) * BUF_LEN);
    if (osal_klib_fread(buf, sizeof(buf), file2) < 0){
        osal_test_log("[failed] osal_klib_fread\n");
    }else {
        osal_test_log("[success] osal_klib_fread\n");
    }
    buf[FTRUNCATE_LEN] = '\0';
    osal_test_log("Read from file test.txt : %s \n", buf);
    osal_klib_fclose(file2);

    if (osal_klib_unlink(filename) == OSAL_SUCCESS) {
        osal_test_log("[success] osal_klib_unlink\n");
    }else{
        osal_test_log("[failed] osal_klib_unlink\n");
    }

}
OSAL_CMD_EXPORT(osal_fileops_test, Test fileops operations);
