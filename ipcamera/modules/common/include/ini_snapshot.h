#ifndef INI_SNAPSHOT_H
#define INI_SNAPSHOT_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A thread-local, bounded read snapshot. begin refreshes changed files. */
int ini_snapshot_begin(const char *path);
FILE *ini_snapshot_open(const char *path);
void ini_snapshot_end(void);

#ifdef __cplusplus
}
#endif

#endif
