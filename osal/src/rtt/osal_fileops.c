#include <dfs_file.h>
#include <dfs_posix.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "osal.h"
#include "osal_inner.h"

void *osal_klib_fopen(const char *file, int flags, int mode)
{
    int *fd = rt_malloc(sizeof(int));
    if (!fd) {
        osal_log("Failed to allocate memory for file object\n");
        return NULL;
    }

    int rt_flags = 0;

    // Parse access mode (must and can only select one)
    if (flags & OSAL_O_RDWR) {
        rt_flags |= O_RDWR;
    } else if (flags & OSAL_O_WRONLY) {
        rt_flags |= O_WRONLY;
    } else { // Default to read-only
        rt_flags |= O_RDONLY;
    }

    // Parse other flags
    if (flags & OSAL_O_CREAT) rt_flags |= O_CREAT;
    if (flags & OSAL_O_EXCL)  rt_flags |= O_EXCL;
    if (flags & OSAL_O_TRUNC) rt_flags |= O_TRUNC;
    if (flags & OSAL_O_APPEND) rt_flags |= O_APPEND;

    // Call open
    int result;
    if (rt_flags & O_CREAT) {
        result = open(file, rt_flags, mode); // Pass permission parameter
    } else {
        result = open(file, rt_flags);
    }

    if (result < 0) {
        osal_log("Failed to open file: %s, errno=%d\n", file, errno);
        rt_free(fd);
        return NULL;
    }

    *fd = result;
    return (void *)fd;
}
// EXPORT_SYMBOL(osal_klib_fopen);

void osal_klib_fclose(void *filp)
{
	int *fd = (int *)filp;
	if (fd == NULL) {
		osal_log("filp is null\n");
		return;
	}
	close(*fd);
	rt_free(fd);
}
// EXPORT_SYMBOL(osal_klib_fclose);

int osal_klib_fwrite(const char *buf, unsigned long size, void *filp)
{
	int *fd = (int *)filp;

	if (fd == NULL || buf == NULL) {
		osal_log("filp or buf is null\n");
		return -1;
	}

	int written_size = write(*fd, buf, size);
	if (written_size < 0) {
		osal_log("Failed to write to file\n");
		return -1;
	}
	return written_size;
}
// EXPORT_SYMBOL(osal_klib_fwrite);

int osal_klib_fread(char *buf, unsigned long size, void *filp)
{
	int *fd = (int *)filp;

	if (fd == NULL || buf == NULL) {
		osal_log("filp is null\n");
		return OSAL_FAILURE;
	}

	int read_size = read(*fd, buf, size);
    rt_kprintf("read size = %d\n", read_size);
	if (read_size < 0) {
		osal_log("osal_klib_fread fail\n");
	}
	return read_size;
}
// EXPORT_SYMBOL(osal_klib_fread);

int osal_klib_fseek(long long offset, int whence, void *filp)
{
	int *fd = (int *)filp;
	if (fd == NULL) {
		osal_log("filp is null\n");
		return OSAL_FAILURE;
	}

	if (whence < 0) {
		osal_log("whence should not be smaller than 0\n");
	}

	int result = lseek(*fd, offset, whence);
	if (result < 0) {
		osal_log("osal_klib_fseek fail\n");
		return 1;
	}
	return result;
}
// EXPORT_SYMBOL(osal_klib_fseek);

void osal_klib_fsync(void *filp)
{
	int *fd = (int *)filp;
	if (fd == NULL) {
		osal_log("filp is null\n");
		return;
	}

	if (fsync(*fd) < 0) {
		osal_log("osal_klib_fsync fail\n");
	}
}
// EXPORT_SYMBOL(osal_klib_fsync);

int osal_klib_unlink(const char *path)
{
	if (path == NULL) {
		osal_log("path is null\n");
		return OSAL_FAILURE;
	}

	if (unlink(path) < 0) {
		osal_log("Failed to unlink file: %s\n", path);
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_klib_unlink);

int osal_klib_ftruncate(void *filp, unsigned long len)
{
	int *fd = (int *)filp;
	if (fd == NULL) {
		osal_log("filp is null\n");
		return OSAL_FAILURE;
	}

	if (ftruncate(*fd, len) < 0) {
		osal_log("Failed to ftruncate: %d\n", *fd);
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_klib_ftruncate);