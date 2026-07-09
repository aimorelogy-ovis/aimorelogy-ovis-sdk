//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include "errno.h"
#include "main_helper.h"
#include "malloc.h"

/*
 * Platform dependent functions
 */
#if defined(PLATFORM_LINUX)
#include "pthread.h"
#include "unistd.h"

typedef void *(*PTHREAD_START_ROUTINE)(void *);

VpuThread VpuThread_Create(VpuThreadRunner runner, void *arg)
{
	Int32 ret;
	VpuThread thread = (pthread_t *)malloc(sizeof(pthread_t));

	ret = pthread_create((pthread_t *)thread, NULL, (PTHREAD_START_ROUTINE)runner, arg);
	if (ret != 0) {
		free(thread);
		VLOG(ERR, "Failed to pthread_create ret(%d)\n", ret);
		return NULL;
	}

	return thread;
}

BOOL VpuThread_Join(VpuThread thread)
{
	Int32 ret;
	pthread_t pthreadHandle;

	if (thread == NULL) {
		VLOG(ERR, "%s:%d invalid thread handle\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	pthreadHandle = *(pthread_t *)thread;

	ret = pthread_join(pthreadHandle, NULL);
	if (ret != 0) {
		VLOG(ERR, "%s:%d Failed to pthread_join ret(%d)\n", __FUNCTION__, __LINE__, ret);
		return FALSE;
	}

	return TRUE;
}

void MSleep(Uint32 ms)
{
	usleep(ms * 1000);
}

typedef struct {
	pthread_mutex_t lock;
} VpuMutexHandle;

VpuMutex VpuMutex_Create(void)
{
	VpuMutexHandle *handle = (VpuMutexHandle *)malloc(sizeof(VpuMutexHandle));

	if (handle == NULL) {
		VLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	if (pthread_mutex_init(&handle->lock, NULL) < 0) {
		free(handle);
		VLOG(ERR, "%s:%d failed to pthread_mutex_init() errno(%d)\n", __FUNCTION__, __LINE__, errno);
		return NULL;
	}

	return (VpuMutex)handle;
}

void VpuMutex_Destroy(VpuMutex handle)
{
	if (handle == NULL) {
		VLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
		return;
	}

	free(handle);
}

BOOL VpuMutex_Lock(VpuMutex handle)
{
	VpuMutexHandle *mutex = (VpuMutexHandle *)handle;

	if (mutex == NULL) {
		VLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	pthread_mutex_lock(&mutex->lock);

	return TRUE;
}

BOOL VpuMutex_Unlock(VpuMutex handle)
{
	VpuMutexHandle *mutex = (VpuMutexHandle *)handle;

	if (mutex == NULL) {
		VLOG(ERR, "%s:%d Invalid mutex handle\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	pthread_mutex_unlock(&mutex->lock);

	return TRUE;
}
#else
VpuThread VpuThread_Create(VpuThreadRunner runner, void *arg)
{
	UNREFERENCED_PARAMETER(runner);
	UNREFERENCED_PARAMETER(arg);

	VLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

	return NULL;
}

BOOL VpuThread_Join(VpuThread thread)
{
	UNREFERENCED_PARAMETER(thread);

	VLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

	return FALSE;
}

void MSleep(Uint32 ms)
{
	UNREFERENCED_PARAMETER(ms);
}

VpuMutex VpuMutex_Create(void)
{
	void *ctx;
	VLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

	ctx = malloc(sizeof(Int32));

	return (VpuMutex)ctx;
}

void VpuMutex_Destroy(VpuMutex handle)
{
	UNREFERENCED_PARAMETER(handle);

	VLOG(WARN, "%s not implemented yet\n", __FUNCTION__);

	free((void *)handle);
}

BOOL VpuMutex_Lock(VpuMutex handle)
{
	UNREFERENCED_PARAMETER(handle);

	return TRUE;
}

BOOL VpuMutex_Unlock(VpuMutex handle)
{
	UNREFERENCED_PARAMETER(handle);

	return TRUE;
}
#endif /* WINDOWS PLATFORM */
