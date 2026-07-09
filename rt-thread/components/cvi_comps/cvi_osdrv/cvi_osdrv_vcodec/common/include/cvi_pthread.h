#ifndef __CVI_PTHREAD_H__
#define __CVI_PTHREAD_H__

#include <base_ctx.h>
#include <pthread.h>
#include <semaphore.h>

#define MUTEX_T              pthread_mutex_t
#define MUTEX_INIT(mutex)    pthread_mutex_init(&mutex, NULL)
#define MUTEX_LOCK(mutex)    pthread_mutex_lock(&mutex)
#define MUTEX_UNLOCK(mutex)  pthread_mutex_unlock(&mutex)
#define MUTEX_DESTROY(mutex) pthread_mutex_destroy(&mutex)

// #define SEM_T ptread_sem_s
#define SEM_T              sem_t
#define SEM_INIT(mutex)    sem_init(&mutex, 0, 0)
#define SEM_WAIT(mutex)    sem_wait(&mutex)
#define SEM_POST(mutex)    sem_post(&mutex)
#define SEM_DESTROY(mutex) sem_destroy(&mutex)

#define PTHREAD_T pthread_t

#endif //__CVI_PTHREAD_H__
