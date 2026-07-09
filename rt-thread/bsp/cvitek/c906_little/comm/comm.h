#ifndef __COMM_H__
#define __COMM_H__
#include <rtthread.h>
#include "comm_def.h"

void prvQueueISR(int vector, void *param);
void prvCmdQuRunTask(void *parameter);
void prvAudioRunTask(void *pvParameters);
void GetCommInfo(void);
void prvRGNRunTask(void *pvParameters);
rt_mq_t main_GetMODHandle(QUEUE_HANDLE_E handle_idx);
void    main_create_comm_tasks(void);
#endif /* __COMM_H__ */