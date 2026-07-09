#include "jdi.h"

#include "jpuapifunc.h"
#include "jpulog.h"
#include "malloc.h"
#include "stdint.h"

#define JPU_BIT_REG_SIZE 0x1000
#define JPU_BIT_REG_BASE (0x0B000000)

#define JDI_DRAM_PHYSICAL_BASE 0x130000000
#define JDI_DRAM_PHYSICAL_SIZE 0x00800000

#define JDI_SYSTEM_ENDIAN JDI_LITTLE_ENDIAN
#ifndef UNREFERENCED_PARAM
#define UNREFERENCED_PARAM(x) ((void)(x))
#endif

typedef struct jpu_buffer_t jpudrv_buffer_t;

typedef struct jpu_buffer_pool_t
{
    jpudrv_buffer_t jdb;
    int             inuse;
    int             is_jpe;
} jpu_buffer_pool_t;

typedef struct
{
    int                  jpu_fd;
    jpu_instance_pool_t *pjip;
    int                  jip_size;
    int                  task_num;
    int                  bSingleEsBuf;
    int                  single_es_buf_size;
    int                  clock_state;
    jpudrv_buffer_t      jdb_video_memory;
    jpudrv_buffer_t      jdb_register;
    jpu_buffer_pool_t    jpu_buffer_pool[MAX_JPU_BUFFER_POOL];
    int                  jpu_buffer_pool_count;
#ifdef TRY_SEM_MUTEX
    void *jpu_mutex;
#endif

    jpu_buffer_t vbStream;
    int          enc_task_num;
} jdi_info_t;

extern void soc_dcache_invalid_range(unsigned long addr, uint32_t size);
extern void soc_dcache_clean_invalid_range(unsigned long addr, uint32_t size);

static jdi_info_t          s_jdi_info[MAX_NUM_JPU_CORE] = {0};
static jpu_instance_pool_t s_jip                        = {0};
static int                 jpu_chn_idx                  = -1;

static int jpu_swap_endian(unsigned char *data, int len, int endian);

void jpu_set_channel_num(int chnIdx)
{
    jpu_chn_idx = chnIdx;
}

int jdi_probe(void)
{
    int ret;
    ret = jdi_init();

    jdi_release();

    return ret;
}

int jdi_get_task_num(void)
{
    jdi_info_t *jdi = &s_jdi_info[0];
    return jdi->task_num;
}

int jdi_use_single_es_buffer(void)
{
    jdi_info_t *jdi = &s_jdi_info[0];
    return jdi->bSingleEsBuf;
}

int jdi_set_enc_task(int bSingleEsBuf, int *singleEsBufSize)
{
    jdi_info_t *jdi = &s_jdi_info[0];

    if (jdi->enc_task_num == 0)
    {
        jdi->bSingleEsBuf       = bSingleEsBuf;
        jdi->single_es_buf_size = *singleEsBufSize;
    }
    else
    {
        if (jdi->bSingleEsBuf != bSingleEsBuf)
        {
            JLOG(ERR,
                 "[JDI]all instance use single es buffer flag must be the same\n");
            return -1;
        }
    }

    if (jdi->bSingleEsBuf)
    {
        if (jdi->single_es_buf_size != *singleEsBufSize)
        {
            JLOG(WARN, "different single es buf size %d ignored\n", *singleEsBufSize);
            JLOG(WARN, "current single es buf size %d\n", jdi->single_es_buf_size);
            *singleEsBufSize = jdi->single_es_buf_size;
        }
    }

    jdi->enc_task_num++;

    return 0;
}

int jdi_delete_enc_task(void)
{
    jdi_info_t *jdi = &s_jdi_info[0];

    if (jdi->enc_task_num == 0)
    {
        JLOG(ERR, "[JDI]jdi_delete_enc_task == 0\n");
        return -1;
    }
    jdi->enc_task_num--;
    return 0;
}

int jdi_get_enc_task_num(void)
{
    jdi_info_t *jdi = &s_jdi_info[0];

    return jdi->enc_task_num;
}

int jdi_init(void)
{
    jdi_info_t *jdi = &s_jdi_info[0];
    int         ret;

    if (jdi->jpu_fd != -1 && jdi->jpu_fd != 0x00)
    {
        jdi->task_num++;
        return 0;
    }
    memset(jdi, 0, sizeof(jdi_info_t));

    jdi->jpu_fd = 1;

    memset((void *)&jdi->jpu_buffer_pool, 0x00,
           sizeof(jpu_buffer_pool_t) * MAX_JPU_BUFFER_POOL);
    jdi->jpu_buffer_pool_count = 0;

    jdi->pjip = jdi_get_instance_pool();
    if (!(jdi->pjip))
    {
        JLOG(ERR, "[JDI] fail to create instance pool for saving context\n");
        goto ERR_JDI_INIT;
    }

#ifdef TRY_SEM_MUTEX
    // it's not error if semaphore existing
    jdi->jpu_mutex = sem_open(JPU_SEM_NAME, OPEN_FLAG, OPEN_MODE, INIT_V);
    if (jdi->jpu_mutex == SEM_FAILED)
    {
        JLOG(ERR, "sem_open failed...\n");
        return -1;
    }
#endif

    if (!jdi->pjip->instance_pool_inited)
    {
#if defined(LIBCVIJPULITE)
        jdi->pjip->jpu_mutex = 0;
#endif
    }

#if defined(LIBCVIJPULITE)
    ret = jdi_lock(100);
#else
    // pthread_mutex_init((pthread_mutex_t *)&jdi->pjip->jpu_mutex, 0);
    ret = jdi_lock();
#endif
    if (ret < 0)
    {
        JLOG(ERR, "[JDI] fail to pthread_mutex_t lock function\n");
        goto ERR_JDI_INIT;
    }

    jdi->jdb_register.phys_addr = JPU_BIT_REG_BASE;
    jdi->jdb_register.virt_addr = (uint8_t *)JPU_BIT_REG_BASE;
    jdi->jdb_register.size      = JPU_BIT_REG_SIZE;
    jdi->task_num++;

    jdi_set_clock_gate(1);

    // JLOG(INFO, "[JDI] success to init driver \n");
    return jdi->jpu_fd;

ERR_JDI_INIT:
    jdi_unlock();
    jdi_release();
    return -1;
}

int jdi_release(void)
{
    jdi_info_t *jdi = &s_jdi_info[0];
    int         ret;
    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return 0;

#if defined(LIBCVIJPULITE)
    ret = jdi_lock(100);
#else
    ret = jdi_lock();
#endif
    if (ret < 0)
    {
        CVI_JPG_DBG_ERR("[JDI] fail to pthread_mutex_t lock function\n");
        return -1;
    }

    jdi->task_num--;
    // means that the opened instance remains
    if (jdi->task_num > 0)
    {
        CVI_JPG_DBG_INFO("s_task_num =%d", jdi->task_num);
        jdi_unlock();
        return 0;
    }

    jdi_set_clock_gate(0);
    memset(&jdi->jdb_register, 0x00, sizeof(jpudrv_buffer_t));

    jdi_unlock();

    jdi->pjip = NULL;

    if (jdi->jpu_fd != -1 && jdi->jpu_fd != 0x00) jdi->jpu_fd = -1;
#ifdef TRY_SEM_MUTEX
    if (sem_close((MUTEX_HANDLE *)jdi->jpu_mutex) < 0)
    {
        JLOG(ERR, "[JDI]  sem_close thread id = %lu\n", pthread_self());
    }
#endif

    memset(jdi, 0, sizeof(jdi_info_t));
    return 0;
}

jpu_instance_pool_t *jdi_get_instance_pool(void)
{
    jdi_info_t *jdi = &s_jdi_info[0];

    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return NULL;

    if (sizeof(JpgInst) > MAX_INST_HANDLE_SIZE)
    {
        CVI_JPG_DBG_ERR("JpgInst = %d, MAX_INST_HANDLE_SIZE = %d\n",
                        (int)sizeof(JpgInst), MAX_INST_HANDLE_SIZE);
    }

    if (jdi->pjip) return (jpu_instance_pool_t *)jdi->pjip;

    jdi->jip_size = sizeof(jpu_instance_pool_t);
    jdi->pjip     = &s_jip;

    return (jpu_instance_pool_t *)jdi->pjip;
}

int jdi_open_instance(unsigned long instIdx)
{
    jdi_info_t *jdi;

    UNREFERENCED_PARAM(instIdx);
    jdi = &s_jdi_info[0];
    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return -1;

    jdi->pjip->jpu_instance_num++;

    return 0;
}

int jdi_close_instance(unsigned long instIdx)
{
    jdi_info_t *jdi;

    UNREFERENCED_PARAM(instIdx);
    jdi = &s_jdi_info[0];
    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return -1;

    jdi->pjip->jpu_instance_num--;

    return 0;
}

int jdi_get_instance_num(void)
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info[0];
    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return -1;

    return jdi->pjip->jpu_instance_num;
}

int jdi_hw_reset(void)
{
    jdi_info_t *jdi = &s_jdi_info[0];

    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return -1;

    // to do any action for hw reset

    return 0;
}

#if defined(LIBCVIJPULITE)
int jdi_lock(int sleep_us)
#else
int jdi_lock(void)
#endif
{
#if defined(LIBCVIJPULITE)

    jdi_info_t     *jdi = &s_jdi_info[0];
    struct timespec req = {0, sleep_us * 1000};

    while (!__sync_bool_compare_and_swap(&(jdi->pjip->jpu_mutex), 0, 1))
    {
        int ret = nanosleep(&req, NULL);
        /* interrupted by a signal handler */
        if (ret < 0)
        {
            jdi->pjip->jpu_mutex = 0;
            return -1;
        }
    }
#endif

    return 0;
}

void jdi_unlock(void)
{
#if defined(LIBCVIJPULITE)
    jdi_info_t *jdi      = &s_jdi_info[0];
    jdi->pjip->jpu_mutex = 0;
#endif
}

void jdi_write_register(unsigned int addr, unsigned int data)
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info[0];
    unsigned int *reg_addr;

    if (!jdi->pjip || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return;

    reg_addr                           = (unsigned int *)(addr + jdi->jdb_register.virt_addr);
    *(volatile unsigned int *)reg_addr = data;
}

unsigned int jdi_read_register(unsigned int addr)
{
    unsigned int *reg_addr;
    jdi_info_t   *jdi;

    jdi = &s_jdi_info[0];

    reg_addr = (unsigned int *)(addr + jdi->jdb_register.virt_addr);
    return *(volatile unsigned int *)reg_addr;
}

int jdi_write_memory(unsigned long addr, unsigned char *data, int len,
                     int endian)
{
    jpudrv_buffer_t jdb;
    unsigned long   offset;
    int             i;

    jdi_info_t *jdi;

    jdi = &s_jdi_info[0];
    if (!jdi->pjip || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return -1;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    for (i = 0; i < MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 1)
        {
            jdb = jdi->jpu_buffer_pool[i].jdb;
            if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size)) break;
        }
    }

    if (!jdb.size)
    {
        JLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
        return -1;
    }

    offset = addr - (unsigned long)jdb.phys_addr;

    jpu_swap_endian(data, len, endian);
    memcpy((void *)((unsigned long)jdb.virt_addr + offset), data, len);

    return len;
}

int jdi_read_memory(unsigned long addr, unsigned char *data, int len,
                    int endian)
{
    jpudrv_buffer_t jdb;
    unsigned long   offset;
    int             i;
    jdi_info_t     *jdi;

    jdi = &s_jdi_info[0];
    if (!jdi->pjip || jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return -1;

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    for (i = 0; i < MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 1)
        {
            jdb = jdi->jpu_buffer_pool[i].jdb;
            if (addr >= jdb.phys_addr && addr < (jdb.phys_addr + jdb.size)) break;
        }
    }

    if (!jdb.size) return -1;

    offset = addr - (unsigned long)jdb.phys_addr;

    memcpy(data, (const void *)((unsigned long)jdb.virt_addr + offset), len);
    jpu_swap_endian(data, len, endian);

    return len;
}

int jdi_get_allocated_memory(jpu_buffer_t *vb, int is_jpe)
{
    jdi_info_t      *jdi = &s_jdi_info[0];
    jpudrv_buffer_t *jdb;
    int              i;
    int              isFind = 0;

    if (!jdi->pjip)
    {
        JLOG(ERR, "Invalid handle! jdi->pjip is NULL!\n");
        return -1;
    }

    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
    {
        JLOG(ERR, "Invalid handle! jdi->jpu_fd=%d\n", jdi->jpu_fd);
        return -1;
    }

    for (i = 0; i < MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 1 && jdi->jpu_buffer_pool[i].is_jpe == is_jpe)
        {
            jdb = &jdi->jpu_buffer_pool[i].jdb;
            if (vb->size == jdb->size)
            {
                vb->size      = jdb->size;
                vb->base      = jdb->base;
                vb->phys_addr = jdb->phys_addr;
                vb->virt_addr = jdb->virt_addr;
                isFind        = 1;
                break;
            }
        }
    }
    if (isFind)
        return 0;
    else
        return -1;
}
#undef ALIGN
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

int jdi_invalidate_ion_cache(uint64_t u64PhyAddr, void *pVirAddr,
                             uint32_t u32Len)
{
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, (void *)u64PhyAddr, u32Len);
    return 0;
}

int jdi_flush_ion_cache(uint64_t u64PhyAddr, void *pVirAddr, uint32_t u32Len)
{
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)u64PhyAddr, u32Len);
    return 0;
}

int jdi_allocate_dma_memory(jpu_buffer_t *vb, int is_jpe)
{
    jdi_info_t     *jdi = &s_jdi_info[0];
    jpudrv_buffer_t jdb = {0};
    int             i;
    int             ret = 0;

    if (!jdi->pjip)
    {
        JLOG(ERR, "Invalid handle! jdi->pjip is NULL!");
        return -1;
    }

    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
    {
        JLOG(ERR, "Invalid handle! jdi->jpu_fd=%d", jdi->jpu_fd);
        return -1;
    }

#if defined(LIBCVIJPULITE)
    jdi_lock(100);
#else
    jdi_lock();
#endif

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));
    jdb.size      = vb->size + 64;
    jdb.base      = (uint64_t)calloc(1, jdb.size);
    jdb.phys_addr = ALIGN(jdb.base, 64);

    if (jdb.phys_addr == (unsigned long)-1)
    {
        JLOG(ERR, "Not enough memory. size=%d\n", vb->size);
        ret = -1;
        goto fail;
    }

    jdb.virt_addr = (uint8_t *)jdb.phys_addr;

    vb->phys_addr = jdb.phys_addr;
    vb->base      = jdb.base;
    vb->virt_addr = jdb.virt_addr;

    for (i = 0; i < MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].inuse == 0)
        {
            jdi->jpu_buffer_pool[i].jdb = jdb;
            jdi->jpu_buffer_pool_count++;
            jdi->jpu_buffer_pool[i].inuse  = 1;
            jdi->jpu_buffer_pool[i].is_jpe = is_jpe;
            break;
        }
    }

    jdi_flush_ion_cache(jdb.phys_addr, jdb.virt_addr, jdb.size);
    if (i >= MAX_JPU_BUFFER_POOL)
    {
        JLOG(ERR, "fail to find an unused buffer in pool!\n");
        memset(vb, 0x00, sizeof(jpu_buffer_t));
        ret = -1;
        goto fail;
    }

fail:
    jdi_unlock();

    return ret;
}

void jdi_free_dma_memory(jpu_buffer_t *vb)
{
    jdi_info_t     *jdi = &s_jdi_info[0];
    jpudrv_buffer_t jdb;
    int             i;

    if (!jdi->pjip)
    {
        JLOG(ERR, "Invalid handle! jdi->pjip is NULL!");
        return;
    }

    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00)
    {
        JLOG(ERR, "Invalid handle! jdi->jpu_fd=%d", jdi->jpu_fd);
        return;
    }

    if (vb->size == 0)
    {
        JLOG(ERR, "addr=0x%lx\n", vb->phys_addr);
        return;
    }

#if defined(LIBCVIJPULITE)
    jdi_lock(100);
#else
    jdi_lock();
#endif

    memset(&jdb, 0x00, sizeof(jpudrv_buffer_t));

    for (i = 0; i < MAX_JPU_BUFFER_POOL; i++)
    {
        if (jdi->jpu_buffer_pool[i].jdb.phys_addr == vb->phys_addr)
        {
            jdi->jpu_buffer_pool[i].inuse  = 0;
            jdi->jpu_buffer_pool[i].is_jpe = 0;
            jdi->jpu_buffer_pool_count--;
            jdb = jdi->jpu_buffer_pool[i].jdb;
            break;
        }
    }

    if (!jdb.size)
    {
        JLOG(ERR, "[JDI] invalid buffer to free address = 0x%lx\n", jdb.phys_addr);
        jdi_unlock();
        return;
    }

    free((void *)jdb.base);
    memset(vb, 0, sizeof(jpu_buffer_t));
    jdi_unlock();
}

int jdi_set_clock_gate(int enable)
{
    jdi_info_t *jdi = &s_jdi_info[0];
    // JLOG(INFO, "jdi set clk %d\n", enable);

    if (jdi->jpu_fd == -1 || jdi->jpu_fd == 0x00) return 0;

    jdi->clock_state = enable;

    return 0;
}

int jdi_get_clock_gate(void)
{
    jdi_info_t *jdi;

    jdi = &s_jdi_info[0];
    return jdi->clock_state;
}

int jdi_wait_interrupt(int timeout)
{
    UNREFERENCED_PARAM(timeout);
    // int count = 0;

    while (1)
    {
        if (jdi_read_register(MJPEG_PIC_STATUS_REG)) break;

        // Sleep(1);    // 1ms sec
        // if (count++ > timeout)
        //    return -1;
    }

    return 0;
}

void jdi_log(int cmd, int step)
{
    int i;

    switch (cmd)
    {
    case JDI_LOG_CMD_PICRUN:
        if (step == 1) //
            JLOG(INFO, "\n**PIC_RUN start\n");
        else
            JLOG(INFO, "\n**PIC_RUN end \n");
        break;
    }

    for (i = 0; i <= 0x238; i = i + 16)
    {
        JLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i,
             jdi_read_register(i), jdi_read_register(i + 4),
             jdi_read_register(i + 8), jdi_read_register(i + 0xc));
    }
}

int jpu_swap_endian(unsigned char *data, int len, int endian)
{
    unsigned long *p;
    unsigned long  v1, v2, v3;
    int            i;
    int            swap = 0;
    p                   = (unsigned long *)data;

    if (endian == JDI_SYSTEM_ENDIAN)
        swap = 0;
    else
        swap = 1;

    if (swap)
    {
        if (endian == JDI_LITTLE_ENDIAN || endian == JDI_BIG_ENDIAN)
        {
            for (i = 0; i < (len >> 2); i += 2)
            {
                v1        = p[i];
                v2        = (v1 >> 24) & 0xFF;
                v2       |= ((v1 >> 16) & 0xFF) << 8;
                v2       |= ((v1 >> 8) & 0xFF) << 16;
                v2       |= ((v1 >> 0) & 0xFF) << 24;
                v3        = v2;
                v1        = p[i + 1];
                v2        = (v1 >> 24) & 0xFF;
                v2       |= ((v1 >> 16) & 0xFF) << 8;
                v2       |= ((v1 >> 8) & 0xFF) << 16;
                v2       |= ((v1 >> 0) & 0xFF) << 24;
                p[i]      = v2;
                p[i + 1]  = v3;
            }
        }
        else
        {
            int sys_endian = JDI_SYSTEM_ENDIAN;
            int swap4byte  = 0;
            swap           = 0;
            if (endian == JDI_32BIT_LITTLE_ENDIAN)
            {
                if (sys_endian == JDI_BIG_ENDIAN)
                {
                    swap = 1;
                }
            }
            else
            {
                if (sys_endian == JDI_BIG_ENDIAN)
                {
                    swap4byte = 1;
                }
                else if (sys_endian == JDI_LITTLE_ENDIAN)
                {
                    swap4byte = 1;
                    swap      = 1;
                }
                else
                {
                    swap = 1;
                }
            }
            if (swap)
            {
                for (i = 0; i < (len >> 2); i++)
                {
                    v1    = p[i];
                    v2    = (v1 >> 24) & 0xFF;
                    v2   |= ((v1 >> 16) & 0xFF) << 8;
                    v2   |= ((v1 >> 8) & 0xFF) << 16;
                    v2   |= ((v1 >> 0) & 0xFF) << 24;
                    p[i]  = v2;
                }
            }
            if (swap4byte)
            {
                for (i = 0; i < (len >> 2); i += 2)
                {
                    v1       = p[i];
                    v2       = p[i + 1];
                    p[i]     = v2;
                    p[i + 1] = v1;
                }
            }
        }
    }
    return swap;
}
PhysicalAddress jdi_get_memory_addr_high(PhysicalAddress addr)
{
    PhysicalAddress ret = 0;

    return (ret << 32) | addr;
}
