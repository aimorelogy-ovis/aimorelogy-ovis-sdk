#include "vb.h"
#include "base.h"
#include "cvi_base.h"
#include "cvi_comm_math.h"
#include "cvi_debug.h"
#include "cvi_type.h"
#include "cvi_hashmap.h"
#include "pthread.h"
#include "queue.h"
#include "sys.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vip_atomic.h>

#define VB_BASE_ADDR_ALIGN (0x1000)

CVI_U32 vb_max_pools = 512;
CVI_U32 vb_pool_max_blk = 128;

static struct cvi_vb_cfg stVbConfig;
static struct vb_pool *vbPool;
static int ref_count;
static struct pthread_mutex getVB_lock;
static struct pthread_mutex g_lock;
static Hashmap *vbHashmap;

#define CHECK_VB_HANDLE_NULL(x)                             \
    do {                                                    \
        if ((x) == NULL) {                                  \
            CVI_TRACE_VB(CVI_DBG_ERR, " NULL VB HANDLE\n"); \
            return -1;                                      \
        }                                                   \
    } while (0)

#define CHECK_VB_HANDLE_VALID(x)                               \
    do {                                                       \
        if ((x)->magic != CVI_VB_MAGIC) {                      \
            CVI_TRACE_VB(CVI_DBG_ERR, " invalid VB Handle\n"); \
            return -1;                                         \
        }                                                      \
    } while (0)

#define CHECK_VB_POOL_VALID_WEAK(x)                                        \
    do {                                                                   \
        if ((x) == VB_STATIC_POOLID)                                       \
            break;                                                         \
        if ((x) >= (vb_max_pools)) {                                       \
            CVI_TRACE_VB(CVI_DBG_ERR, " invalid VB Pool(%d)\n", x);        \
            return -1;                                                     \
        }                                                                  \
        if (!isPoolInited(x)) {                                            \
            CVI_TRACE_VB(CVI_DBG_ERR, "VB_POOL(%d) isn't init yet.\n", x); \
            return -1;                                                     \
        }                                                                  \
    } while (0)

#define CHECK_VB_POOL_VALID_STRONG(x)                                      \
    do {                                                                   \
        if ((x) >= (vb_max_pools)) {                                       \
            CVI_TRACE_VB(CVI_DBG_ERR, " invalid VB Pool(%d)\n", x);        \
            return -1;                                                     \
        }                                                                  \
        if (!isPoolInited(x)) {                                            \
            CVI_TRACE_VB(CVI_DBG_ERR, "VB_POOL(%d) isn't init yet.\n", x); \
            return -1;                                                     \
        }                                                                  \
    } while (0)

static inline bool isPoolInited(VB_POOL poolId)
{
    return (vbPool[poolId].memBase == 0) ? CVI_FALSE : CVI_TRUE;
}

static int _vb_hash_key(void *key)
{
    return (((uintptr_t)key) >> 10);
}

static bool _vb_hash_equals(void *keyA, void *keyB)
{
    return (keyA == keyB);
}

static bool _vb_hash_find(CVI_U64 u64PhyAddr, struct vb_s **vb, bool del)
{
    bool is_found = false;
    struct vb_s *obj;
    void *p = NULL;

    p = hashmapGet(vbHashmap, (void *)(uintptr_t)u64PhyAddr);
    if (p) {
        obj = (struct vb_s *)p;
        if (del)
            hashmapRemove(vbHashmap, (void *)(uintptr_t)u64PhyAddr);
        is_found = true;
    }

    if (is_found)
        *vb = obj;
    return is_found;
}

static bool _is_vb_all_released(void)
{
    CVI_U32 i;

    for (i = 0; i < vb_max_pools; ++i) {
        if (isPoolInited(i)) {
            if (FIFO_CAPACITY(&vbPool[i].freeList) != FIFO_SIZE(&vbPool[i].freeList)) {
                CVI_TRACE_VB(CVI_DBG_ERR, "pool(%d) blk has not been all released yet\n", i);
                return false;
            }
        }
    }
    return true;
}

bool _hash_print_cb(void *key, void *value, void *context)
{
    struct vb_s *p = value;
    VB_POOL *poolId = context;
    CVI_CHAR str[64] = {0};
    CVI_S32 s32Len = 0;

    UNUSED(key);
    if (poolId && (*poolId != p->vb_pool))
        return true;

    s32Len = snprintf(str, 63, "Pool[%d] vb paddr(%lx) usr_cnt(%d) /", p->vb_pool, p->phy_addr, (int)atomic_read(&(p->usr_cnt)));

    for (CVI_U32 i = 0; i < CVI_ID_BUTT; ++i) {
        if (p->mod_ids & BIT(i)) {
            s32Len += snprintf(str + s32Len, 63, "%s/", base_get_modname(i));
        }
    }
    CVI_TRACE_VB(CVI_DBG_NOTICE, "%s\n", str);
    return true;
}

static bool _hash_free_cb(void *key, void *value, void *context)
{
    struct vb_s *p = value;

    if (p->vb_pool == VB_STATIC_POOLID)
        base_ion_free(p->phy_addr);
    free(value);
    return true;
}

static CVI_S32 _vb_set_config(struct cvi_vb_cfg *vb_cfg)
{
    int i;

    if (vb_cfg->comm_pool_cnt > VB_COMM_POOL_MAX_CNT || vb_cfg->comm_pool_cnt == 0) {
        CVI_TRACE_VB(CVI_DBG_ERR, "Invalid comm_pool_cnt(%d)\n", vb_cfg->comm_pool_cnt);
        return -1;
    }

    for (i = 0; i < vb_cfg->comm_pool_cnt; ++i) {
        if (vb_cfg->comm_pool[i].blk_size == 0 || vb_cfg->comm_pool[i].blk_cnt == 0 || vb_cfg->comm_pool[i].blk_cnt > vb_pool_max_blk) {
            CVI_TRACE_VB(CVI_DBG_ERR, "Invalid pool cfg, pool(%d), blk_size(%d), blk_cnt(%d)\n",
                         i, vb_cfg->comm_pool[i].blk_size,
                         vb_cfg->comm_pool[i].blk_cnt);
            return -1;
        }
    }
    stVbConfig = *vb_cfg;

    return 0;
}

static void _vb_cleanup(void)
{
    int i;
    struct vb_pool *pstPool;
    struct vb_req *req, *req_tmp;

    // free vb pool
    for (i = 0; i < vb_max_pools; ++i) {
        if (isPoolInited(i)) {
            pstPool = &vbPool[i];
            pthread_mutex_lock(&pstPool->lock);
            FIFO_EXIT(&pstPool->freeList);
            base_ion_free(pstPool->memBase);
            pthread_mutex_unlock(&pstPool->lock);
            pthread_mutex_destroy(&pstPool->lock);
            // free reqQ
            pthread_mutex_lock(&pstPool->reqQ_lock);
            if (!STAILQ_EMPTY(&pstPool->reqQ)) {
                STAILQ_FOREACH_SAFE(req, &pstPool->reqQ, stailq, req_tmp)
                {
                    STAILQ_REMOVE(&pstPool->reqQ, req, vb_req, stailq);
                    free(req);
                }
            }
            pthread_mutex_unlock(&pstPool->reqQ_lock);
            pthread_mutex_destroy(&pstPool->reqQ_lock);
        }
    }
    free(vbPool);
    vbPool = NULL;

    // free vb blk
    hashmapForEach(vbHashmap, _hash_free_cb, NULL);
    hashmapFree(vbHashmap);

    pthread_mutex_destroy(&getVB_lock);
}

static CVI_S32 _vb_create_pool(struct cvi_vb_pool_cfg *config, bool isComm)
{
    CVI_U32 poolSize;
    struct vb_s *p;
    bool isCache;
    char ion_name[10];
    CVI_S32 ret, i;
    VB_POOL poolId = config->pool_id;
    void *ion_v = NULL;
    poolSize = config->blk_size * config->blk_cnt + VB_BASE_ADDR_ALIGN;
    isCache = (config->remap_mode == _VB_REMAP_MODE_CACHED);
    printf("poolSize = %d\n", poolSize);
    snprintf(ion_name, 10, "VbPool%d", poolId);
    ret = base_ion_alloc(&vbPool[poolId].memBase, &ion_v, ion_name, poolSize, isCache);
    if (ret) {
        CVI_TRACE_VB(CVI_DBG_ERR, "base_ion_alloc fail! ret(%d)\n", ret);
        return ret;
    }
    vbPool[poolId].memBaseAlign = ALIGN(vbPool[poolId].memBase, VB_BASE_ADDR_ALIGN);
    ion_v = ion_v + (vbPool[poolId].memBaseAlign - vbPool[poolId].memBase);
    config->mem_base = vbPool[poolId].memBaseAlign;

    STAILQ_INIT(&vbPool[poolId].reqQ);
    pthread_mutex_init(&vbPool[poolId].reqQ_lock, NULL);
    pthread_mutex_init(&vbPool[poolId].lock, NULL);
    pthread_mutex_lock(&vbPool[poolId].lock);
    vbPool[poolId].poolID = poolId;
    vbPool[poolId].ownerID = (isComm) ? POOL_OWNER_COMMON : POOL_OWNER_PRIVATE;
    vbPool[poolId].vmemBase = ion_v;
    vbPool[poolId].blk_cnt = config->blk_cnt;
    vbPool[poolId].blk_size = config->blk_size;
    vbPool[poolId].remap_mode = config->remap_mode;
    vbPool[poolId].bIsCommPool = isComm;
    vbPool[poolId].u32FreeBlkCnt = config->blk_cnt;
    vbPool[poolId].u32MinFreeBlkCnt = vbPool[poolId].u32FreeBlkCnt;
    if (strlen(config->pool_name) != 0)
        strncpy(vbPool[poolId].acPoolName, config->pool_name,
                sizeof(vbPool[poolId].acPoolName));
    else
        strncpy(vbPool[poolId].acPoolName, "vbpool", sizeof(vbPool[poolId].acPoolName));
    vbPool[poolId].acPoolName[VB_POOL_NAME_LEN - 1] = '\0';

    FIFO_INIT(&vbPool[poolId].freeList, vbPool[poolId].blk_cnt);
    for (i = 0; i < vbPool[poolId].blk_cnt; ++i) {
        p = malloc(sizeof(*p));
        p->phy_addr = vbPool[poolId].memBaseAlign + (i * vbPool[poolId].blk_size);
        p->vir_addr = vbPool[poolId].vmemBase + (p->phy_addr - vbPool[poolId].memBaseAlign);
        p->vb_pool = poolId;
        p->usr_cnt = 0;
        p->magic = CVI_VB_MAGIC;
        p->mod_ids = 0;
        p->external = false;
        FIFO_PUSH(&vbPool[poolId].freeList, p);

        hashmapPut(vbHashmap, (void *)(uintptr_t)p->phy_addr, p);
    }
    pthread_mutex_unlock(&vbPool[poolId].lock);

    return 0;
}

static void _vb_destroy_pool(VB_POOL poolId)
{
    struct vb_pool *pstPool = &vbPool[poolId];
    struct vb_s *vb, *tmp_vb;
    struct vb_req *req, *req_tmp;

    CVI_TRACE_VB(CVI_DBG_DEBUG, "vb destroy pool, pool[%d]: capacity(%d) size(%d).\n", poolId, FIFO_CAPACITY(&pstPool->freeList), FIFO_SIZE(&pstPool->freeList));
    if (FIFO_CAPACITY(&pstPool->freeList) != FIFO_SIZE(&pstPool->freeList)) {
        CVI_TRACE_VB(CVI_DBG_WARN, "pool(%d) blk should be all released before destroy pool\n", poolId);
        return;
    }

    pthread_mutex_lock(&pstPool->lock);
    while (!FIFO_EMPTY(&pstPool->freeList)) {
        FIFO_POP(&pstPool->freeList, &vb);
        _vb_hash_find(vb->phy_addr, &tmp_vb, true);
        free(vb);
    }
    FIFO_EXIT(&pstPool->freeList);
    base_ion_free(pstPool->memBase);
    pthread_mutex_unlock(&pstPool->lock);
    pthread_mutex_destroy(&pstPool->lock);

    // free reqQ
    pthread_mutex_lock(&pstPool->reqQ_lock);
    if (!STAILQ_EMPTY(&pstPool->reqQ)) {
        STAILQ_FOREACH_SAFE(req, &pstPool->reqQ, stailq, req_tmp)
        {
            STAILQ_REMOVE(&pstPool->reqQ, req, vb_req, stailq);
            free(req);
        }
    }
    pthread_mutex_unlock(&pstPool->reqQ_lock);
    pthread_mutex_destroy(&pstPool->reqQ_lock);

    memset(&vbPool[poolId], 0, sizeof(vbPool[poolId]));
}

static CVI_S32 _vb_init(void)
{
    CVI_U32 i;
    CVI_S32 ret;

    if (ref_count > 0) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb has already init\n");
        return 0;
    }

    pthread_mutex_init(&g_lock, NULL);
    pthread_mutex_lock(&g_lock);
    if (ref_count == 0) {
        vbHashmap = hashmapCreate(20, _vb_hash_key, _vb_hash_equals);
        vbPool = calloc(vb_max_pools, sizeof(struct vb_pool));
        if (!vbPool) {
            CVI_TRACE_VB(CVI_DBG_ERR, "vbPool kzalloc fail!\n");
            pthread_mutex_unlock(&g_lock);
            return -1;
        }

        for (i = 0; i < stVbConfig.comm_pool_cnt; ++i) {
            stVbConfig.comm_pool[i].pool_id = i;
            ret = _vb_create_pool(&stVbConfig.comm_pool[i], true);
            if (ret) {
                CVI_TRACE_VB(CVI_DBG_ERR, "_vb_create_pool fail, ret(%d)\n", ret);
                goto VB_INIT_FAIL;
            }
        }

        pthread_mutex_init(&getVB_lock, NULL);
        CVI_TRACE_VB(CVI_DBG_DEBUG, "_vb_init -\n");
    }
    ref_count++;
    pthread_mutex_unlock(&g_lock);
    return 0;

VB_INIT_FAIL:
    for (i = 0; i < stVbConfig.comm_pool_cnt; ++i) {
        if (isPoolInited(i))
            _vb_destroy_pool(i);
    }

    if (vbPool)
        free(vbPool);
    vbPool = NULL;
    pthread_mutex_unlock(&g_lock);
    return ret;
}

static CVI_S32 _vb_exit(void)
{
    int i;

    pthread_mutex_lock(&g_lock);
    if (ref_count == 0) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb has already exited\n");
        pthread_mutex_unlock(&g_lock);
        return 0;
    }

    if (--ref_count > 0) {
        pthread_mutex_unlock(&g_lock);
        return 0;
    }

    if (!_is_vb_all_released()) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb has not been all released\n");
        for (i = 0; i < vb_max_pools; ++i) {
            if (isPoolInited(i))
                hashmapForEach(vbHashmap, _hash_print_cb, &vbPool[i].poolID);
        }
    }

    _vb_cleanup();
    pthread_mutex_unlock(&g_lock);
    pthread_mutex_destroy(&g_lock);
    CVI_TRACE_VB(CVI_DBG_WARN, "_vb_exit -\n");

    return 0;
}

VB_POOL _find_vb_pool(CVI_U32 u32BlkSize)
{
    VB_POOL Pool = VB_INVALID_POOLID;
    int i;

    for (i = 0; i < VB_COMM_POOL_MAX_CNT; ++i) {
        if (!isPoolInited(i))
            continue;
        if (vbPool[i].ownerID != POOL_OWNER_COMMON)
            continue;
        if (u32BlkSize > vbPool[i].blk_size)
            continue;
        if ((Pool == VB_INVALID_POOLID) || (vbPool[Pool].blk_size > vbPool[i].blk_size))
            Pool = i;
    }
    return Pool;
}

static VB_BLK _vb_get_block_static(CVI_U32 u32BlkSize)
{
    CVI_S32 ret = CVI_SUCCESS;
    CVI_U64 phy_addr = 0;
    void *ion_v = NULL;

    // allocate with ion
    ret = base_ion_alloc(&phy_addr, &ion_v, "static_pool", u32BlkSize, true);
    if (ret) {
        CVI_TRACE_VB(CVI_DBG_ERR, "base_ion_alloc fail! ret(%d)\n", ret);
        return VB_INVALID_HANDLE;
    }

    return vb_create_block(phy_addr, ion_v, VB_STATIC_POOLID, false);
}

/* _vb_get_block: acquice a vb_blk with specific size from pool.
 *
 * @param pool: the pool to acquice blk.
 * @param u32BlkSize: the size of vb_blk to acquire.
 * @param modId: the Id of mod which acquire this blk
 * @return: the vb_blk if available. otherwise, VB_INVALID_HANDLE.
 */
static VB_BLK _vb_get_block(struct vb_pool *pool, CVI_U32 u32BlkSize, MOD_ID_E modId)
{
    struct vb_s *p;

    if (u32BlkSize > pool->blk_size) {
        CVI_TRACE_VB(CVI_DBG_ERR, "PoolID(%#x) blksize(%d) > pool's(%d).\n", pool->poolID, u32BlkSize, pool->blk_size);
        return VB_INVALID_HANDLE;
    }

    pthread_mutex_lock(&pool->lock);
    if (FIFO_EMPTY(&pool->freeList)) {
        CVI_TRACE_VB(CVI_DBG_DEBUG, "VB_POOL owner(%#x) poolID(%#x) pool is empty.\n",
                     pool->ownerID, pool->poolID);
        pthread_mutex_unlock(&pool->lock);
        hashmapForEach(vbHashmap, _hash_print_cb, &pool->poolID);
        return VB_INVALID_HANDLE;
    }

    FIFO_POP(&pool->freeList, &p);
    pool->u32FreeBlkCnt--;
    pool->u32MinFreeBlkCnt =
        (pool->u32FreeBlkCnt < pool->u32MinFreeBlkCnt) ? pool->u32FreeBlkCnt : pool->u32MinFreeBlkCnt;
    atomic_set(&p->usr_cnt, 1);
    p->mod_ids = BIT(modId);
    pthread_mutex_unlock(&pool->lock);
    // CVI_TRACE_VB(CVI_DBG_ERR, "Mod(%s) phy-addr(%#lx).\n", base_get_modname(modId), p->phy_addr);
    return (VB_BLK)p;
}

static CVI_S32 _vb_get_blk_info(struct cvi_vb_blk_info *blk_info)
{
    VB_BLK blk = (VB_BLK)blk_info->blk;
    struct vb_s *vb;

    vb = (struct vb_s *)blk;
    CHECK_VB_HANDLE_NULL(vb);
    CHECK_VB_HANDLE_VALID(vb);

    blk_info->phy_addr = vb->phy_addr;
    blk_info->pool_id = vb->vb_pool;
    blk_info->usr_cnt = vb->usr_cnt;
    return 0;
}
/**************************************************************************
 *   global APIs.
 **************************************************************************/
CVI_S32 vb_get_pool_info(struct vb_pool **pool_info)
{
    CHECK_VB_HANDLE_NULL(pool_info);
    CHECK_VB_HANDLE_NULL(vbPool);

    *pool_info = vbPool;
    return 0;
}

void vb_cleanup(void)
{
    pthread_mutex_lock(&g_lock);
    if (ref_count == 0) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb has already exited\n");
        pthread_mutex_unlock(&g_lock);
        return;
    }
    _vb_cleanup();
    ref_count = 0;
    pthread_mutex_unlock(&g_lock);
    CVI_TRACE_VB(CVI_DBG_ERR, "vb_cleanup done\n");
}

CVI_S32 vb_get_config(struct cvi_vb_cfg *pstVbConfig)
{
    if (!pstVbConfig) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_get_config NULL ptr!\n");
        return -1;
    }

    *pstVbConfig = stVbConfig;
    return 0;
}

CVI_S32 vb_create_pool(struct cvi_vb_pool_cfg *config)
{
    CVI_U32 i;
    CVI_S32 ret;

    config->pool_id = VB_INVALID_POOLID;
    if ((config->blk_size == 0) || (config->blk_cnt == 0) || (config->blk_cnt > vb_pool_max_blk)) {
        CVI_TRACE_VB(CVI_DBG_ERR, "Invalid pool cfg, blk_size(%d), blk_cnt(%d)\n",
                     config->blk_size, config->blk_cnt);
        return -1;
    }

    if (ref_count == 0) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb module hasn't inited yet.\n");
        return -1;
    }

    for (i = 0; i < vb_max_pools; ++i) {
        if (!isPoolInited(i))
            break;
    }
    if (i >= vb_max_pools) {
        CVI_TRACE_VB(CVI_DBG_ERR, "Exceed vb_max_pools cnt: %d\n", vb_max_pools);
        return -1;
    }

    config->pool_id = i;
    ret = _vb_create_pool(config, false);
    if (ret) {
        CVI_TRACE_VB(CVI_DBG_ERR, "_vb_create_pool fail, ret(%d)\n", ret);
        return ret;
    }
    return 0;
}

CVI_S32 vb_destroy_pool(VB_POOL poolId)
{
    CHECK_VB_POOL_VALID_STRONG(poolId);

    _vb_destroy_pool(poolId);
    return 0;
}

/* vb_create_block: create a vb blk per phy-addr given.
 *
 * @param phyAddr: phy-address of the buffer for this new vb.
 * @param virAddr: virtual-address of the buffer for this new vb.
 * @param VbPool: the pool of the vb belonging.
 * @param isExternal: if the buffer is not allocated by mmf
 */
VB_BLK vb_create_block(CVI_U64 phyAddr, void *virAddr, VB_POOL VbPool, bool isExternal)
{
    struct vb_s *p = NULL;

    p = malloc(sizeof(*p));
    if (!p) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vmalloc failed.\n");
        return VB_INVALID_HANDLE;
    }

    p->phy_addr = phyAddr;
    p->vir_addr = virAddr;
    p->vb_pool = VbPool;
    p->usr_cnt = 1;
    p->magic = CVI_VB_MAGIC;
    p->mod_ids = 0;
    p->external = isExternal;
    hashmapPut(vbHashmap, (void *)(uintptr_t)p->phy_addr, p);
    return (VB_BLK)p;
}

VB_BLK vb_get_block_with_id(VB_POOL poolId, CVI_U32 u32BlkSize, MOD_ID_E modId)
{
    VB_BLK blk = VB_INVALID_HANDLE;

    if (ref_count == 0) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb module hasn't inited yet.\n");
        return VB_INVALID_POOLID;
    }

    pthread_mutex_lock(&getVB_lock);
    // common pool
    if (poolId == VB_INVALID_POOLID) {
        poolId = _find_vb_pool(u32BlkSize);
        if (poolId == VB_INVALID_POOLID) {
            CVI_TRACE_VB(CVI_DBG_ERR, "No valid pool for size(%d).\n", u32BlkSize);
            goto get_vb_done;
        }
    } else if (poolId == VB_STATIC_POOLID) {
        blk = _vb_get_block_static(u32BlkSize); // need not mapping pool, allocate vb block directly
        goto get_vb_done;
    } else if (poolId >= vb_max_pools) {
        CVI_TRACE_VB(CVI_DBG_ERR, " invalid VB Pool(%d)\n", poolId);
        goto get_vb_done;
    } else {
        if (!isPoolInited(poolId)) {
            CVI_TRACE_VB(CVI_DBG_ERR, "VB_POOL(%d) isn't init yet.\n", poolId);
            goto get_vb_done;
        }

        if (u32BlkSize > vbPool[poolId].blk_size) {
            CVI_TRACE_VB(CVI_DBG_ERR, "required size(%d) > pool(%d)'s blk-size(%d).\n", u32BlkSize, poolId,
                         vbPool[poolId].blk_size);
            goto get_vb_done;
        }
    }
    blk = _vb_get_block(&vbPool[poolId], u32BlkSize, modId);

get_vb_done:
    pthread_mutex_unlock(&getVB_lock);
    return blk;
}

static CVI_S32 _vb_handle_acquire_block(struct vb_pool *pool)
{
    bool bReq = false;
    struct vb_req *req, *tmp;
    CVI_S32 result;

    pthread_mutex_lock(&pool->reqQ_lock);

    if (!STAILQ_EMPTY(&pool->reqQ)) {
        STAILQ_FOREACH_SAFE(req, &pool->reqQ, stailq, tmp)
        {
            if (req->VbPool != pool->poolID) {
                CVI_TRACE_VB(CVI_DBG_ERR, "reqpool(%d) but pool(%d)\n", req->VbPool, pool->poolID);
                continue;
            }
            CVI_TRACE_VB(CVI_DBG_DEBUG, "pool(%d) release, Try acquire vb for %s\n",
                         pool->poolID, base_get_modname(req->chn.enModId));
            STAILQ_REMOVE(&pool->reqQ, req, vb_req, stailq);
            bReq = true;
            break;
        }
    }

    if (bReq) {
        result = req->fp(req->chn);
        if (result) { // req->fp return fail
            // pthread_mutex_lock(&pool->reqQ_lock);
            STAILQ_INSERT_TAIL(&pool->reqQ, req, stailq);
            // pthread_mutex_unlock(&pool->reqQ_lock);
        } else
            free(req);
    }

    pthread_mutex_unlock(&pool->reqQ_lock);
    return result;
}

CVI_S32 vb_release_block(VB_BLK blk)
{
    struct vb_s *vb = (struct vb_s *)blk;
    struct vb_s *vb_tmp;
    struct vb_pool *pool;
    int cnt;

    CHECK_VB_HANDLE_NULL(vb);
    CHECK_VB_HANDLE_VALID(vb);
    CHECK_VB_POOL_VALID_WEAK(vb->vb_pool);

    pool = &vbPool[vb->vb_pool];
    pthread_mutex_lock(&pool->lock);
    cnt = atomic_read(&vb->usr_cnt);
    atomic_dec(&vb->usr_cnt);
    pthread_mutex_unlock(&pool->lock);

    if (cnt <= 1) {
        // CVI_TRACE_VB(CVI_DBG_ERR, "phy-addr(%#lx) release.\n", vb->phy_addr);

        if (vb->external) {
            CVI_TRACE_VB(CVI_DBG_ERR, "external buffer phy-addr(%#lx) release.\n", vb->phy_addr);
            _vb_hash_find(vb->phy_addr, &vb_tmp, true);
            free(vb);
            return 0;
        }

        // free VB_STATIC_POOLID
        if (vb->vb_pool == VB_STATIC_POOLID) {
            CVI_S32 ret = 0;

            ret = base_ion_free(vb->phy_addr);
            _vb_hash_find(vb->phy_addr, &vb_tmp, true);
            free(vb);
            return ret;
        }

        if (cnt == 0) {
            int i = 0;

            CVI_TRACE_VB(CVI_DBG_ERR, "vb usr_cnt is zero.\n");
            pool = &vbPool[vb->vb_pool];
            pthread_mutex_lock(&pool->lock);
            FIFO_FOREACH(vb_tmp, &pool->freeList, i)
            {
                if (vb_tmp->phy_addr == vb->phy_addr) {
                    atomic_set(&vb->usr_cnt, 0);
                    pthread_mutex_unlock(&pool->lock);
                    return 0;
                }
            }
            pthread_mutex_unlock(&pool->lock);
        }

        pool = &vbPool[vb->vb_pool];
        pthread_mutex_lock(&pool->lock);
        memset(&vb->buf, 0, sizeof(vb->buf));
        atomic_set(&vb->usr_cnt, 0);
        vb->mod_ids = 0;
        FIFO_PUSH(&pool->freeList, vb);
        ++pool->u32FreeBlkCnt;
        pthread_mutex_unlock(&pool->lock);

        _vb_handle_acquire_block(pool);
    }
    return 0;
}

VB_BLK vb_physAddr2Handle(CVI_U64 u64PhyAddr)
{
    struct vb_s *vb = NULL;

    if (!_vb_hash_find(u64PhyAddr, &vb, false)) {
        CVI_TRACE_VB(CVI_DBG_ERR, "Cannot find vb corresponding to phyAddr:%#llx\n", u64PhyAddr);
        return VB_INVALID_HANDLE;
    } else
        return (VB_BLK)vb;
}

CVI_U64 vb_handle2PhysAddr(VB_BLK blk)
{
    struct vb_s *vb = (struct vb_s *)blk;

    if ((vb == NULL) || (vb->magic != CVI_VB_MAGIC))
        return 0;
    return vb->phy_addr;
}

void *vb_handle2VirtAddr(VB_BLK blk)
{
    struct vb_s *vb = (struct vb_s *)blk;

    if ((vb == NULL) || (vb->magic != CVI_VB_MAGIC))
        return NULL;
    return vb->vir_addr;
}

VB_POOL vb_handle2PoolId(VB_BLK blk)
{
    struct vb_s *vb = (struct vb_s *)blk;

    if ((vb == NULL) || (vb->magic != CVI_VB_MAGIC))
        return VB_INVALID_POOLID;
    return vb->vb_pool;
}

CVI_S32 vb_inquireUserCnt(VB_BLK blk, CVI_U32 *pCnt)
{
    struct vb_s *vb = (struct vb_s *)blk;

    CHECK_VB_HANDLE_NULL(vb);
    CHECK_VB_HANDLE_VALID(vb);

    *pCnt = vb->usr_cnt;
    return 0;
}

/* vb_acquire_block: to register a callback to acquire vb_blk at CVI_VB_ReleaseBlock
 *                      in case of CVI_VB_GetBlock failure.
 *
 * @param fp: callback to acquire blk for module.
 * @param chn: info of the module which needs this helper.
 */
void vb_acquire_block(vb_acquire_fp fp, MMF_CHN_S chn,
                      CVI_U32 u32BlkSize, VB_POOL VbPool)
{
    if (VbPool == VB_INVALID_POOLID) {
        VbPool = _find_vb_pool(u32BlkSize);
        if (VbPool == VB_INVALID_POOLID) {
            CVI_TRACE_VB(CVI_DBG_ERR, "Not find pool for size(%d).\n", u32BlkSize);
            return;
        }
    }
    if (VbPool >= vb_max_pools) {
        CVI_TRACE_VB(CVI_DBG_ERR, "invalid VB Pool(%d)\n", VbPool);
        return;
    }
    if (!isPoolInited(VbPool)) {
        CVI_TRACE_VB(CVI_DBG_ERR, "VB_POOL(%d) isn't init yet.\n", VbPool);
        return;
    }

    struct vb_req *req = malloc(sizeof(*req));
    if (!req) {
        CVI_TRACE_VB(CVI_DBG_ERR, "VB req malloc failed.\n");
        return;
    }

    req->fp = fp;
    req->chn = chn;
    req->VbPool = VbPool;

    pthread_mutex_lock(&vbPool[VbPool].reqQ_lock);
    STAILQ_INSERT_TAIL(&vbPool[VbPool].reqQ, req, stailq);
    pthread_mutex_unlock(&vbPool[VbPool].reqQ_lock);

    _vb_handle_acquire_block(&vbPool[VbPool]);
}

void vb_cancel_block(MMF_CHN_S chn, CVI_U32 u32BlkSize, VB_POOL VbPool)
{
    struct vb_req *req, *tmp;

    if (VbPool == VB_INVALID_POOLID) {
        VbPool = _find_vb_pool(u32BlkSize);
        if (VbPool == VB_INVALID_POOLID) {
            CVI_TRACE_VB(CVI_DBG_ERR, "Not find pool for size(%d).\n", u32BlkSize);
            return;
        }
    }
    if (VbPool >= vb_max_pools) {
        CVI_TRACE_VB(CVI_DBG_ERR, "invalid VB Pool(%d)\n", VbPool);
        return;
    }
    if (!isPoolInited(VbPool)) {
        CVI_TRACE_VB(CVI_DBG_ERR, "VB_POOL(%d) isn't init yet.\n", VbPool);
        return;
    }

    pthread_mutex_lock(&vbPool[VbPool].reqQ_lock);
    if (!STAILQ_EMPTY(&vbPool[VbPool].reqQ)) {
        STAILQ_FOREACH_SAFE(req, &vbPool[VbPool].reqQ, stailq, tmp)
        {
            if (CHN_MATCH(&req->chn, &chn)) {
                STAILQ_REMOVE(&vbPool[VbPool].reqQ, req, vb_req, stailq);
                free(req);
            }
        }
    }
    pthread_mutex_unlock(&vbPool[VbPool].reqQ_lock);
}

CVI_S32 vb_ctrl(struct vb_ext_control *p)
{
    CVI_S32 ret = 0;
    u32 id = p->id;

    switch (id) {
    case VB_IOCTL_SET_CONFIG: {
        struct cvi_vb_cfg cfg;

        if (ref_count) {
            CVI_TRACE_VB(CVI_DBG_ERR, "vb has already inited, set_config cmd has no effect\n");
            break;
        }

        memcpy(&cfg, p->ptr, sizeof(struct cvi_vb_cfg));
        ret = _vb_set_config(&cfg);
        break;
    }

    case VB_IOCTL_GET_CONFIG: {
        memcpy(p->ptr, &stVbConfig, sizeof(struct cvi_vb_cfg));
        break;
    }

    case VB_IOCTL_INIT:
        ret = _vb_init();
        memcpy(p->ptr, &stVbConfig, sizeof(struct cvi_vb_cfg));
        break;

    case VB_IOCTL_EXIT:
        ret = _vb_exit();

        break;

    case VB_IOCTL_CREATE_POOL: {
        struct cvi_vb_pool_cfg cfg;

        memcpy(&cfg, p->ptr, sizeof(struct cvi_vb_pool_cfg));
        ret = vb_create_pool(&cfg);
        if (ret == 0)
            memcpy(p->ptr, &cfg, sizeof(struct cvi_vb_pool_cfg));

        break;
    }

    case VB_IOCTL_DESTROY_POOL: {
        VB_POOL pool;

        pool = (VB_POOL)p->value;
        ret = vb_destroy_pool(pool);
        break;
    }

    case VB_IOCTL_GET_BLOCK: {
        struct cvi_vb_blk_cfg cfg;
        VB_BLK block;

        memcpy(&cfg, p->ptr, sizeof(struct cvi_vb_blk_cfg));
        block = vb_get_block_with_id(cfg.pool_id, cfg.blk_size, CVI_ID_USER);
        if (block == VB_INVALID_HANDLE)
            ret = -1;
        else {
            cfg.blk = (CVI_U64)block;
            memcpy(p->ptr, &cfg, sizeof(struct cvi_vb_blk_cfg));
        }
        break;
    }

    case VB_IOCTL_RELEASE_BLOCK: {
        VB_BLK blk = (VB_BLK)p->value64;

        ret = vb_release_block(blk);
        break;
    }

    case VB_IOCTL_PHYS_TO_HANDLE: {
        struct cvi_vb_blk_info blk_info;
        VB_BLK block;

        memcpy(&blk_info, p->ptr, sizeof(struct cvi_vb_blk_info));

        block = vb_physAddr2Handle(blk_info.phy_addr);
        if (block == VB_INVALID_HANDLE)
            ret = -1;
        else {
            blk_info.blk = (CVI_U64)block;
            memcpy(p->ptr, &blk_info, sizeof(struct cvi_vb_blk_info));
        }
        break;
    }

    case VB_IOCTL_GET_BLK_INFO: {
        struct cvi_vb_blk_info blk_info;

        memcpy(&blk_info, p->ptr, sizeof(struct cvi_vb_blk_info));
        ret = _vb_get_blk_info(&blk_info);
        if (ret == 0)
            memcpy(p->ptr, &blk_info, sizeof(struct cvi_vb_blk_info));

        break;
    }

    case VB_IOCTL_GET_POOL_MAX_CNT: {
        p->value = (CVI_S32)vb_max_pools;
        break;
    }

    case VB_IOCTL_PRINT_POOL: {
        VB_POOL pool;

        pool = (VB_POOL)p->value;
        hashmapForEach(vbHashmap, _hash_print_cb, &pool);
        break;
    }

    default:
        break;
    }
    return ret;
}
