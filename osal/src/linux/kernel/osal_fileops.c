#include <linux/fs.h>
#include <linux/version.h>
#include <linux/namei.h>
#include "osal.h"
#include "osal_errno.h"
#include "osal_inner.h"

static struct file *klib_fopen(const char *file, int flags, int mode)
{
    struct file *filp = filp_open(file, flags, mode);
    if (IS_ERR(filp)) {
        long err_code = PTR_ERR(filp); // Get error code
        osal_log("Failed to open file %s: error %ld\n", file, err_code);
        return NULL; // Return NULL to indicate failure
    }

    // File successfully opened, perform subsequent operations
    osal_log("File %s opened successfully.\n", file);

    return filp; // Return successfully opened file pointer
}

static void klib_fclose(struct file *filp)
{
    if (filp != NULL) {
        filp_close(filp, NULL);
    }
    return;
}

static int klib_fwrite(const char *buf, unsigned long size, struct file *filp)
{
    int writelen;

    writelen = kernel_write(filp, buf, size, &filp->f_pos);
    return writelen;
}

static int klib_fread(char *buf, unsigned long size, struct file *filp)
{
    int readlen;
    readlen = kernel_read(filp, buf, size, &filp->f_pos);

    return readlen;
}

void *osal_klib_fopen(const char *file, int flags, int mode)
{
    if (file == NULL) {
        osal_log("file NULL!\n");
        return NULL;
    }

    return (void *)klib_fopen(file, flags, mode);
}
EXPORT_SYMBOL(osal_klib_fopen);

void osal_klib_fclose(void *filp)
{
    if (filp == NULL) {
        osal_log("filp NULL!\n");
        return;
    }

    klib_fclose((struct file *)filp);
}
EXPORT_SYMBOL(osal_klib_fclose);

int osal_klib_fwrite(const char *buf, unsigned long size, void *filp)
{
    if ((buf == NULL) || (filp == NULL)) {
        osal_log("buf&filp NULL!\n");
        return OSAL_FAILURE;
    }

    return klib_fwrite(buf, size, (struct file *)filp);
}
EXPORT_SYMBOL(osal_klib_fwrite);

int osal_klib_fread(char *buf, unsigned long size, void *filp)
{
    if ((buf == NULL) || (filp == NULL)) {
        osal_log("buf&filp NULL!\n");
        return OSAL_FAILURE;
    }

    return klib_fread(buf, size, (struct file *)filp);
}
EXPORT_SYMBOL(osal_klib_fread);

int osal_klib_fseek(long long offset, int whence, void *filp)
{
    int ret;
    loff_t res;

    if (filp == NULL) {
        osal_log("filp NULL!\n");
        return OSAL_FAILURE;
    }

    res = vfs_llseek(filp, offset, whence);
    ret = (int)res;
    if (res != (loff_t)ret) {
        ret = OSAL_EOVERFLOW;
    }

    return ret;
}
EXPORT_SYMBOL(osal_klib_fseek);

void osal_klib_fsync(void *filp)
{
    if (filp == NULL) {
        osal_log("filp NULL!\n");
        return;
    }

    vfs_fsync(filp, 0);
}
EXPORT_SYMBOL(osal_klib_fsync);

int osal_klib_unlink(const char *path)
{
    // In the Linux kernel, we need to use vfs_unlink instead of the user-space unlink
    struct path p;
    int error;

    if (path == NULL) {
        osal_log("path NULL!\n");
        return OSAL_FAILURE;
    }

    error = kern_path(path, 0, &p);
    if (error) {
        osal_log("Failed to unlink file: %s\n", path);
        return OSAL_FAILURE;
    }

    error = vfs_unlink(p.dentry->d_parent->d_inode, p.dentry, NULL);
    path_put(&p);

    return (error) ? OSAL_FAILURE : OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_klib_unlink);

int osal_klib_ftruncate(void *filp, unsigned long len)
{
    struct file *fp;
    int error;

    if (filp == NULL) {
        osal_log("filp NULL!\n");
        return OSAL_FAILURE;
    }

    fp = (struct file *)filp;
    // In the Linux kernel, use vfs_truncate and pass the length parameter
    error = vfs_truncate(&fp->f_path, len);

    return (error) ? OSAL_FAILURE : OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_klib_ftruncate);