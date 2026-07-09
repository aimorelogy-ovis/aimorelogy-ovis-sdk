#ifndef __COMM_DEF_H__
#define __COMM_DEF_H__
#include <rtthread.h>

#define RT_STACK_SIZE_MIN 1024
#define RT_PRIORITY_BASE  1


typedef enum _QUEUE_HANDLE_E {
	E_QUEUE_ISP,
	E_QUEUE_VCODEC,
	E_QUEUE_VI,
	E_QUEUE_CAMERA,
	E_QUEUE_CMDQU,
	E_QUEUE_AUDIO,
	E_QUEUE_RGN,
	E_QUEUE_MAX,
} QUEUE_HANDLE_E;

#define STOP_CMD_DONE_NONE  0x00
#define STOP_CMD_DONE_ISP   0x01
#define STOP_CMD_DONE_VI    0x02
#define STOP_CMD_DONE_VCODE 0x04
#define STOP_CMD_DONE_ALL   (STOP_CMD_DONE_VI | STOP_CMD_DONE_VCODE)

typedef struct _TASK_CTX_S {
	char        name[32];
	rt_uint32_t stack_size;
	rt_uint8_t  priority;
	void (*runTask)(void *parameter);
	rt_uint8_t queLength;
	rt_mq_t    queHandle;
} TASK_CTX_S;

#endif /* __COMM_DEF_H__ */