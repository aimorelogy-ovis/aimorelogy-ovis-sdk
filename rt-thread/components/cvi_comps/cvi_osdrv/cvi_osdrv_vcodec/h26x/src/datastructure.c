//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2013  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include "main_helper.h"
#include "malloc.h"

Queue *Queue_Create(Uint32 itemCount, Uint32 itemSize)
{
	Queue *queue = NULL;

	queue = (Queue *)malloc(sizeof(Queue));
	if (queue == NULL)
		return NULL;
	queue->size = itemCount;
	queue->itemSize = itemSize;
	queue->count = 0;
	queue->front = 0;
	queue->rear = 0;
	queue->buffer = (Uint8 *)malloc(itemCount * itemSize);
	queue->lock = NULL;

	return queue;
}

void Queue_Destroy(Queue *queue)
{
	if (queue == NULL)
		return;

	if (queue->buffer)
		free(queue->buffer);
	if (queue->lock)
		VpuMutex_Destroy(queue->lock);
	free(queue);
}

BOOL Queue_Enqueue(Queue *queue, void *data)
{
	Uint8 *ptr;
	Uint32 offset;

	if (queue == NULL)
		return FALSE;

	/* Queue is full */
	if (queue->count == queue->size)
		return -1;

	if (queue->lock)
		VpuMutex_Lock(queue->lock);
	offset = queue->rear * queue->itemSize;

	ptr = &queue->buffer[offset];
	memcpy(ptr, data, queue->itemSize);
	queue->rear++;
	queue->rear %= queue->size;
	queue->count++;
	if (queue->lock)
		VpuMutex_Unlock(queue->lock);

	return TRUE;
}

void *Queue_Dequeue(Queue *queue)
{
	void *data;
	Uint32 offset;

	if (queue == NULL)
		return NULL;
	/* Queue is empty */
	if (queue->count == 0)
		return NULL;
	if (queue->lock)
		VpuMutex_Lock(queue->lock);
	offset = queue->front * queue->itemSize;
	data = (void *)&queue->buffer[offset];
	queue->front++;
	queue->front %= queue->size;
	queue->count--;
	if (queue->lock)
		VpuMutex_Unlock(queue->lock);
	return data;
}

void *Queue_Peek(Queue *queue)
{
	Uint32 offset;
	void *temp;

	if (queue == NULL)
		return NULL;
	/* Queue is empty */
	if (queue->count == 0)
		return NULL;
	if (queue->lock)
		VpuMutex_Lock(queue->lock);
	offset = queue->front * queue->itemSize;
	temp = (void *)&queue->buffer[offset];
	if (queue->lock)
		VpuMutex_Unlock(queue->lock);
	return temp;
}
