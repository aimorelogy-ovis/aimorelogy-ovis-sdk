#define __STDC_WANT_LIB_EXT2__ 1  //Define you want TR 24731-2:2010 extensions
#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <pthread.h>
#include <stdarg.h>
#include <byteswap.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/prctl.h>

#include "cvi_system.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* linux/include/linux/module.h has limit of 64 chars on module names */
#define MODULE_NAME_LEN 64
#define SWAP_BE32(x)    bswap_32(x)

#ifdef __UCLIBC__
extern int init_module(void *module_image, unsigned long len, const char *param_values);
extern int delete_module(const char *name, int flags);
#else
#include <sys/syscall.h>
#define init_module(mod, len, opts) syscall(__NR_init_module, mod, len, opts)
#define delete_module(mod, flags)   syscall(__NR_delete_module, mod, flags)
#endif

#define DOT_OR_DOTDOT(s) ((s)[0] == '.' && (!(s)[1] || ((s)[1] == '.' && !(s)[2])))

static const char *moderror(int err)
{
    switch (err) {
        case -1: /* btw: it's -EPERM */
            return "no such module";

        case ENOEXEC:
            return "invalid module format";

        case ENOENT:
            return "unknown symbol in module, or unknown parameter";

        case ESRCH:
            return "module has wrong symbol version";

        case ENOSYS:
            return "kernel does not support requested operation";
    }

    if (err < 0) { /* should always be */
        err = -err;
    }

    return strerror(err);
}

static char *filename2modname(const char *filename, char *modname)
{
    int i;
    const char *from;

    if (filename == NULL) {
        return NULL;
    }

    if (modname != NULL) {
        from = filename;

        for (i = 0; i < (MODULE_NAME_LEN - 1) && from[i] != '\0' && from[i] != '.'; i++) {
            modname[i] = (from[i] == '-') ? '_' : from[i];
        }

        modname[i] = '\0';
    }

    return modname;
}

static void *try_to_mmap_module(const char *filename, size_t *image_size_p)
{
    /* We have user reports of failure to load 3MB module
     * on a 16MB RAM machine. Apparently even a transient
     * memory spike to 6MB during module load
     * is too big for that system. */
    void *image;
    struct stat st;
    int fd;

    fd = open(filename, O_RDONLY, 0666);
    fstat(fd, &st);
    image = NULL;

    /* st.st_size is off_t, we can't just pass it to mmap */
    if ((size_t)st.st_size <= *image_size_p) {
        size_t image_size = st.st_size;
        image = mmap(NULL, image_size, PROT_READ, MAP_PRIVATE, fd, 0);

        if (image == MAP_FAILED) {
            image = NULL;
        } else if (*(uint32_t *)image != SWAP_BE32(0x7f454C46)) {
            /* No ELF signature. Compressed module? */
            munmap(image, image_size);
            image = NULL;
        } else {
            /* Success. Report the size */
            *image_size_p = image_size;
        }
    }

    close(fd);
    return image;
}

static int cvi_init_module(const char *filename, const char *options)
{
    size_t image_size;
    char *image;
    int rc;

    if (!options) {
        options = "";
    }

    image_size = INT_MAX - 4095;
    image = try_to_mmap_module(filename, &image_size);

    if (!image) {
        errno = ENOMEM;
        return -errno;
    }

    errno = 0;
    init_module(image, image_size, options);
    rc = errno;
    munmap(image, image_size);
    return rc;
}

static int cvi_delete_module(const char *module, unsigned int flags)
{
    errno = 0;
    delete_module(module, flags);
    return errno;
}

static char *local_basename(char *filename)
{
    char *p;

    if (filename == NULL || filename[0] == '\0') {
        /* We return a pointer to a static string containing ".". */
        p = (char *)".";
    } else {
        p = strrchr(filename, '/');

        if (p == NULL) {
            /* There is no slash in the filename.  Return the whole string. */
            p = filename;
        } else {
            if (p[1] == '\0') {
                /* We must remove trailing '/'. */
                while (p > filename && p[-1] == '/') {
                    --p;
                }

                /* Now we can be in two situations:
                   a) the string only contains '/' characters, so we return '/'
                   b) p points past the last component, but we have to remove the trailing slash.
                */
                if (p > filename) {
                    *p-- = '\0';
                    while (p > filename && p[-1] != '/') {
                        --p;
                    }
                } else {
                    /* The last slash we already found is the right position to return. */
                    while (p[1] != '\0') {
                        ++p;
                    }
                }
            } else {
                /* Go to the first character of the name. */
                ++p;
            }
        }
    }

    return p;
}
/*********************************************************************************/

int cvi_insmod(const char *pszPath, const char *pszOptions)
{
    int rc = 0;
    rc = cvi_init_module(pszPath, pszOptions);

    if (rc) {
        printf("can't insert '%s': %s\n", pszPath, moderror(rc));
    }

    return rc;
}

int cvi_rmmod(const char *pszPath)
{
    uint32_t flags = O_NONBLOCK | O_EXCL;
    char modname[MODULE_NAME_LEN];
    const char *bname = local_basename((char *)pszPath);

    if (NULL == filename2modname(bname, modname)) {
        return -1;
    }

    if (cvi_delete_module(modname, flags)) {
        printf("can't unload '%s': %s\n", modname, moderror(errno));
        return -1;
    }

    return 0;
}

int cvi_PathIsDirectory(const char *pszPath)
{
    if(!pszPath) {
        printf("pointer[pszPath] is NULL\n");
        return -1;
    }
    if (0 == strlen(pszPath)) {
        return -1;
    }
    struct stat stStatBuf;
    int s32Ret = stat(pszPath, &stStatBuf);

    if (0 == s32Ret) {
        if (stStatBuf.st_mode & S_IFDIR) {
            return 1;
        } else {
            return 0;
        }
    }

    return -1;
}

int cvi_rmdir(const char *pszPath)
{
    if(!pszPath) {
        printf("pointer[pszPath] is NULL\n");
        return -1;
    }
    struct dirent *pDirent = NULL;
    DIR *pDir = NULL;
    char aszSubDir[CVI_MAX_PATH_LEN + CVI_MAX_PATH_LEN + 1] = {
        0,
    };
    int s32Ret = 0;
    pDir = opendir(pszPath);

    if (!pDir) {
        printf("opendir[%s] failed\n", pszPath);
        return -1;
    }

    while ((pDirent = readdir(pDir)) != NULL) {
        if ((0 == strcmp(pDirent->d_name, ".")) || (0 == strcmp(pDirent->d_name, ".."))) {
            continue;
        }

        snprintf(aszSubDir, sizeof(aszSubDir), "%s/%s", pszPath, pDirent->d_name);
        s32Ret = cvi_PathIsDirectory(aszSubDir);

        if (1 == s32Ret) {
            s32Ret = cvi_rmdir(aszSubDir);

            if (0 != s32Ret) {
                printf("Delete SubDir[%s] failed\n", aszSubDir);
                closedir(pDir);
                return -1;
            }
        } else if (0 == s32Ret) {
            s32Ret = remove(aszSubDir);

            if (0 != s32Ret) {
                printf("Delete File[%s] failed\n", aszSubDir);
                closedir(pDir);
                return -1;
            }
        }
    }

    closedir(pDir);
    s32Ret = remove(pszPath);

    if (0 != s32Ret) {
        printf("remove[%s] failed\n", pszPath);
        return -1;
    }

    return 0;
}

int cvi_mkdir(const char *pszPath, mode_t mode)
{
    char DirName[CVI_MAX_PATH_LEN] = { 0 };
    char DirNameBak[CVI_MAX_PATH_LEN + 4] = { 0 };
    struct stat stStat;
    int s32Fd = -1;
    int s32Len = 0;
    int s32Idx = 0;
    int s32Ret = 0;

    /* Check Dir Path */
    if(!pszPath) {
        printf("pointer[pszPath] is NULL\n");
        return -1;
    }

    if ((s32Len = strlen(pszPath)) == 0) {
        printf("Empty Dir path\n");
        return -1;
    }

    /* Add tail '/' if not exist */
    if (pszPath[s32Len - 1] != '/') {
        snprintf(DirName, sizeof(DirName), "%s/", pszPath);
    } else {
        snprintf(DirName, sizeof(DirName), "%s", pszPath);
    }

    s32Len = strlen(DirName);

    /* Create Directory */
    for (s32Idx = 1; s32Idx < s32Len; s32Idx++) {
        if (DirName[s32Idx] == '/') {
            DirName[s32Idx] = 0; /* Split DirName temporary */
            /* open & fsat way to avoid the TOCTOU coverity problem */
            s32Fd = open(DirName, O_RDONLY);

            if (0 > s32Fd) {
                if (ENOENT == errno) { /* dir not exist */
                    if (mkdir(DirName, mode) < 0) {
                        printf("!!!Failed to mkdir %s\n", DirName);
                        return -1;
                    }
                }
            } else {
                s32Ret = fstat(s32Fd, &stStat);
                close(s32Fd);

                if ((0 == s32Ret) && (!S_ISDIR(stStat.st_mode))) {
                    /* it is not a dir,Rename it */
                    snprintf(DirNameBak, sizeof(DirNameBak), "%s.BAK", DirName);
                    s32Ret = rename(DirName, DirNameBak);

                    if (0 != s32Ret) {
                        printf("!!!Rename FIle %s to %s failed\n", DirName, DirNameBak);
                    }

                    /* create dir */
                    if (mkdir(DirName, mode) < 0) {
                        printf("!!!Failed to mkdir %s\n", DirName);
                        return -1;
                    }
                }
            }

            DirName[s32Idx] = '/';
        }
    }

    return 0;
}

int cvi_system(const char *pszCmd)
{
    pid_t pid;
    int status;
    if (pszCmd == NULL) {
        return (1); /**< if cmdstring is NULL return no zero */
    }
    if ((pid = vfork()) < 0) { /**<vfork,child pid share resource with parrent,not copy */
        status = -1;           /**<vfork fail */
    } else if (pid == 0) {
        execl("/bin/sh", "sh", "-c", pszCmd, (char *)0);
        _exit(127); /**< return 127 only exec fail;the chid procee is not exist normore if exec success */
    } else {        /** parrent pid */
        while (waitpid(pid, &status, 0) < 0) {
            if (errno != EINTR) {
                status = -1; /**< return -1 when interrupted by signal except EINTR */
                break;
            }
        }
    }
    return status; /**< return the state of child progress if waitpid success */
}

int cvi_usleep(uint32_t usec)
{
    int ret;
    struct timespec requst;
    struct timespec remain;
    remain.tv_sec = usec / 1000000;
    remain.tv_nsec = (usec % 1000000) * 1000;
    do {
        requst = remain;
        ret = nanosleep(&requst, &remain);
    } while (-1 == ret && errno == EINTR);
    return ret;
}

struct globals {
    int slink_depth;
    int du_depth;
    dev_t dir_dev;
} FIX_ALIASING;

static struct globals G;

static char *last_char_is(const char *s, int c)
{
    if (s && *s) {
        size_t sz = strlen(s) - 1;
        s += sz;

        if ((unsigned char)*s == c) {
            return (char *)s;
        }
    }

    return NULL;
}

static char *xasprintf(const char *format, ...)
{
    va_list p;
    char *string_ptr;
    va_start(p, format);
    vasprintf(&string_ptr, format, p);
    va_end(p);
    return string_ptr;
}

static char *concat_path_file(const char *path, const char *filename)
{
    char *lc;

    if (!path) {
        path = "";
    }

    lc = last_char_is(path, '/');

    while (*filename == '/') {
        filename++;
    }

    return xasprintf("%s%s%s", path, (lc == NULL ? "/" : ""), filename);
}

static char *concat_subpath_file(const char *path, const char *f)
{
    if (f && DOT_OR_DOTDOT(f)) {
        return NULL;
    }

    return concat_path_file(path, f);
}

static unsigned long long du(const char *filename)
{
    G.slink_depth = INT_MAX;
    struct stat statbuf;
    unsigned long long sum;

    if (lstat(filename, &statbuf) != 0) {
        return 0;
    }

    if (G.du_depth == 0) {
        G.dir_dev = statbuf.st_dev;
    } else if (G.dir_dev != statbuf.st_dev) {
        return 0;
    }

    sum = statbuf.st_blocks;

    if (S_ISLNK(statbuf.st_mode)) {
        if (G.slink_depth > G.du_depth) {
            if (stat(filename, &statbuf) != 0) {
                return 0;
            }

            sum = statbuf.st_blocks;

            if (G.slink_depth == 1) {
                G.slink_depth = INT_MAX;
            }
        }
    }

    if (S_ISDIR(statbuf.st_mode)) {
        DIR *dir;
        struct dirent *entry;
        char *newfile;
        dir = opendir(filename);

        if (!dir) {
            return sum;
        }

        while ((entry = readdir(dir))) {
            newfile = concat_subpath_file(filename, entry->d_name);

            if (newfile == NULL) {
                continue;
            }

            ++G.du_depth;
            sum += du(newfile);
            --G.du_depth;
            free(newfile);
        }

        closedir(dir);
    } else {
        if (G.du_depth != 0) {
            return sum;
        }
    }

    return sum;
}
/*********************************************************************************/

int cvi_du(const char *pszPath, uint64_t *pu64Size_KB)
{
    if(!pu64Size_KB) {
        printf("pointer[pu64Size_KB] is NULL\n");
        return -1;
    }
    if(!pszPath) {
        printf("pointer[pszPath] is NULL\n");
        return -1;
    }
    int s32Ret = 0;
    *pu64Size_KB = 0;
    s32Ret = cvi_PathIsDirectory(pszPath);

    if (-1 == s32Ret) {
        return -1;
    } else {
        uint64_t u64Totalblocks = 0;
        u64Totalblocks = du(pszPath);
        if (0 < u64Totalblocks) {
            u64Totalblocks++;
            u64Totalblocks >>= 1;
        }
        *pu64Size_KB = u64Totalblocks;
        return 0;
    }
}

static pthread_t s_UpdateTid = (pthread_t)-1;
static sem_t s_SyncSem;
static bool s_SyscThreadStart = false;

static void *Update_Thread(void *pvargv)
{
    pthread_detach(pthread_self());
    (void)pvargv;
    prctl(PR_SET_NAME, __func__, 0, 0, 0);
    while (1) {
        printf("before sync.");
        sync();
        printf("after sync finished.");
        if (0 != sem_wait(&s_SyncSem)) {
            break;
        }
    }
    sem_destroy(&s_SyncSem);
    s_UpdateTid = (pthread_t)-1;
    s_SyscThreadStart = false;
    return NULL;
}

int cvi_async(void)
{
    if (false == s_SyscThreadStart) {
        sem_init(&s_SyncSem, 0, 0);
        if (0 != pthread_create(&s_UpdateTid, NULL, Update_Thread, NULL)) {
            printf("create Update_Thread fail:%s\n", strerror(errno));
            return -1;
        }
        s_SyscThreadStart = true;
    } else {
        sem_post(&s_SyncSem);
    }
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
