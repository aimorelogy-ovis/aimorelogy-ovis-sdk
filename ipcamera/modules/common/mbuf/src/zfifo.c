#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "app_ipcam_comm.h"
#include "cvi_mbuf.h"
#include "zfifo.h"

ZFIFO *zfifo_init(const char *zfifo_name, int size)
{
    ZFIFO *zfifo = (ZFIFO *)malloc(sizeof(ZFIFO));
    if (zfifo == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc failed\n");
        return NULL;
    }
    memset(zfifo, 0, sizeof(ZFIFO));

    zfifo->name = (char *)malloc(strlen(zfifo_name) + 1);
    if (zfifo->name == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc failed\n");
        free(zfifo);
        return NULL;
    }
    sprintf(zfifo->name, "%s", zfifo_name);

    zfifo->buf = malloc(size);
    if (zfifo->buf == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc failed\n");
        free(zfifo->name);
        free(zfifo);
        return NULL;
    }

    zfifo->buf_size = size;
    pthread_mutex_init(&zfifo->mutex, 0);
    pthread_cond_init(&zfifo->cond, NULL);
    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "zfifo:%p,data:%p,size:%d\n", zfifo, zfifo->buf, zfifo->buf_size);
    return zfifo;
}

void zfifo_uninit(ZFIFO *zfifo)
{
    if (zfifo == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "zfifo uninit\n");
        return;
    }
    pthread_mutex_destroy(&zfifo->mutex);
    pthread_cond_destroy(&zfifo->cond);
    if (zfifo->buf != NULL)
    {
        free(zfifo->buf);
        zfifo->buf = NULL;
    }

    if (zfifo->name != NULL)
    {
        free(zfifo->name);
        zfifo->name = NULL;
    }

    if (zfifo != NULL)
    {
        free(zfifo);
        zfifo = NULL;
    }
}

ZFIFO_DESC *zfifo_open(ZFIFO *fifo)
{
    if (fifo == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "zfifo uninit\n");
        return NULL;
    }

    ZFIFO_DESC *zfifo_desc = (ZFIFO_DESC *)malloc(sizeof(ZFIFO_DESC));
    if (zfifo_desc == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc failed\n");
        return NULL;
    }
    memset(zfifo_desc, 0, sizeof(ZFIFO_DESC));
    zfifo_desc->zfifo = fifo;

    zfifo_desc->index = 1;
    zfifo_desc->offset = 0;
    return zfifo_desc;
}

void zfifo_close(ZFIFO_DESC *zfifo_desc)
{
    if (zfifo_desc != NULL)
    {
        free(zfifo_desc);
        zfifo_desc = NULL;
    }
}

int zfifo_wirte_node(void *buf, const ZFIFO_NODE *iov, int iovcnt)
{
    int i;
    int offset = 0;
    for (i = 0; i < iovcnt; i++)
    {
        memcpy(buf + offset, &(iov[i].len), sizeof(iov[i].len));
        offset += sizeof(iov[i].len);
        memcpy(buf + offset, iov[i].base, iov[i].len);
        offset += iov[i].len;
    }

    return offset;
}

int zfifo_read_node(const void *buf, ZFIFO_NODE *iov, int iovcnt)
{
    int i;
    int offset = 0;
    int buf_size;
    for (i = 0; i < iovcnt; i++)
    {
        buf_size = iov[i].len;
        memcpy(&(iov[i].len), buf + offset, sizeof(iov[i].len));
        offset += sizeof(iov[i].len);
        if (iov[i].len <= buf_size)
        {
            memcpy(iov[i].base, buf + offset, iov[i].len);
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Buffer size:%d is not enough, expect size:%d. \n"
                , buf_size
                , iov[i].len);
            return -1;
        }
        offset += iov[i].len;
    }

    return offset;
}

int zfifo_del_node(void *buf, int iovcnt)
{
    int i;
    int offset = 0;
    int len_tmp;

    for (i = 0; i < iovcnt; i++)
    {
        memcpy(&len_tmp, buf + offset, sizeof(len_tmp));
        offset += sizeof(len_tmp);
        offset += len_tmp;
    }

    return offset;
}

int calc_len(const ZFIFO_NODE *iov, int iovcnt)
{
    int i;
    int alllen = 0;

    for (i = 0; i < iovcnt; i++)
    {
        alllen += sizeof(iov[i].len);
        //APP_PROF_LOG_PRINT(LEVEL_DEBUG, "zfifo calc len:%d\n", alllen);
        alllen += iov[i].len;
        //APP_PROF_LOG_PRINT(LEVEL_DEBUG, "zfifo calc len:%d\n", alllen);
    }

    return alllen;
}

int zfifo_writev(ZFIFO_DESC *zfifo_desc, const ZFIFO_NODE *iov, int iovcnt)
{
    if ((zfifo_desc == NULL) || (iov == NULL))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param is wrong.\n");
        return -1;
    }
    ZFIFO *zfifo = zfifo_desc->zfifo;

    int all_len = calc_len(iov, iovcnt);
    if (all_len > zfifo->buf_size)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "buf is too small.\n");
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "all_len = %d, buf size = %d \n", all_len, zfifo->buf_size);
        return -1;
    }

    // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "all_len is %d.\n", all_len);
    int n = 0;
    pthread_mutex_lock(&zfifo->mutex);
    if (zfifo->end_offset == 0)
    {
        n = zfifo_wirte_node(zfifo->buf + 0, iov, iovcnt);
        zfifo->first_index = 1;
        zfifo->first_offset = 0;
        zfifo->last_index = 1;
        zfifo->last_offset = 0;
        zfifo->end_offset += n;
        zfifo->deadline_offset = -1;

        pthread_cond_broadcast(&zfifo->cond);
        pthread_mutex_unlock(&zfifo->mutex);
        return n;
    }

    if (zfifo->end_offset > zfifo->first_offset)
    {
        if (zfifo->end_offset + all_len <= zfifo->buf_size)
        {
            n = zfifo_wirte_node(zfifo->buf + zfifo->end_offset, iov, iovcnt);
            zfifo->last_index++;
            zfifo->last_offset = zfifo->end_offset;
            zfifo->end_offset += n;

            if (zfifo->end_offset > zfifo->deadline_offset)
                zfifo->deadline_offset = zfifo->end_offset;
        }
        else
        {
            zfifo->deadline_offset = zfifo->end_offset;

            while ((zfifo->first_offset < zfifo->deadline_offset) &&
                   (zfifo->first_offset < all_len))
            {
                n = zfifo_del_node(zfifo->buf + zfifo->first_offset, iovcnt);
                zfifo->first_index++;
                zfifo->first_offset += n;
            }
            if (zfifo->first_offset == zfifo->deadline_offset)
                zfifo->first_offset = 0;

            n = zfifo_wirte_node(zfifo->buf + 0, iov, iovcnt);
            zfifo->last_index++;
            zfifo->last_offset = 0;
            zfifo->end_offset = n;
            if (zfifo->end_offset > zfifo->buf_size)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "write offset:%d, deadline:%d buf size:%d\n", zfifo->end_offset, zfifo->deadline_offset, zfifo->buf_size);
                pthread_mutex_unlock(&zfifo->mutex);
                return -1;
            }
        }
    }
    else
    {
        if (zfifo->end_offset + all_len <= zfifo->buf_size)
        {
            while ((zfifo->first_offset < zfifo->deadline_offset) &&
                   (zfifo->first_offset - zfifo->end_offset < all_len))
            {
                n = zfifo_del_node(zfifo->buf + zfifo->first_offset, iovcnt);
                zfifo->first_index++;
                zfifo->first_offset += n;
            }
            if (zfifo->first_offset == zfifo->deadline_offset)
            {
                zfifo->first_offset = 0;
                n = zfifo_wirte_node(zfifo->buf + zfifo->end_offset, iov, iovcnt);
                zfifo->last_index++;
                zfifo->last_offset = zfifo->end_offset;
                zfifo->end_offset += n;
                if (zfifo->end_offset > zfifo->deadline_offset)
                    zfifo->deadline_offset = zfifo->end_offset;
            }
            else
            {
                n = zfifo_wirte_node(zfifo->buf + zfifo->end_offset, iov, iovcnt);
                zfifo->last_index++;
                zfifo->last_offset = zfifo->end_offset;
                zfifo->end_offset += n;
            }

            if (zfifo->end_offset > zfifo->buf_size)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "write offset:%d, deadline:%d buf size:%d\n", zfifo->end_offset, zfifo->deadline_offset, zfifo->buf_size);
                pthread_mutex_unlock(&zfifo->mutex);
                return -1;
            }
        }
        else
        {
            while (zfifo->first_offset < zfifo->deadline_offset)
            {
                n = zfifo_del_node(zfifo->buf + zfifo->first_offset, iovcnt);
                zfifo->first_index++;
                zfifo->first_offset += n;
            }
            zfifo->first_offset = 0;

            zfifo->deadline_offset = zfifo->end_offset;

            while ((zfifo->first_offset < zfifo->deadline_offset) &&
                   (zfifo->first_offset < all_len))
            {
                n = zfifo_del_node(zfifo->buf + zfifo->first_offset, iovcnt);
                zfifo->first_index++;
                zfifo->first_offset += n;
            }
            if (zfifo->first_offset == zfifo->deadline_offset)
                zfifo->first_offset = 0;

            n = zfifo_wirte_node(zfifo->buf + 0, iov, iovcnt);
            zfifo->last_index++;
            zfifo->last_offset = 0;
            zfifo->end_offset = n;
            if (zfifo->end_offset > zfifo->buf_size)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "write offset:%d, deadline:%d buf size:%d\n", zfifo->end_offset, zfifo->deadline_offset, zfifo->buf_size);
                pthread_mutex_unlock(&zfifo->mutex);
                return -1;
            }
        }
    }

    pthread_cond_broadcast(&zfifo->cond);
    pthread_mutex_unlock(&zfifo->mutex);

    return n;
}

int zfifo_writev_plus(ZFIFO_DESC *zfifo_desc, const ZFIFO_NODE *iov, int iovcnt, int flag)
{
    if ((zfifo_desc == NULL) || (iov == NULL))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param is wrong.");
        return -1;
    }
    ZFIFO *zfifo = zfifo_desc->zfifo;

    int all_len = calc_len(iov, iovcnt);
    if (all_len > zfifo->buf_size)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "buf is too small.\n");
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "all_len = %d, buf size = %d \n", all_len, zfifo->buf_size);
        return -1;
    }

    // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "all_len is %d.\n", all_len);
    int n = 0;
    pthread_mutex_lock(&zfifo->mutex);
    if (zfifo->end_offset == 0)
    {
        n = zfifo_wirte_node(zfifo->buf + 0, iov, iovcnt);
        zfifo->first_index = 1;
        zfifo->first_offset = 0;
        zfifo->last_index = 1;
        zfifo->last_offset = 0;
        if (flag)
        {
            zfifo->flag_index = zfifo->last_index;
            zfifo->flag_offset = zfifo->last_offset;
        }
        zfifo->end_offset += n;
        zfifo->deadline_offset = -1;

        pthread_cond_broadcast(&zfifo->cond);
        pthread_mutex_unlock(&zfifo->mutex);
        return n;
    }

    if (zfifo->end_offset > zfifo->first_offset)
    {
        if (zfifo->end_offset + all_len <= zfifo->buf_size)
        {
            n = zfifo_wirte_node(zfifo->buf + zfifo->end_offset, iov, iovcnt);
            zfifo->last_index++;
            zfifo->last_offset = zfifo->end_offset;
            if (flag)
            {
                zfifo->flag_index = zfifo->last_index;
                zfifo->flag_offset = zfifo->last_offset;
            }
            zfifo->end_offset += n;
            /* add 2015.9.15 */
            if (zfifo->end_offset > zfifo->deadline_offset)
                zfifo->deadline_offset = zfifo->end_offset;
        }
        else
        {
            zfifo->deadline_offset = zfifo->end_offset;

            while ((zfifo->first_offset < zfifo->deadline_offset) &&
                   (zfifo->first_offset < all_len))
            {
                n = zfifo_del_node(zfifo->buf + zfifo->first_offset, iovcnt);
                zfifo->first_index++;
                zfifo->first_offset += n;
            }
            if (zfifo->first_offset == zfifo->deadline_offset)
                zfifo->first_offset = 0;

            n = zfifo_wirte_node(zfifo->buf + 0, iov, iovcnt);
            zfifo->last_index++;
            zfifo->last_offset = 0;
            if (flag)
            {
                zfifo->flag_index = zfifo->last_index;
                zfifo->flag_offset = zfifo->last_offset;
            }
            zfifo->end_offset = n;
            if (zfifo->end_offset > zfifo->buf_size)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "write offset:%d, deadline:%d buf size:%d\n", zfifo->end_offset, zfifo->deadline_offset, zfifo->buf_size);
                pthread_mutex_unlock(&zfifo->mutex);
                return -1;
            }
        }
    }
    else
    {
        if (zfifo->end_offset + all_len <= zfifo->buf_size)
        {
            while ((zfifo->first_offset < zfifo->deadline_offset) &&
                   (zfifo->first_offset - zfifo->end_offset < all_len))
            {
                n = zfifo_del_node(zfifo->buf + zfifo->first_offset, iovcnt);
                zfifo->first_index++;
                zfifo->first_offset += n;
            }
            if (zfifo->first_offset == zfifo->deadline_offset)
            {
                zfifo->first_offset = 0;
                n = zfifo_wirte_node(zfifo->buf + zfifo->end_offset, iov, iovcnt);
                zfifo->last_index++;
                zfifo->last_offset = zfifo->end_offset;
                if (flag)
                {
                    zfifo->flag_index = zfifo->last_index;
                    zfifo->flag_offset = zfifo->last_offset;
                }
                zfifo->end_offset += n;
                if (zfifo->end_offset > zfifo->deadline_offset)
                    zfifo->deadline_offset = zfifo->end_offset;
            }
            else
            {
                n = zfifo_wirte_node(zfifo->buf + zfifo->end_offset, iov, iovcnt);
                zfifo->last_index++;
                zfifo->last_offset = zfifo->end_offset;
                if (flag)
                {
                    zfifo->flag_index = zfifo->last_index;
                    zfifo->flag_offset = zfifo->last_offset;
                }
                zfifo->end_offset += n;
            }

            if (zfifo->end_offset > zfifo->buf_size)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "write offset:%d, deadline:%d buf size:%d\n", zfifo->end_offset, zfifo->deadline_offset, zfifo->buf_size);
                pthread_mutex_unlock(&zfifo->mutex);
                return -1;
            }
        }
        else
        {
            while (zfifo->first_offset < zfifo->deadline_offset)
            {
                n = zfifo_del_node(zfifo->buf + zfifo->first_offset, iovcnt);
                zfifo->first_index++;
                zfifo->first_offset += n;
            }
            zfifo->first_offset = 0;

            zfifo->deadline_offset = zfifo->end_offset;

            while ((zfifo->first_offset < zfifo->deadline_offset) &&
                   (zfifo->first_offset < all_len))
            {
                n = zfifo_del_node(zfifo->buf + zfifo->first_offset, iovcnt);
                zfifo->first_index++;
                zfifo->first_offset += n;
            }
            if (zfifo->first_offset == zfifo->deadline_offset)
                zfifo->first_offset = 0;

            n = zfifo_wirte_node(zfifo->buf + 0, iov, iovcnt);
            zfifo->last_index++;
            zfifo->last_offset = 0;
            if (flag)
            {
                zfifo->flag_index = zfifo->last_index;
                zfifo->flag_offset = zfifo->last_offset;
            }
            zfifo->end_offset = n;
            if (zfifo->end_offset > zfifo->buf_size)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "write offset:%d, deadline:%d buf size:%d\n", zfifo->end_offset, zfifo->deadline_offset, zfifo->buf_size);
                pthread_mutex_unlock(&zfifo->mutex);
                return -1;
            }
        }
    }

    pthread_cond_broadcast(&zfifo->cond);
    pthread_mutex_unlock(&zfifo->mutex);

    return n;
}

#define MICROSECONDS (1000 * 1000)
int zfifo_readv(ZFIFO_DESC *zfifo_desc, ZFIFO_NODE *iov, int iovcnt, int timeout)
{
    ZFIFO *zfifo = zfifo_desc->zfifo;
    int n;

    if (timeout == 0)
    {
        timeout = 1000;
    }

    struct timeval now;
    struct timeval tv;
    struct timespec ts;

    pthread_mutex_lock(&zfifo->mutex);

    gettimeofday(&now, NULL);
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    ts.tv_sec = now.tv_sec + tv.tv_sec + (now.tv_usec + tv.tv_usec) / MICROSECONDS;
    ts.tv_nsec = ((now.tv_usec + tv.tv_usec) % MICROSECONDS) * 1000;

    int ret = 0;
    while ((zfifo_desc->index > zfifo->last_index) && (ret != ETIMEDOUT))
    {
        ret = pthread_cond_timedwait(&zfifo->cond, &zfifo->mutex, &ts);
    }
    if (ret == ETIMEDOUT)
    {
        /* timeout occurred */
        pthread_mutex_unlock(&zfifo->mutex);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "Read timeout, desc:%p\n", zfifo_desc);
        return 0;
    }

    if (zfifo_desc->index <= zfifo->first_index)
    {
        if (zfifo_desc->index < zfifo->first_index)
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "ziffo name:%s,lost data: cur_index = %d, first_index = %d, read desc:0x%p\n", zfifo->name, zfifo_desc->index, zfifo->first_index, zfifo_desc);
        zfifo_desc->index = zfifo->first_index;
        zfifo_desc->offset = zfifo->first_offset;
        // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "1, read_index:%d read_offset:%d first_index:%d first_offset:%d read desc:0x%x\n", zfifo_desc->index, zfifo_desc->offset, zfifo->first_index, zfifo->first_offset, (int)zfifo_desc);
    }
    // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "1.1, read_index:%d read_offset:%d first_index:%d first_offset:%d read desc:0x%x\n", zfifo_desc->index, zfifo_desc->offset, zfifo->first_index, zfifo->first_offset, (int)zfifo_desc);

    if ((zfifo_desc->index <= zfifo->last_index) &&
        (zfifo_desc->index >= zfifo->first_index))
    {
        if (zfifo_desc->offset == zfifo->deadline_offset)
            zfifo_desc->offset = 0;

        n = zfifo_read_node(zfifo->buf + zfifo_desc->offset, iov, iovcnt);
        if (n < 0)
        {
            pthread_mutex_unlock(&zfifo->mutex);
            return -1;
        }

        zfifo_desc->index++;
        zfifo_desc->offset += n;
        if (zfifo_desc->offset > zfifo->buf_size)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "read offset:%d, deadline:%d buf size:%d read desc:0x%p\n", zfifo_desc->offset, zfifo->deadline_offset, zfifo->buf_size, zfifo_desc);
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "read index:%d, first index:%d, last_index:%d read desc:0x%p\n", zfifo_desc->index, zfifo->first_index, zfifo->last_index, zfifo_desc);
            pthread_mutex_unlock(&zfifo->mutex);
            return -1;
        }
    }
    else
    {
        n = 0;
    }

    pthread_mutex_unlock(&zfifo->mutex);
    // usleep(500000);
    return n;
}

int zfifo_readv_plus(ZFIFO_DESC *zfifo_desc, ZFIFO_NODE *iov, int iovcnt, int timeout)
{
    ZFIFO *zfifo = zfifo_desc->zfifo;
    int n;

    struct timeval now;
    struct timeval tv;
    struct timespec ts;

    pthread_mutex_lock(&zfifo->mutex);

    gettimeofday(&now, NULL);
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    ts.tv_sec = now.tv_sec + tv.tv_sec + (now.tv_usec + tv.tv_usec) / MICROSECONDS;
    ts.tv_nsec = ((now.tv_usec + tv.tv_usec) % MICROSECONDS) * 1000;

    int ret = 0;

    while (((zfifo_desc->index > zfifo->last_index)) &&
           (ret != ETIMEDOUT))
    {
        if(timeout <= 0)
        {
            pthread_mutex_unlock(&zfifo->mutex);
            return 0;
        }
        ret = pthread_cond_timedwait(&zfifo->cond, &zfifo->mutex, &ts);
    }

    if (ret == ETIMEDOUT)
    {
        /* timeout occurred */
        pthread_mutex_unlock(&zfifo->mutex);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "Read timeout, desc:%p\n", zfifo_desc);
        return 0;
    }

    if (zfifo_desc->index <= zfifo->first_index)
    {
        if (zfifo_desc->index < zfifo->first_index)
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "lost data: cur_index = %d, first_index = %d, read desc:0x%p\n", zfifo_desc->index, zfifo->first_index, zfifo_desc);

        zfifo_desc->index = zfifo->first_index;
        zfifo_desc->offset = zfifo->first_offset;

        // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "1, read_index:%d read_offset:%d first_index:%d first_offset:%d read desc:0x%x\n", zfifo_desc->index, zfifo_desc->offset, zfifo->first_index, zfifo->first_offset, (int)zfifo_desc);
    }
    // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "1.1, read_index:%d read_offset:%d first_index:%d first_offset:%d read desc:0x%x\n", zfifo_desc->index, zfifo_desc->offset, zfifo->first_index, zfifo->first_offset, (int)zfifo_desc);

    if ((zfifo_desc->index <= zfifo->last_index) &&
        (zfifo_desc->index >= zfifo->first_index))
    {
        if (zfifo_desc->offset == zfifo->deadline_offset)
            zfifo_desc->offset = 0;

        n = zfifo_read_node(zfifo->buf + zfifo_desc->offset, iov, iovcnt);
        if (n < 0)
        {
            pthread_mutex_unlock(&zfifo->mutex);
            return -1;
        }

        zfifo_desc->index++;
        zfifo_desc->offset += n;
        if (zfifo_desc->offset > zfifo->buf_size)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "read offset:%d, deadline:%d buf size:%d read desc:0x%p\n", zfifo_desc->offset, zfifo->deadline_offset, zfifo->buf_size, zfifo_desc);
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "read index:%d, first index:%d, last_index:%d read desc:0x%p\n", zfifo_desc->index, zfifo->first_index, zfifo->last_index, zfifo_desc);
            pthread_mutex_unlock(&zfifo->mutex);
            return -1;
        }
    }
    else
    {
        n = 0;
    }

    pthread_mutex_unlock(&zfifo->mutex);
    // usleep(500000);
    return n;
}

int zfifo_readv_flag_plus(ZFIFO_DESC *zfifo_desc, ZFIFO_NODE *iov, int iovcnt, int timeout)
{
    ZFIFO *zfifo = zfifo_desc->zfifo;
    int n;

    struct timeval now;
    struct timeval tv;
    struct timespec ts;

    if (zfifo->flag_index > 0)
    {
        zfifo_desc->index = zfifo->flag_index;
        zfifo_desc->offset = zfifo->flag_offset;
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Invalid not key fps %d\n", zfifo->flag_index);
        return -1;
    }

    pthread_mutex_lock(&zfifo->mutex);

    gettimeofday(&now, NULL);
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    ts.tv_sec = now.tv_sec + tv.tv_sec + (now.tv_usec + tv.tv_usec) / MICROSECONDS;
    ts.tv_nsec = ((now.tv_usec + tv.tv_usec) % MICROSECONDS) * 1000;

    int ret = 0;

    while (((zfifo_desc->index > zfifo->last_index) || (zfifo->flag_index < zfifo->first_index)) &&
           (ret != ETIMEDOUT))
    {
        if(timeout <= 0)
        {
            pthread_mutex_unlock(&zfifo->mutex);
            return 0;
        }
        ret = pthread_cond_timedwait(&zfifo->cond, &zfifo->mutex, &ts);
    }
    if (ret == ETIMEDOUT)
    {
        /* timeout occurred */
        pthread_mutex_unlock(&zfifo->mutex);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "Read timeout, desc:%p\n", zfifo_desc);
        return 0;
    }

    if (zfifo_desc->index <= zfifo->first_index)
    {
        if (zfifo_desc->index < zfifo->first_index)
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "lost data: cur_index = %d, first_index = %d, read desc:0x%p\n", zfifo_desc->index, zfifo->first_index, zfifo_desc);
        zfifo_desc->index = zfifo->flag_index;
        zfifo_desc->offset = zfifo->flag_offset;
        // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "1, read_index:%d read_offset:%d first_index:%d first_offset:%d read desc:0x%x\n", zfifo_desc->index, zfifo_desc->offset, zfifo->first_index, zfifo->first_offset, (int)zfifo_desc);
    }
    // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "1.1, read_index:%d read_offset:%d first_index:%d first_offset:%d read desc:0x%x\n", zfifo_desc->index, zfifo_desc->offset, zfifo->first_index, zfifo->first_offset, (int)zfifo_desc);

    if ((zfifo_desc->index <= zfifo->last_index) &&
        (zfifo_desc->index >= zfifo->first_index))
    {
        // APP_PROF_LOG_PRINT(LEVEL_DEBUG, "2.2, read_index:%d read_offset:%d first_index:%d first_offset:%d d_offset:%d read desc:0x%x\n", zfifo_desc->index, zfifo_desc->offset, zfifo->first_index, zfifo->first_offset, zfifo->deadline_offset, (int)zfifo_desc);
        if (zfifo_desc->offset == zfifo->deadline_offset)
            zfifo_desc->offset = 0;

        n = zfifo_read_node(zfifo->buf + zfifo_desc->offset, iov, iovcnt);
        if (n < 0)
        {
            pthread_mutex_unlock(&zfifo->mutex);
            return -1;
        }

        zfifo_desc->index++;
        zfifo_desc->offset += n;
        if (zfifo_desc->offset > zfifo->buf_size)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "read offset:%d, deadline:%d buf size:%d read desc:0x%p\n", zfifo_desc->offset, zfifo->deadline_offset, zfifo->buf_size, zfifo_desc);
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "read index:%d, first index:%d, last_index:%d read desc:0x%p\n", zfifo_desc->index, zfifo->first_index, zfifo->last_index, zfifo_desc);
            pthread_mutex_unlock(&zfifo->mutex);
            return -1;
        }
    }
    else
    {
        n = 0;
    }

    pthread_mutex_unlock(&zfifo->mutex);
    // usleep(500000);
    return n;
}

void zfifo_print(ZFIFO_NODE *iov, int iovcnt)
{
    NODE_HEADER *h = iov[0].base;
    printf("read, len = %d, index = %d type = %d size = %d\n", iov[0].len, h->index, h->type, h->size);
    printf("read, len = %d, str = %s \n", iov[1].len, (char *)iov[1].base);
}

void printbuf(void *buf, int size)
{
    int i;
    unsigned char *p = buf;
    printf("buf: ");
    for (i = 0; i < size; i++)
    {
        if (i % 10 == 0)
            printf("\n");
        printf("%02x ", p[i]);
    }
    printf("\n");
}

int zfifo_set_newest_frame(ZFIFO_DESC *zfifo_desc)
{
    if (zfifo_desc == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param error.\n");
        return -1;
    }

    ZFIFO *zfifo = zfifo_desc->zfifo;
    pthread_mutex_lock(&zfifo->mutex);
    if (zfifo->last_index != 0)
    {
        zfifo_desc->index = zfifo->last_index;
        zfifo_desc->offset = zfifo->last_offset;
    }
    pthread_mutex_unlock(&zfifo->mutex);
    return 0;
}

int zfifo_set_oldest_frame(ZFIFO_DESC *zfifo_desc)
{
    if (zfifo_desc == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "param error.\n");
        return -1;
    }

    ZFIFO *zfifo = zfifo_desc->zfifo;
    pthread_mutex_lock(&zfifo->mutex);
    if (zfifo->last_index != 0)
    {
        zfifo_desc->index = zfifo->first_index;
        zfifo_desc->offset = zfifo->first_offset;
    }
    pthread_mutex_unlock(&zfifo->mutex);
    return 0;
}

#if 0
///////////////////////////////////////////////////////////////
void *write_pthread(void *arg)
{
    ZFIFO *zfifo = (ZFIFO *)arg;
    ZFIFO_DESC *wfd = zfifo_open(zfifo);

    ZFIFO_NODE iov[2];
    int n;
    int no = 0;

    char str1[] = "hello";
    NODE_HEADER h1 = {0};
    char str2[] = "haha";
    NODE_HEADER h2 = {0};


    while (1) {
        memset(iov, 0, sizeof(ZFIFO_NODE) * 2);

        if (no % 2 == 0) {
            memset(&h1, 0, sizeof(NODE_HEADER));
            h1.index = ++no;
            h1.type = 0xff;
            h1.size = strlen(str1) + 1;

            iov[0].base = &h1;
            iov[0].len = sizeof(NODE_HEADER);
            iov[1].base = str1;
            iov[1].len = strlen(str1) + 1;
        } else {
            memset(&h1, 0, sizeof(NODE_HEADER));
            h2.index = ++no;
            h2.type = 0xaa;
            h2.size = strlen(str2) + 1;

            iov[0].base = &h2;
            iov[0].len = sizeof(NODE_HEADER);
            iov[1].base = str2;
            iov[1].len = strlen(str2) + 1;
        }
        n = zfifo_writev(wfd, iov, 2);
        printf("writen %d \n", n);
        printbuf(zfifo->buf, zfifo->buf_size);
        printf("first_index: %d, last index:%d \n\n", zfifo->first_index, zfifo->last_index);

        sleep(2);
    }

    zfifo_close(wfd);
}

void *read_pthread(void *arg)
{
    ZFIFO *zfifo = (ZFIFO *)arg;

    ZFIFO_DESC *rfd = zfifo_open(zfifo);

    int n;
    char str[100] = {0};
    ZFIFO_NODE iov[2];
    NODE_HEADER h1 = {0};
    iov[0].base = &h1;
    iov[1].base = &str;

    while (1) {
        n = zfifo_readv(rfd, iov, 2, 0);

        printf("read %d \n", n);
        zfifo_print(iov, 2);

        sleep(3);
    }

    zfifo_close(rfd);
}


int main()
{
    ZFIFO *zfifo = zfifo_init("main_stream", 80);

    pthread_t t1;
    pthread_t t2;
    //pthread_t t3;

    pthread_create(&t1, NULL, write_pthread, (void *)zfifo);
    pthread_create(&t2, NULL, read_pthread, (void *)zfifo);
    //pthread_create(&t3, NULL, read_pthread, (void *)zfifo);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    //pthread_join(t3, NULL);

    zfifo_uninit(zfifo);

    return 0;
}
#endif
