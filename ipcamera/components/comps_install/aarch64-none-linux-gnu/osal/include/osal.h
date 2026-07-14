#ifndef __OSAL_H__
#define __OSAL_H__
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#define OSAL_SUCCESS ((int32_t)(0))
#define OSAL_ERR_FAILURE ((int32_t)(-1001))
#define OSAL_ERR_NOMEM ((int32_t)(-1002))
#define OSAL_ERR_TIMEOUT ((int32_t)(-1003))


#ifdef __cplusplus
extern "C" {
#endif
/*
 * time
 */
int32_t OSAL_TIME_GetBootTimeUs(uint64_t *time_us);
int32_t OSAL_TIME_GetBootTimeMs(uint64_t *time_ms);
int32_t OSAL_TIME_GetBootTimeNs(uint64_t *time_ns);
/*
 * mutex
 */
typedef struct OSAL_MUTEX_ATTR {
	const char *name;
	int32_t type;
} OSAL_MUTEX_ATTR_S;
typedef struct OSAL_MUTEX {
	OSAL_MUTEX_ATTR_S attr;
	pthread_mutex_t mutex;
} OSAL_MUTEX_S, *OSAL_MUTEX_HANDLE_S;
int32_t OSAL_MUTEX_Create(OSAL_MUTEX_ATTR_S *attr, OSAL_MUTEX_HANDLE_S *mutex);
int32_t OSAL_MUTEX_Destroy(OSAL_MUTEX_HANDLE_S mutex);
int32_t OSAL_MUTEX_Lock(OSAL_MUTEX_HANDLE_S mutex);
int32_t OSAL_MUTEX_Unlock(OSAL_MUTEX_HANDLE_S mutex);
#define OSAL_MUTEX_INITIALIZER \
	{                              \
		{"static", PTHREAD_MUTEX_NORMAL}, PTHREAD_MUTEX_INITIALIZER}
#ifdef __USE_GNU
#define OSAL_MUTEX_INITIALIZER_R \
	{                                \
		{"static_r", PTHREAD_MUTEX_RECURSIVE}, PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP}
#else

#ifdef __arm__
#define OSAL_MUTEX_INITIALIZER_R                                   \
	{                                                                  \
		{"static_r", PTHREAD_MUTEX_RECURSIVE},                         \
		{                                                              \
			{                                                          \
				{                                                      \
                    PTHREAD_MUTEX_RECURSIVE, 0, 0, 0, 0, 0 				\
                }                                                      \
            }                                                          \
		}                                                              \
	}
#else
#define OSAL_MUTEX_INITIALIZER_R                                   \
	{                                                                  \
		{"static_r", PTHREAD_MUTEX_RECURSIVE},                         \
		{                                                              \
			{                                                          \
				{                                                      \
                    PTHREAD_MUTEX_RECURSIVE, 0, 0, 0, 0, 0, 0, 0, 0, 0 \
                }                                                      \
            }                                                          \
		}                                                              \
	}
#endif
#endif
typedef struct OSAL_COND_ATTR {
	const char *name;
	int32_t clock;
} OSAL_COND_ATTR_S;
typedef struct OSAL_COND {
	OSAL_COND_ATTR_S attr;
	pthread_cond_t cond;
} OSAL_COND_S, *OSAL_COND_HANDLE_S;
int32_t OSAL_COND_Create(OSAL_COND_ATTR_S *attr, OSAL_COND_HANDLE_S *cond);
int32_t OSAL_COND_Destroy(OSAL_COND_HANDLE_S cond);
int32_t OSAL_COND_Signal(OSAL_COND_HANDLE_S cond);
int32_t OSAL_COND_Wait(OSAL_COND_HANDLE_S cond, OSAL_MUTEX_HANDLE_S mutex);
int32_t OSAL_COND_TimedWait(OSAL_COND_HANDLE_S cond, OSAL_MUTEX_HANDLE_S mutex, int64_t timeout_us);
#define OSAL_COND_INITIALIZER \
	{                             \
		{"static", CLOCK_MONOTONIC}, PTHREAD_COND_INITIALIZER}



#define OSAL_SEM_NO_WAIT ((int64_t)(0))
#define OSAL_SEM_WAIT_FOREVER ((int64_t)(-1))
typedef struct OSAL_SEM_ATTR {
	const char *name;
} OSAL_SEM_ATTR_S;
typedef struct OSAL_SEM {
	OSAL_SEM_ATTR_S attr;
	sem_t sem;
} OSAL_SEM_S, *OSAL_SEM_HANDLE_S;
int32_t OSAL_SEM_Create(OSAL_SEM_ATTR_S *attr, OSAL_SEM_HANDLE_S *sem);
int32_t OSAL_SEM_Destroy(OSAL_SEM_HANDLE_S sem);
int32_t OSAL_SEM_Wait(OSAL_SEM_HANDLE_S sem, int64_t timeout_us);
int32_t OSAL_SEM_Post(OSAL_SEM_HANDLE_S sem);

/*
 * task
 */

#define OSAL_TASK_PRI_NORMAL ((int32_t)0)
#define OSAL_TASK_PRI_RT_LOWEST ((int32_t)1)
#define OSAL_TASK_PRI_RT_LOW ((int32_t)9)
#define OSAL_TASK_PRI_RT_MID ((int32_t)49)
#define OSAL_TASK_PRI_RT_HIGH ((int32_t)80)
#define OSAL_TASK_PRI_RT_HIGHEST ((int32_t)99)

typedef void (*OSAL_TASK_Entry_CB)(void *arg);
typedef struct OSAL_TASK_ATTR {
	const char *name;
	OSAL_TASK_Entry_CB entry;
	void *param;
	int32_t priority;
	bool detached;
	int32_t stack_size;
} OSAL_TASK_ATTR_S;
typedef struct OSAL_TASK {
	OSAL_TASK_ATTR_S attr;
	pthread_t task;
} OSAL_TASK_S, *OSAL_TASK_HANDLE_S;
int32_t OSAL_TASK_Create(OSAL_TASK_ATTR_S *attr, OSAL_TASK_HANDLE_S *task);
int32_t OSAL_TASK_Destroy(OSAL_TASK_HANDLE_S *task);
int32_t OSAL_TASK_Join(OSAL_TASK_HANDLE_S task);
void OSAL_TASK_Sleep(int64_t time_us);
void OSAL_TASK_Resched(void);

/*
 * fs
 */

#if defined(__UCLIBC__)
#include <sys/syscall.h>
#define FALLOC_FL_KEEP_SIZE 0x01
#define HIDWORD(a) ((uint32_t)(((uint64_t)(a)) >> 32))
#define LODWORD(a) ((uint32)(uint64_t)(a))
#define fallocate(fd, mode, offset, len) syscall(__NR_fallocate, fd, mode, LODWORD(offset), HIDWORD(offset), LODWORD(len), HIDWORD(len))
#endif

#define OSAL_FS_MAX_PATH_LEN (64)

typedef enum {
	OSAL_FS_TYPE_FATXX,
	OSAL_FS_TYPE_EXFAT,
	OSAL_FS_TYPE_EXT2,
	OSAL_FS_TYPE_EXT3,
	OSAL_FS_TYPE_NTFS,
	OSAL_FS_TYPE_NFS,
	OSAL_FS_TYPE_CIFS,
	OSAL_FS_TYPE_UNKNOWN
} OSAL_FS_TYPE_E;

int32_t OSAL_FS_Insmod(const char *pszPath, const char *pszOptions);
int32_t OSAL_FS_Rmmod(const char *pszPath);
int32_t OSAL_FS_PathIsDirectory(const char *pszPath);
int32_t OSAL_FS_Rmdir(const char *pszPath);
int32_t OSAL_FS_Mkdir(const char *pszPath, mode_t mode);
int32_t OSAL_FS_System(const char *pszCmd);
int32_t OSAL_FS_Du(const char *pszPath, uint64_t *pu64Size_KB);
int32_t OSAL_FS_Async(void);
int32_t OSAL_FS_Touch(const char *file);
OSAL_FS_TYPE_E OSAL_FS_GetFileSystemType(const char *path);
int32_t OSAL_FS_SetHiddenAttribute(const char *path, int32_t set);
int32_t OSAL_FS_Ftruncate(const char *file, uint64_t sizeBits);

#ifdef __cplusplus
}
#endif
#endif
