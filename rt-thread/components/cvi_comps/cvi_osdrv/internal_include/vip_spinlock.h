/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <stdio.h>

#ifdef CONFIG_PTHREAD_SPINLOCK_SUPPORT
#include <pthread.h>

typedef struct spinlock {
    unsigned int flag;
    pthread_spinlock_t ps;
} spinlock_t;

#define __SPIN_LOCK_UNLOCKED(lock) {.flag = 0}

#define DEFINE_SPINLOCK(lock) struct spinlock lock = { \
                                  .flag = 0}

#define spin_lock_init(lock)                                             \
    do {                                                                 \
        int ret = -1;                                                    \
        if ((lock)->flag == 1)                                           \
            break;                                                       \
        ret = pthread_spin_init(&((lock)->ps), PTHREAD_PROCESS_PRIVATE); \
        if (ret) {                                                       \
            printf("pthread_spin_init failed, ret:%d\r\n", ret);         \
        }                                                                \
        (lock)->flag = 1;                                                \
    } while (0)

// TODO: how to guarantee this procedure is atomic?
#define spin_lock_irqsave(lock, flags)  \
    do {                                \
        (void)flags;                    \
        if (!(lock)->flag) {            \
            spin_lock_init(lock);       \
            (lock)->flag = 1;           \
        }                               \
        pthread_spin_lock(&(lock)->ps); \
    } while (0)

#define spin_unlock_irqrestore(lock, flags) \
    do {                                    \
        (void)flags;                        \
        pthread_spin_unlock(&(lock)->ps);   \
    } while (0)

#define spin_lock(lock)                   \
    do {                                  \
        pthread_spin_lock(&((lock)->ps)); \
    } while (0)

#define spin_unlock(lock)                   \
    do {                                    \
        pthread_spin_unlock(&((lock)->ps)); \
    } while (0)

#define spin_lock_irq(lock)               \
    unsigned long flags;                  \
    do {                                  \
        pthread_spin_lock(&((lock)->ps)); \
    } while (0)

#define spin_unlock_irq(lock)               \
    do {                                    \
        pthread_spin_unlock(&((lock)->ps)); \
    } while (0)

#define spin_lock_destroy(lock)              \
    do {                                     \
        pthread_spin_destroy(&((lock)->ps)); \
    } while (0)
#else
#include "rttypes.h"
#include "rtthread.h"

typedef struct spinlock {
    unsigned int flag;
    rt_spinlock_t as;
    rt_base_t level;
} spinlock_t;

#define __SPIN_LOCK_UNLOCKED(lock) {.flag = 0}

#define DEFINE_SPINLOCK(lock) struct spinlock lock = { \
                                  .flag = 0}

#define spin_lock_init(lock)                  \
    do {                                      \
        if ((lock)->flag == 1)                \
            break;                            \
        rt_spin_lock_init(&((lock)->as)); \
        (lock)->flag = 1;                     \
    } while (0)

#define spin_lock_irqsave(lock, flags)                 \
    do {                                               \
        if (!(lock)->flag) {                           \
            rt_spin_lock_init(&((lock)->as));      \
            (lock)->flag = 1;                          \
        }                                              \
        (lock)->level = rt_spin_lock_irqsave(&((lock)->as)); \
    } while (0)

#define spin_unlock_irqrestore(lock, flags)                 \
    do {                                                    \
        (void)flags;                                        \
        rt_spin_unlock_irqrestore(&((lock)->as), (lock)->level); \
    } while (0)

#define spin_lock(lock)                  \
    do {                                 \
        rt_spin_lock(&((lock)->as)); \
    } while (0)

#define spin_unlock(lock)                  \
    do {                                   \
        rt_spin_unlock(&((lock)->as)); \
    } while (0)

#define spin_lock_irq(lock)              \
    unsigned long flags;                 \
    do {                                 \
        rt_spin_lock(&((lock)->as)); \
    } while (0)

#define spin_unlock_irq(lock)              \
    do {                                   \
        rt_spin_unlock(&((lock)->as)); \
    } while (0)

#define spin_lock_destroy(lock) \
    do {                        \
        (lock)->flag = 0;       \
    } while (0)
#endif

typedef spinlock_t raw_spinlock_t;

#define __RAW_SPIN_LOCK_UNLOCKED(lock) { \
    .flag = 0}
#define raw_spin_lock_init(lock) spin_lock_init(lock)

#define raw_spin_lock_irqsave(lock, flags) spin_lock_irqsave(lock, flags)
#define raw_spin_unlock_irqrestore(lock, flags) spin_unlock_irqrestore(lock, flags)

#define raw_spin_lock_irq(lock) spin_lock_irq(lock)
#define raw_spin_unlock_irq(lock) spin_unlock_irq(lock)

#endif //_SPINLOCK_H_
