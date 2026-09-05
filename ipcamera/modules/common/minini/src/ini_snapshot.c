#include "ini_snapshot.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define INI_SNAPSHOT_MAX_SIZE (1024 * 1024)

static __thread struct {
    char *path;
    char *data;
    size_t size;
    struct stat identity;
} snapshot;

void ini_snapshot_end(void)
{
    free(snapshot.path);
    free(snapshot.data);
    memset(&snapshot, 0, sizeof(snapshot));
}

static int same_file(const struct stat *a, const struct stat *b)
{
    return a->st_dev == b->st_dev && a->st_ino == b->st_ino &&
        a->st_size == b->st_size &&
        a->st_mtim.tv_sec == b->st_mtim.tv_sec &&
        a->st_mtim.tv_nsec == b->st_mtim.tv_nsec &&
        a->st_ctim.tv_sec == b->st_ctim.tv_sec &&
        a->st_ctim.tv_nsec == b->st_ctim.tv_nsec;
}

int ini_snapshot_begin(const char *path)
{
    struct stat before, after;
    FILE *file;
    size_t count;
    int failed;

    if (stat(path, &before) != 0 || !S_ISREG(before.st_mode) ||
        before.st_size <= 0 || before.st_size > INI_SNAPSHOT_MAX_SIZE) {
        ini_snapshot_end();
        return -1;
    }
    if (snapshot.path != NULL && strcmp(path, snapshot.path) == 0 &&
        same_file(&before, &snapshot.identity)) {
        return 0;
    }
    ini_snapshot_end();
    file = fopen(path, "rb");
    if (file == NULL)
        return -1;
    snapshot.data = malloc((size_t)before.st_size);
    snapshot.path = strdup(path);
    if (snapshot.data == NULL || snapshot.path == NULL) {
        fclose(file);
        ini_snapshot_end();
        return -1;
    }
    count = fread(snapshot.data, 1, (size_t)before.st_size, file);
    failed = ferror(file) || fstat(fileno(file), &after) != 0;
    fclose(file);
    if (failed || count != (size_t)before.st_size || !same_file(&before, &after)) {
        ini_snapshot_end();
        return -1;
    }
    snapshot.identity = after;
    snapshot.size = count;
    return 0;
}

FILE *ini_snapshot_open(const char *path)
{
    if (snapshot.path != NULL && strcmp(path, snapshot.path) == 0)
        return fmemopen(snapshot.data, snapshot.size, "r");
    return fopen(path, "rb");
}
