#include <aos/kernel.h>
#include <aos/debug.h>
#include "osal.h"
#include "osal_errno.h"
#include "osal_inner.h"
#include "vfs.h"

void *osal_klib_fopen(const char *file, int flags, int mode)
{
    int *fd = aos_malloc(sizeof(int));
    if (fd == NULL) {
        osal_log("Failed to allocate memory for file object\n");
        return NULL;
    }

    int result = aos_open(file, flags | mode);
    if (result < 0) {
        osal_log("Failed to open file: %s, error code: %d\n", file, result);
        aos_free(fd);
        return NULL;
    }
    *fd = result;
    return (void *)fd;
}

void osal_klib_fclose(void *filp)
{
    int *fd = (int *)filp;
    if (fd == NULL) {
        osal_log("filp is null\n");
    }
    aos_close(*fd);
    aos_free(fd);
}

int osal_klib_fwrite(const char *buf, unsigned long size, void *filp)
{
    int *fd = (int *)filp;
    if (fd == NULL || buf == NULL) {
        osal_log("filp or buf is null\n");
        return -1;
    }

    int written_size = aos_write(*fd, buf, size);
    if (written_size < 0) {
        osal_log("Failed to write to file\n");
        return -1;
    }
    return written_size;
}

int osal_klib_fread(char *buf, unsigned long size, void *filp)
{
    int *fd = (int *)filp;
    if (fd == NULL || buf == NULL) {
        osal_log("filp or buf is null\n");
        return -1;
    }

    int read_size = aos_read(*fd, buf, size);
    if (read_size < 0) {
        osal_log("Failed to read from file\n");
        return -1;
    } else if (read_size == 0) {
        read_size = size;
    }

    return read_size;
}

int osal_klib_fseek(long long offset, int whence, void *filp)
{
    int *fd = (int *)filp;
    if (fd == NULL) {
        osal_log("filp is null\n");
        return -1;
    }

    int result = aos_lseek(*fd, offset, whence);
    if (result < 0)
    {
        osal_log("Failed to seek in file\n");
        return -1;
    }

    return result;
}

void osal_klib_fsync(void *filp)
{
    int *fd = (int *)filp;
    if (fd == NULL) {
        osal_log("filp is null\n");
        return;
    }

    aos_sync(*fd);
}

int osal_klib_unlink(const char *path)
{
    if (path == NULL) {
        osal_log("path is null\n");
        return -1;
    }

    return aos_unlink(path);
}
