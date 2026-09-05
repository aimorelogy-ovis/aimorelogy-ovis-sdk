#include "minIni.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    char path[] = "/tmp/ovis-ini-snapshot-XXXXXX";
    char cached[128], direct[128];
    const char *sections[] = { "test", "TEST", "duplicate", "missing" };
    const char *keys[] = { "quoted", "number", "colon", "missing" };
    int fd = mkstemp(path);
    FILE *file;

    assert(fd >= 0);
    file = fdopen(fd, "w");
    assert(file != NULL);
    fputs("[test]\nquoted = \"a;b#c\"\nnumber = 42 ; note\ncolon: yes\n"
          "[duplicate]\nnumber=7\n[duplicate]\nnumber=9\n", file);
    assert(fclose(file) == 0);
    for (size_t i = 0; i < sizeof(sections) / sizeof(sections[0]); i++) {
        for (size_t j = 0; j < sizeof(keys) / sizeof(keys[0]); j++) {
            ini_snapshot_end();
            ini_gets(sections[i], keys[j], "default", direct, sizeof(direct), path);
            assert(ini_snapshot_begin(path) == 0);
            ini_gets(sections[i], keys[j], "default", cached, sizeof(cached), path);
            assert(strcmp(cached, direct) == 0);
        }
    }
    file = fopen(path, "w");
    assert(file != NULL);
    fputs("[test]\nnumber=123456\n", file);
    assert(fclose(file) == 0);
    assert(ini_getl("test", "number", 0, path) == 42);
    assert(ini_snapshot_begin(path) == 0);
    assert(ini_getl("test", "number", 0, path) == 123456);
    ini_snapshot_end();
    assert(unlink(path) == 0);
    assert(ini_snapshot_begin(path) != 0);
    assert(ini_getl("test", "number", 77, path) == 77);
    return 0;
}
