/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: remote.c
 * Description:
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "sys/socket.h"
// #include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "cvi_buffer.h"
#include "raw_dump.h"
#include "raw_dump_internal.h"

#define LOGOUT(fmt, arg...) printf("[REMOTE] %s,%d: " fmt, __func__, __LINE__, ##arg)

#define ABORT_IF(EXP)                      \
    do {                                   \
        if (EXP) {                         \
            LOGOUT("abort: " #EXP "\r\n"); \
            abort();                       \
        }                                  \
    } while (0)

#define RETURN_FAILURE_IF(EXP)            \
    do {                                  \
        if (EXP) {                        \
            LOGOUT("fail: " #EXP "\r\n"); \
            return CVI_FAILURE;           \
        }                                 \
    } while (0)

#define GOTO_FAIL_IF(EXP, LABEL)          \
    do {                                  \
        if (EXP) {                        \
            LOGOUT("fail: " #EXP "\r\n"); \
            goto LABEL;                   \
        }                                 \
    } while (0)

#define WARN_IF(EXP)                      \
    do {                                  \
        if (EXP) {                        \
            LOGOUT("warn: " #EXP "\r\n"); \
        }                                 \
    } while (0)

#define ERROR_IF(EXP)                      \
    do {                                   \
        if (EXP) {                         \
            LOGOUT("error: " #EXP "\r\n"); \
        }                                  \
    } while (0)

#define REMOTE_PORT 5567
#define CMD_LENGTH  64

static int handle_raw_dump(int data_length);

/*
 * cmd: 64 byte
 *
 * cvitek cmd_str data_length
 *
 */

typedef int (*pcmd_fun)(int data_length);

typedef struct {
    char    *cmd_str;
    pcmd_fun fun;
} cmd_item_s;

cmd_item_s cmd_list[] = {{"raw_dump", handle_raw_dump}};

#define QUEUE_LENGTH 1

static int sock_fd;
static int conn_fd;

static VI_PIPE ViPipe;

static int recv_data(uint8_t *data, int length)
{
    int recv_count = 0;

    do {
        recv_count += recv(conn_fd, data + recv_count, length - recv_count, 0);

        if (recv_count <= 0) {
            return recv_count;
        }
    } while (recv_count != length);

    return recv_count;
}

void *cvi_raw_dump_run(void *param)
{
    param = param;

    sock_fd = -1;
    conn_fd = -1;

    char recv_cmd_buffer[CMD_LENGTH];

    int                ret;
    int                recv_count = 0;
    struct sockaddr_in addr_serv;
    struct sockaddr_in client_addr;
    socklen_t          addr_len = sizeof(struct sockaddr_in);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    GOTO_FAIL_IF(sock_fd < 0, fail);

    // LOGOUT("create socket successful!!\r\n");

    int reuseaddr = 1;

    ret = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_family      = AF_INET;
    addr_serv.sin_port        = htons(REMOTE_PORT);
    addr_serv.sin_addr.s_addr = htons(INADDR_ANY);

    ret = bind(sock_fd, (struct sockaddr *)&addr_serv, sizeof(struct sockaddr_in));
    GOTO_FAIL_IF(ret < 0, fail);

    // LOGOUT("bind successful!!\r\n");
    LOGOUT("raw dump ready...\r\n");

    ret = listen(sock_fd, QUEUE_LENGTH);
    GOTO_FAIL_IF(ret < 0, fail);

accept_again:

    conn_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &addr_len);
    GOTO_FAIL_IF(conn_fd < 0, fail);

    LOGOUT("accept client ip: %s port: %d\r\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    while (1) {
        LOGOUT("start recv cmd...\r\n");

        memset(recv_cmd_buffer, 0, CMD_LENGTH);

        recv_count = recv_data((uint8_t *)recv_cmd_buffer, CMD_LENGTH);
        GOTO_FAIL_IF(recv_count < 0, fail);

        if (recv_count == 0) {
            // break;
            LOGOUT("interrupt, accept again...\r\n");
            close(conn_fd);
            conn_fd = -1;
            goto accept_again;
        }

        LOGOUT("recv cmd: %s\r\n", recv_cmd_buffer);

        if (strncmp((const char *)recv_cmd_buffer, "cvitek", strlen("cvitek")) != 0x00) {
            LOGOUT("recv unknown cmd!!!\r\n");
            continue;
        }

        char *p = strchr((const char *)recv_cmd_buffer, ' ');

        if (p == NULL) {
            continue;
        }

        p++;

        for (uint32_t i = 0; i < sizeof(cmd_list) / sizeof(cmd_item_s); i++) {
            if (strstr(p, cmd_list[i].cmd_str) != NULL) {
                uint32_t data_length = 0;

                LOGOUT("\r\nexec cmd: %s\r\n", p);

                p = strchr(p, ' ');

                if (p == NULL) {
                    break;
                }

                p++;

                data_length = atoi(p);

                cmd_list[i].fun(data_length);
            }
        }
    }

    if (conn_fd > 0) {
        close(conn_fd);
    }

    if (sock_fd > 0) {
        close(sock_fd);
    }

    return 0;

fail:

    if (conn_fd > 0) {
        close(conn_fd);
    }

    if (sock_fd > 0) {
        close(sock_fd);
    }

    return (void *)-1;
}

static int handle_raw_dump(int data_length)
{
    int ret        = 0;
    int recv_count = 0;

    LOGOUT("raw dump data_length: %d\r\n", data_length);

    uint8_t *data = calloc(1, data_length + 1);

    recv_count = recv_data(data, data_length);
    GOTO_FAIL_IF(recv_count <= 0, fail);

    LOGOUT("raw dump param: %s\r\n", data);

    RAW_DUMP_INFO_S stRawDumpInfo = {0};

    char *path = (char *)data;

    char *param = strchr((const char *)data, ' ');

    GOTO_FAIL_IF(param == NULL, fail);

    *param = '\0';

    param++;

    if (strcmp("remote", path) == 0) {
        stRawDumpInfo.bDump2Remote = true;
    } else {
        stRawDumpInfo.bDump2Remote = false;
    }

    // GOTO_FAIL_IF(access(path, F_OK) != 0, fail);

    char *ptemp = param;

    stRawDumpInfo.u32TotalFrameCnt = atoi(ptemp);

    ptemp = strchr(ptemp, ',');
    ptemp++;
    GOTO_FAIL_IF(ptemp == NULL, fail);
    stRawDumpInfo.stRoiRect.s32X = atoi(ptemp);

    ptemp = strchr(ptemp, ',');
    ptemp++;
    GOTO_FAIL_IF(ptemp == NULL, fail);
    stRawDumpInfo.stRoiRect.s32Y = atoi(ptemp);

    ptemp = strchr(ptemp, ',');
    ptemp++;
    GOTO_FAIL_IF(ptemp == NULL, fail);
    stRawDumpInfo.stRoiRect.u32Width = atoi(ptemp);

    ptemp = strchr(ptemp, ',');
    ptemp++;
    GOTO_FAIL_IF(ptemp == NULL, fail);
    stRawDumpInfo.stRoiRect.u32Height = atoi(ptemp);

    LOGOUT("bDump2Remote: %d, count: %d, roi: %d,%d,%d,%d\r\n", stRawDumpInfo.bDump2Remote,
           stRawDumpInfo.u32TotalFrameCnt, stRawDumpInfo.stRoiRect.s32X,
           stRawDumpInfo.stRoiRect.s32Y, stRawDumpInfo.stRoiRect.u32Width,
           stRawDumpInfo.stRoiRect.u32Height);

    ret = cvi_raw_dump(ViPipe, &stRawDumpInfo);

    if (data != NULL) {
        free(data);
    }

    return ret;

fail:

    if (data != NULL) {
        free(data);
    }

    return -1;
}

int send_file(const CVI_U8 *pu8data, CVI_U32 u32dateSize, const CVI_CHAR *pcname)
{
    RETURN_FAILURE_IF(conn_fd < 0);

    int ret = 0;

    CVI_S8 as8cmd_buffer[CMD_LENGTH] = {0};

    snprintf((char *)as8cmd_buffer, sizeof(as8cmd_buffer), "%s %d", "cvitek file",
             u32dateSize + FILE_NAME_MAX_LENGTH);

    LOGOUT("send cmd: %s\r\n", as8cmd_buffer);

    ret = send(conn_fd, as8cmd_buffer, CMD_LENGTH, 0);
    GOTO_FAIL_IF(ret != CMD_LENGTH, fail);

    LOGOUT("send file name: %s\r\n", pcname);

    ret = send(conn_fd, pcname, FILE_NAME_MAX_LENGTH, 0);
    GOTO_FAIL_IF(ret != FILE_NAME_MAX_LENGTH, fail);

    LOGOUT("send file data...\r\n");

    ret = send(conn_fd, pu8data, u32dateSize, 0);
    GOTO_FAIL_IF(ret != u32dateSize, fail);

    return 0;
fail:
    return ret;
}

int send_raw(const CVI_CHAR *pcfileName, int totalLength)
{
    RETURN_FAILURE_IF(conn_fd < 0);
    int ret = 0;

    char cmd_buffer[CMD_LENGTH] = {0};

    snprintf(cmd_buffer, sizeof(cmd_buffer), "%s %d", "cvitek raw",
             totalLength + FILE_NAME_MAX_LENGTH);

    LOGOUT("send cmd: %s\r\n", cmd_buffer);

    ret = send(conn_fd, cmd_buffer, CMD_LENGTH, 0);

    GOTO_FAIL_IF(ret != CMD_LENGTH, fail);

    LOGOUT("send file name: %s\r\n", pcfileName);

    ret = send(conn_fd, pcfileName, FILE_NAME_MAX_LENGTH, 0);
    GOTO_FAIL_IF(ret != FILE_NAME_MAX_LENGTH, fail);

    return 0;
fail:
    return ret;
}

int send_raw_data(const uint8_t *data, int length)
{
    RETURN_FAILURE_IF(conn_fd < 0);

    int ret = 0;

    LOGOUT("send raw data length: %d\r\n", length);

    ret = send(conn_fd, data, length, 0);
    GOTO_FAIL_IF(ret != length, fail);

    return 0;

fail:
    return ret;
}

int send_status(int status)
{
    RETURN_FAILURE_IF(conn_fd < 0);

    int ret = 0;

    char cmd_buffer[CMD_LENGTH];

    memset(cmd_buffer, 0, CMD_LENGTH);

    snprintf(cmd_buffer, CMD_LENGTH, "%s %d", "cvitek status", status);

    LOGOUT("send cmd: %s\r\n", cmd_buffer);

    ret = send(conn_fd, cmd_buffer, CMD_LENGTH, 0);
    GOTO_FAIL_IF(ret != CMD_LENGTH, fail);

    return 0;

fail:
    return ret;
}

static pthread_t tid;

int cvi_raw_dump_init(void)
{
    int ret = 0;

    ViPipe = 0;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 128 * 1024);

    ret = pthread_create(&tid, &attr, cvi_raw_dump_run, NULL);
    pthread_setname_np(tid, "RAWDump");
    RETURN_FAILURE_IF(ret < 0);

    return ret;
}

int cvi_raw_dump_uninit(void)
{
    void *ret;

    if (tid != 0) {
        pthread_cancel(tid);
        pthread_join(tid, &ret);
    }

    if (conn_fd > 0) {
        close(conn_fd);
    }

    if (sock_fd > 0) {
        close(sock_fd);
    }

    return 0;
}
