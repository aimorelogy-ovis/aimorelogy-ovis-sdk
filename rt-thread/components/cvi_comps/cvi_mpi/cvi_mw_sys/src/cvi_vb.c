#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <queue.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cvi_mw_base.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "vb_ioctl.h"
#include "vb_uapi.h"

#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif

#define CHECK_VB_POOL_VALID_WEAK(x)                                        \
    do {                                                                   \
        if ((x) == VB_STATIC_POOLID)                                       \
            break;                                                         \
        if ((x) == VB_INVALID_POOLID)                                      \
            break;                                                         \
        if ((x) >= (u32MaxPoolCnt)) {                                      \
            CVI_TRACE_VB(CVI_DBG_ERR, " invalid VB Pool(%d)\n", x);        \
            return CVI_ERR_VB_ILLEGAL_PARAM;                               \
        }                                                                  \
        if (!isPoolInited(x)) {                                            \
            CVI_TRACE_VB(CVI_DBG_ERR, "VB_POOL(%d) isn't init yet.\n", x); \
            return CVI_ERR_VB_ILLEGAL_PARAM;                               \
        }                                                                  \
    } while (0)

#define CHECK_VB_POOL_VALID_STRONG(x)                                      \
    do {                                                                   \
        if ((x) >= (u32MaxPoolCnt)) {                                      \
            CVI_TRACE_VB(CVI_DBG_ERR, " invalid VB Pool(%d)\n", x);        \
            return CVI_ERR_VB_ILLEGAL_PARAM;                               \
        }                                                                  \
        if (!isPoolInited(x)) {                                            \
            CVI_TRACE_VB(CVI_DBG_ERR, "VB_POOL(%d) isn't init yet.\n", x); \
            return CVI_ERR_VB_ILLEGAL_PARAM;                               \
        }                                                                  \
    } while (0)

typedef struct _pool {
    CVI_U64 memBase;
    void *vmemBase;
    VB_POOL_CONFIG_S config;
    pthread_mutex_t lock;
} VB_POOL_S;

static CVI_U32 u32MaxPoolCnt;
static VB_POOL_S *pstVbPool;
static int vb_inited = CVI_FALSE;

static inline CVI_BOOL isPoolInited(VB_POOL Pool)
{
    return (pstVbPool[Pool].memBase == 0) ? CVI_FALSE : CVI_TRUE;
}

static CVI_S32 _update_vb_pool(struct cvi_vb_pool_cfg *cfg)
{
    MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbPool);

    VB_POOL poolId = cfg->pool_id;
    pthread_mutexattr_t ma;

    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&pstVbPool[poolId].lock, &ma);
    pthread_mutex_lock(&pstVbPool[poolId].lock);
    pstVbPool[poolId].memBase = cfg->mem_base;
    pstVbPool[poolId].config.u32BlkSize = cfg->blk_size;
    pstVbPool[poolId].config.u32BlkCnt = cfg->blk_cnt;
    pstVbPool[poolId].config.enRemapMode = cfg->remap_mode;
    strncpy(pstVbPool[poolId].config.acName, cfg->pool_name, MAX_VB_POOL_NAME_LEN - 1);
    pthread_mutex_unlock(&pstVbPool[poolId].lock);

    return CVI_SUCCESS;
}

VB_BLK CVI_VB_GetBlockwithID(VB_POOL Pool, CVI_U32 u32BlkSize, MOD_ID_E modId)
{
    CVI_TRACE_VB(CVI_DBG_ERR, "Pls use CVI_VB_GetBlock\n");
    UNUSED(Pool);
    UNUSED(u32BlkSize);
    UNUSED(modId);
    return 0;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
/* CVI_VB_GetBlock: acquice a vb_blk with specific size from pool.
 *
 * @param pool: the pool to acquice blk. if VB_INVALID_POOLID, go through common-pool to search.
 * @param u32BlkSize: the size of vb_blk to acquire.
 * @return: the vb_blk if available. otherwise, VB_INVALID_HANDLE.
 */
VB_BLK CVI_VB_GetBlock(VB_POOL Pool, CVI_U32 u32BlkSize)
{
    CVI_S32 s32Ret;
    struct cvi_vb_blk_cfg cfg;

    CHECK_VB_POOL_VALID_WEAK(Pool);

    memset(&cfg, 0, sizeof(cfg));
    cfg.pool_id = Pool;
    cfg.blk_size = u32BlkSize;
    s32Ret = vb_ioctl_get_block(&cfg);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_block fail, ret(%d)\n", s32Ret);
        return VB_INVALID_HANDLE;
    }
    return (VB_BLK)cfg.blk;
}

/* CVI_VB_ReleaseBlock: release a vb_blk.
 *
 * @param Block: the vb_blk going to be released.
 * @return: CVI_SUCCESS if success; others if fail.
 */
CVI_S32 CVI_VB_ReleaseBlock(VB_BLK Block)
{
    CVI_S32 s32Ret;

    s32Ret = vb_ioctl_release_block(Block);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_release_block fail, ret(%d)\n", s32Ret);
        return CVI_FAILURE;
    }
    return CVI_SUCCESS;
}

VB_BLK CVI_VB_PhysAddr2Handle(CVI_U64 u64PhyAddr)
{
    CVI_S32 s32Ret;
    struct cvi_vb_blk_info blk_info;

    memset(&blk_info, 0, sizeof(blk_info));
    blk_info.phy_addr = u64PhyAddr;
    s32Ret = vb_ioctl_phys_to_handle(&blk_info);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_phys_to_handle fail, ret(%d)\n", s32Ret);
        return VB_INVALID_HANDLE;
    }
    return (VB_BLK)blk_info.blk;
}

CVI_U64 CVI_VB_Handle2PhysAddr(VB_BLK Block)
{
    CVI_S32 s32Ret;
    struct cvi_vb_blk_info blk_info;

    memset(&blk_info, 0, sizeof(blk_info));
    blk_info.blk = Block;
    s32Ret = vb_ioctl_get_blk_info(&blk_info);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_blk_info fail, ret(%d)\n", s32Ret);
        return 0;
    }
    return (CVI_U64)blk_info.phy_addr;
}

VB_POOL CVI_VB_Handle2PoolId(VB_BLK Block)
{
    CVI_S32 s32Ret;
    struct cvi_vb_blk_info blk_info;

    memset(&blk_info, 0, sizeof(blk_info));
    blk_info.blk = Block;
    s32Ret = vb_ioctl_get_blk_info(&blk_info);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_blk_info fail, ret(%d)\n", s32Ret);
        return VB_INVALID_POOLID;
    }
    return (VB_POOL)blk_info.pool_id;
}

CVI_S32 CVI_VB_InquireUserCnt(VB_BLK Block, CVI_U32 *pCnt)
{
    CVI_S32 s32Ret;
    struct cvi_vb_blk_info blk_info;

    MOD_CHECK_NULL_PTR(CVI_ID_VB, pCnt);

    memset(&blk_info, 0, sizeof(blk_info));
    blk_info.blk = Block;
    s32Ret = vb_ioctl_get_blk_info(&blk_info);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_blk_info fail, ret(%d)\n", s32Ret);
        return CVI_FAILURE;
    }
    *pCnt = blk_info.usr_cnt;
    return CVI_SUCCESS;
}

CVI_S32 CVI_VB_Init(void)
{
    CVI_S32 s32Ret;
    struct cvi_vb_cfg cfg;
    CVI_U32 i;
    int expect = false;

    // Only init once until exit.
    if (!atomic_compare_exchange_strong(&vb_inited, &expect, true))
        return CVI_SUCCESS;

    if (u32MaxPoolCnt == 0) {
        s32Ret = vb_ioctl_get_pool_max_cnt(&u32MaxPoolCnt);
        if (s32Ret != CVI_SUCCESS) {
            CVI_TRACE_VB(CVI_DBG_ERR,
                         "vb_ioctl_get_pool_max_cnt fail, ret(%d)\n", s32Ret);
            return CVI_FAILURE;
        }
        pstVbPool = calloc(u32MaxPoolCnt, sizeof(VB_POOL_S));
        if (!pstVbPool) {
            CVI_TRACE_VB(CVI_DBG_ERR, "malloc fail.\n");
            u32MaxPoolCnt = 0;
            return CVI_ERR_VB_NOMEM;
        }
    }

    memset(&cfg, 0, sizeof(cfg));
    s32Ret = vb_ioctl_init(&cfg);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_init fail, ret(%d)\n", s32Ret);
        return CVI_FAILURE;
    }

    for (i = 0; i < cfg.comm_pool_cnt; ++i)
        _update_vb_pool(&cfg.comm_pool[i]);
    return CVI_SUCCESS;
}

CVI_S32 CVI_VB_Exit(void)
{
    CVI_S32 s32Ret;
    CVI_U32 i;
    int expect = true;

    // Only exit once.
    if (!atomic_compare_exchange_strong(&vb_inited, &expect, false))
        return CVI_SUCCESS;

    for (i = 0; i < u32MaxPoolCnt; ++i) {
        if (pstVbPool[i].vmemBase)
            CVI_VB_MunmapPool(i);
        pthread_mutex_destroy(&pstVbPool[i].lock);
    }

    free(pstVbPool);
    pstVbPool = NULL;
    u32MaxPoolCnt = 0;

    s32Ret = vb_ioctl_exit();
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_exit fail, ret(%d)\n", s32Ret);
        // base_dev_close();
        return CVI_FAILURE;
    }

    // base_dev_close();
    return CVI_SUCCESS;
}

VB_POOL CVI_VB_CreatePool(VB_POOL_CONFIG_S *pstVbPoolCfg)
{
    CVI_S32 s32Ret;
    struct cvi_vb_pool_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbPoolCfg);

    memset(&cfg, 0, sizeof(cfg));
    cfg.blk_size = pstVbPoolCfg->u32BlkSize;
    cfg.blk_cnt = pstVbPoolCfg->u32BlkCnt;
    cfg.remap_mode = pstVbPoolCfg->enRemapMode;
    strncpy(cfg.pool_name, pstVbPoolCfg->acName, VB_POOL_NAME_LEN - 1);

    s32Ret = vb_ioctl_create_pool(&cfg);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_create_pool fail, ret(%d)\n", s32Ret);
        return VB_INVALID_POOLID;
    }
    _update_vb_pool(&cfg);

    return (VB_POOL)cfg.pool_id;
}

CVI_S32 CVI_VB_DestroyPool(VB_POOL Pool)
{
    CVI_S32 s32Ret;

    CHECK_VB_POOL_VALID_STRONG(Pool);

    if (pstVbPool[Pool].vmemBase)
        CVI_VB_MunmapPool(Pool);
    pthread_mutex_destroy(&pstVbPool[Pool].lock);
    memset(&pstVbPool[Pool], 0, sizeof(pstVbPool[Pool]));

    s32Ret = vb_ioctl_destroy_pool(Pool);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_destroy_pool fail, ret(%d)\n", s32Ret);
        return CVI_FAILURE;
    }
    return CVI_SUCCESS;
}

CVI_S32 CVI_VB_SetConfig(const VB_CONFIG_S *pstVbConfig)
{
    CVI_S32 s32Ret;
    struct cvi_vb_cfg cfg;
    CVI_U32 i;

    MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbConfig);
    if (pstVbConfig->u32MaxPoolCnt > VB_COMM_POOL_MAX_CNT || pstVbConfig->u32MaxPoolCnt == 0) {
        CVI_TRACE_VB(CVI_DBG_ERR, "Invalid vb u32MaxPoolCnt(%d)\n",
                     pstVbConfig->u32MaxPoolCnt);
        return CVI_ERR_VB_ILLEGAL_PARAM;
    }

    memset(&cfg, 0, sizeof(cfg));
    cfg.comm_pool_cnt = pstVbConfig->u32MaxPoolCnt;
    for (i = 0; i < cfg.comm_pool_cnt; ++i) {
        cfg.comm_pool[i].blk_size = pstVbConfig->astCommPool[i].u32BlkSize;
        cfg.comm_pool[i].blk_cnt = pstVbConfig->astCommPool[i].u32BlkCnt;
        cfg.comm_pool[i].remap_mode = pstVbConfig->astCommPool[i].enRemapMode;
        strncpy(cfg.comm_pool[i].pool_name,
                pstVbConfig->astCommPool[i].acName, VB_POOL_NAME_LEN - 1);
    }
    s32Ret = vb_ioctl_set_config(&cfg);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_set_config fail, ret(%d)\n", s32Ret);
        return CVI_FAILURE;
    }
    return CVI_SUCCESS;
}

CVI_S32 CVI_VB_GetConfig(VB_CONFIG_S *pstVbConfig)
{
    CVI_S32 s32Ret;
    struct cvi_vb_cfg cfg;
    CVI_U32 i;

    MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbConfig);

    s32Ret = vb_ioctl_get_config(&cfg);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_config fail, ret(%d)\n", s32Ret);
        return CVI_FAILURE;
    }

    memset(pstVbConfig, 0, sizeof(*pstVbConfig));
    pstVbConfig->u32MaxPoolCnt = cfg.comm_pool_cnt;
    for (i = 0; i < cfg.comm_pool_cnt; ++i) {
        pstVbConfig->astCommPool[i].u32BlkSize = cfg.comm_pool[i].blk_size;
        pstVbConfig->astCommPool[i].u32BlkCnt = cfg.comm_pool[i].blk_cnt;
        pstVbConfig->astCommPool[i].enRemapMode = cfg.comm_pool[i].remap_mode;
        strncpy(pstVbConfig->astCommPool[i].acName, cfg.comm_pool[i].pool_name,
                MAX_VB_POOL_NAME_LEN - 1);
    }
    return CVI_SUCCESS;
}

CVI_S32 CVI_VB_InitModCommPool(VB_UID_E enVbUid)
{
    CVI_TRACE_VB(CVI_DBG_WARN, "VB_UID(%d) not supported yet.\n", enVbUid);
    return CVI_SUCCESS;
}

CVI_S32 CVI_VB_ExitModCommPool(VB_UID_E enVbUid)
{
    CVI_TRACE_VB(CVI_DBG_WARN, "VB_UID(%d) not supported yet.\n", enVbUid);
    return CVI_SUCCESS;
}

/* CVI_VB_MmapPool - mmap the whole pool to get virtual-address
 *
 * @param Pool: pool id
 * @return CVI_SUCCESS if success; others if fail
 */
CVI_S32 CVI_VB_MmapPool(VB_POOL Pool)
{
    return CVI_SUCCESS;
}

CVI_S32 CVI_VB_MunmapPool(VB_POOL Pool)
{
    return CVI_SUCCESS;
}

/* CVI_VB_GetBlockVirAddr - to get virtual-address of the Block
 *
 * @param Pool: pool id
 * @param Block: block id
 * @param ppVirAddr: virtual-address of the Block, cached if pool create with VB_REMAP_MODE_CACHED
 * @return CVI_SUCCESS if success; others if fail
 */
CVI_S32 CVI_VB_GetBlockVirAddr(VB_POOL Pool, VB_BLK Block, void **ppVirAddr)
{
    VB_POOL_S *pool;
    VB_POOL poolId;
    CVI_U64 phyAddr;

    MOD_CHECK_NULL_PTR(CVI_ID_VB, ppVirAddr);
    CHECK_VB_POOL_VALID_STRONG(Pool);

    pool = &pstVbPool[Pool];
    pthread_mutex_lock(&pool->lock);
    if (pool->vmemBase == 0) {
        CVI_TRACE_VB(CVI_DBG_ERR, "Blk's Pool isn't mmap yet.\n");
        pthread_mutex_unlock(&pool->lock);
        return CVI_ERR_VB_NOTREADY;
    }

    poolId = CVI_VB_Handle2PoolId(Block);
    if (poolId != Pool) {
        CVI_TRACE_VB(CVI_DBG_ERR, "Blk's Pool(%d) isn't given one(%d).\n", poolId, Pool);
        pthread_mutex_unlock(&pool->lock);
        return CVI_ERR_VB_ILLEGAL_PARAM;
    }

    phyAddr = CVI_VB_Handle2PhysAddr(Block);
    if (!phyAddr) {
        CVI_TRACE_VB(CVI_DBG_ERR, "phyAddr = 0.\n");
        pthread_mutex_unlock(&pool->lock);
        return CVI_ERR_VB_ILLEGAL_PARAM;
    }

    *ppVirAddr = pool->vmemBase + (phyAddr - pool->memBase);
    pthread_mutex_unlock(&pool->lock);
    return CVI_SUCCESS;
}

CVI_VOID CVI_VB_PrintPool(VB_POOL Pool)
{
    CVI_S32 s32Ret;

    s32Ret = vb_ioctl_print_pool(Pool);
    if (s32Ret != CVI_SUCCESS)
        CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_print_pool fail, ret(%d)\n", s32Ret);
}
