#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rbuf.h"

// 模拟内存分配和释放的回调函数
void *mock_malloc(uint32_t size, const char *name) {
    printf("分配 %u 字节用于环形缓冲区: %s\n", size, name);
    return malloc(size);
}

void mock_free(void *ptr) {
    printf("释放环形缓冲区内存\n");
    free(ptr);
}

int main() {
    void *rbuf = NULL;
    const char *buffer_name = "SampleRingBuffer";
    int32_t buffer_size = 1024; // 1 KB 缓冲区
    int32_t outcnt = 1; // 一个输出指针

    // 初始化环形缓冲区
    if (RBUF_Init(&rbuf, buffer_size, buffer_name, outcnt, mock_malloc, mock_free) != 0) {
        printf("初始化环形缓冲区失败\n");
        return -1;
    }
    printf("环形缓冲区 '%s' 初始化成功\n", buffer_name);

    // 写入数据到环形缓冲区
    const char *data1 = "Hello, Ring Buffer!";
    int32_t data1_len = strlen(data1) + 1; // 包括字符串结束符
    if (RBUF_Copy_In(rbuf, (void *)data1, data1_len, 0) == 0) {
        printf("数据写入环形缓冲区: %s\n", data1);
        RBUF_Refresh_In(rbuf, data1_len);
    } else {
        printf("写入数据到环形缓冲区失败\n");
    }

    // 再次写入数据
    const char *data2 = "Another message!";
    int32_t data2_len = strlen(data2) + 1;
    if (RBUF_Copy_In(rbuf, (void *)data2, data2_len, 0) == 0) {
        printf("数据写入环形缓冲区: %s\n", data2);
        RBUF_Refresh_In(rbuf, data2_len);
    } else {
        printf("写入数据到环形缓冲区失败\n");
    }

    // 从环形缓冲区读取数据
    char read_buffer[128] = {0};
    if (RBUF_Copy_Out(rbuf, read_buffer, data1_len, 0, 0) == 0) {
        printf("从环形缓冲区读取数据: %s\n", read_buffer);
        RBUF_Refresh_Out(rbuf, data1_len, 0);
    } else {
        printf("读取数据失败\n");
    }

    // 再次读取数据
    memset(read_buffer, 0, sizeof(read_buffer));
    if (RBUF_Copy_Out(rbuf, read_buffer, data2_len, 0, 0) == 0) {
        printf("从环形缓冲区读取数据: %s\n", read_buffer);
        RBUF_Refresh_Out(rbuf, data2_len, 0);
    } else {
        printf("读取数据失败\n");
    }

    // 显示环形缓冲区日志
    RBUF_ShowLog(rbuf);

    // 销毁环形缓冲区
    RBUF_DeInit(rbuf);
    printf("环形缓冲区已销毁\n");

    return 0;
}
