#include "base.h"
#include "cvi_debug.h"
#include "cvi_efuse.h"
#include "cvi_errno.h"
#include "vb.h"
#include <base_cb.h>
#include <cvi_base_ctx.h>
#include <stdio.h>

#define BIND_NODE_MAXNUM 64

#ifndef TAILQ_FOREACH_SAFE
#define TAILQ_FOREACH_SAFE(var, head, field, tvar)        \
    for ((var) = TAILQ_FIRST((head));                     \
         (var) && ((tvar) = TAILQ_NEXT((var), field), 1); \
         (var) = (tvar))
#endif

#define CHN_MATCH(x, y) (((x)->enModId == (y)->enModId) && ((x)->s32DevId == (y)->s32DevId) && ((x)->s32ChnId == (y)->s32ChnId))

#define GENERATE_STRING(STRING) (#STRING),
static const char *const MOD_STRING[] = FOREACH_MOD(GENERATE_STRING);
struct bind_t {
    TAILQ_ENTRY(bind_t)
    tailq;
    BIND_NODE_S *node;
};
TAILQ_HEAD(bind_head, bind_t)
binds;
static pthread_mutex_t bind_lock;
struct _BIND_NODE_S bind_nodes[BIND_NODE_MAXNUM];
static struct base_m_cb_info base_m_cb[E_MODULE_BUTT];
const char *const CB_MOD_STR[] = CB_FOREACH_MOD(CB_GENERATE_STRING);

const char *base_get_modname(MOD_ID_E id)
{
    return (id < CVI_ID_BUTT) ? MOD_STRING[id] : "UNDEF";
}

int base_rm_module_cb(enum ENUM_MODULES_ID module_id)
{
    if (module_id < 0 || module_id >= E_MODULE_BUTT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "base rm cb error: wrong module_id\n");
        return -1;
    }

    base_m_cb[module_id].dev = NULL;
    base_m_cb[module_id].cb = NULL;

    return 0;
}

int base_reg_module_cb(struct base_m_cb_info *cb_info)
{
    if (!cb_info || !cb_info->dev || !cb_info->cb) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "base reg cb error: no data\n");
        return -1;
    }

    if (cb_info->module_id < 0 || cb_info->module_id >= E_MODULE_BUTT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "base reg cb error: wrong module_id\n");
        return -1;
    }

    base_m_cb[cb_info->module_id] = *cb_info;

    return 0;
}

int base_exe_module_cb(struct base_exe_m_cb *exe_cb)
{
    struct base_m_cb_info *cb_info;

    if (exe_cb->caller < 0 || exe_cb->caller >= E_MODULE_BUTT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "base exe cb error: wrong caller\n");
        return -1;
    }

    if (exe_cb->callee < 0 || exe_cb->callee >= E_MODULE_BUTT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "base exe cb error: wrong callee\n");
        return -1;
    }

    cb_info = &base_m_cb[exe_cb->callee];

    if (!cb_info->cb) {
        CVI_TRACE_SYS(CVI_DBG_INFO, "base exe cb error: cb of callee(%s) is null, caller(%s)\n",
                      IDTOSTR(exe_cb->callee), IDTOSTR(exe_cb->caller));
        return -1;
    }

    return cb_info->cb(cb_info->dev, exe_cb->caller, exe_cb->cmd_id, exe_cb->data);
}

void base_save_modules_cb(struct base_m_cb_info **sys_m_cb)
{
    *sys_m_cb = base_m_cb;
}

void base_ctx_release_bind(void)
{
    struct bind_t *item, *item_tmp;

    pthread_mutex_lock(&bind_lock);
    TAILQ_FOREACH_SAFE(item, &binds, tailq, item_tmp)
    {
        TAILQ_REMOVE(&binds, item, tailq);
        rt_free(item);
    }
    memset(bind_nodes, 0, sizeof(bind_nodes));
    pthread_mutex_unlock(&bind_lock);
}

CVI_S32 base_ctx_bind(MMF_CHN_S *pstSrcChn, MMF_CHN_S *pstDestChn)
{
    struct bind_t *item, *item_tmp;
    CVI_S32 ret = 0, i;

    CVI_TRACE_SYS(CVI_DBG_DEBUG, "src(mId=%d, dId=%d, cId=%d), dst(mId=%d, dId=%d, cId=%d)\n",
                  pstSrcChn->enModId, pstSrcChn->s32DevId, pstSrcChn->s32ChnId,
                  pstDestChn->enModId, pstDestChn->s32DevId, pstDestChn->s32ChnId);

    pthread_mutex_lock(&bind_lock);
    TAILQ_FOREACH_SAFE(item, &binds, tailq, item_tmp)
    {
        if (!CHN_MATCH(&item->node->src, pstSrcChn))
            continue;

        // check if dst already bind to src
        for (i = 0; i < item->node->dsts.u32Num; ++i) {
            if (CHN_MATCH(&item->node->dsts.astMmfChn[i], pstDestChn)) {
                CVI_TRACE_SYS(CVI_DBG_ERR, "Duplicate Dst(%d-%d-%d) to Src(%d-%d-%d)\n",
                              pstDestChn->enModId, pstDestChn->s32DevId, pstDestChn->s32ChnId,
                              pstSrcChn->enModId, pstSrcChn->s32DevId, pstSrcChn->s32ChnId);
                ret = -1;
                goto BIND_EXIT;
            }
        }
        // check if dsts have enough space for one more bind
        if (item->node->dsts.u32Num >= BIND_DEST_MAXNUM) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "Over max bind Dst number\n");
            ret = -1;
            goto BIND_EXIT;
        }
        item->node->dsts.astMmfChn[item->node->dsts.u32Num++] = *pstDestChn;

        goto BIND_SUCCESS;
    }

    // if src not found
    for (i = 0; i < BIND_NODE_MAXNUM; ++i) {
        if (!bind_nodes[i].bUsed) {
            memset(&bind_nodes[i], 0, sizeof(bind_nodes[i]));
            bind_nodes[i].bUsed = true;
            bind_nodes[i].src = *pstSrcChn;
            bind_nodes[i].dsts.u32Num = 1;
            bind_nodes[i].dsts.astMmfChn[0] = *pstDestChn;
            break;
        }
    }

    if (i == BIND_NODE_MAXNUM) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "No free bind node\n");
        ret = -1;
        goto BIND_EXIT;
    }

    item = rt_calloc(1, sizeof(*item));
    if (item == NULL) {
        memset(&bind_nodes[i], 0, sizeof(bind_nodes[i]));
        ret = CVI_ERR_SYS_NOMEM;
        goto BIND_EXIT;
    }

    item->node = &bind_nodes[i];
    TAILQ_INSERT_TAIL(&binds, item, tailq);

BIND_SUCCESS:
    ret = 0;
//TODO: fix it after vc done
    if (pstDestChn->enModId == CVI_ID_VENC)
        venc_vb_ctx[pstDestChn->s32ChnId].enable_bind_mode = CVI_TRUE;

    if (pstSrcChn->enModId == CVI_ID_VDEC)
        vdec_vb_ctx[pstSrcChn->s32ChnId].enable_bind_mode = CVI_TRUE;


BIND_EXIT:
    pthread_mutex_unlock(&bind_lock);

    return ret;
}

CVI_S32 base_ctx_unbind(MMF_CHN_S *pstSrcChn, MMF_CHN_S *pstDestChn)
{
    struct bind_t *item, *item_tmp;
    CVI_U32 i;

    pthread_mutex_lock(&bind_lock);
    TAILQ_FOREACH_SAFE(item, &binds, tailq, item_tmp)
    {
        if (!CHN_MATCH(&item->node->src, pstSrcChn))
            continue;

        for (i = 0; i < item->node->dsts.u32Num; ++i) {
            if (CHN_MATCH(&item->node->dsts.astMmfChn[i], pstDestChn)) {
                if (--item->node->dsts.u32Num) {
                    for (; i < item->node->dsts.u32Num; i++)
                        item->node->dsts.astMmfChn[i] = item->node->dsts.astMmfChn[i + 1];
                }

                if (pstDestChn->enModId == CVI_ID_VENC)
                    venc_vb_ctx[pstDestChn->s32ChnId].enable_bind_mode = CVI_FALSE;

                if (pstSrcChn->enModId == CVI_ID_VDEC)
                    vdec_vb_ctx[pstSrcChn->s32ChnId].enable_bind_mode = CVI_FALSE;

                pthread_mutex_unlock(&bind_lock);
                return 0;
            }
        }
    }
    pthread_mutex_unlock(&bind_lock);
    return 0;
}

CVI_S32 base_ctx_get_bindbysrc(MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
    struct bind_t *item, *item_tmp;
    CVI_U32 i;

    CVI_TRACE_SYS(CVI_DBG_DEBUG, "src(.enModId=%d, .s32DevId=%d, .s32ChnId=%d)\n",
                  pstSrcChn->enModId, pstSrcChn->s32DevId, pstSrcChn->s32ChnId);

    pthread_mutex_lock(&bind_lock);
    TAILQ_FOREACH_SAFE(item, &binds, tailq, item_tmp)
    {
        for (i = 0; i < item->node->dsts.u32Num; ++i) {
            if (CHN_MATCH(&item->node->src, pstSrcChn)) {
                *pstBindDest = item->node->dsts;
                pthread_mutex_unlock(&bind_lock);
                return 0;
            }
        }
    }
    pthread_mutex_unlock(&bind_lock);
    return -1;
}

CVI_S32 base_ctx_get_bindbydst(MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn)
{
    struct bind_t *item, *item_tmp;
    CVI_U32 i;

    CVI_TRACE_SYS(CVI_DBG_DEBUG, "dst(.enModId=%d, .s32DevId=%d, .s32ChnId=%d)\n",
                  pstSrcChn->enModId, pstSrcChn->s32DevId, pstSrcChn->s32ChnId);

    pthread_mutex_lock(&bind_lock);
    TAILQ_FOREACH_SAFE(item, &binds, tailq, item_tmp)
    {
        for (i = 0; i < item->node->dsts.u32Num; ++i) {
            if (CHN_MATCH(&item->node->dsts.astMmfChn[i], pstDestChn)) {
                *pstSrcChn = item->node->src;
                pthread_mutex_unlock(&bind_lock);
                return 0;
            }
        }
    }
    pthread_mutex_unlock(&bind_lock);
    return -1;
}

CVI_S32 base_bind(MMF_CHN_S *pstSrcChn, MMF_CHN_S *pstDestChn)
{
    return base_ctx_bind(pstSrcChn, pstDestChn);
}

CVI_S32 base_unbind(MMF_CHN_S *pstSrcChn, MMF_CHN_S *pstDestChn)
{
    return base_ctx_unbind(pstSrcChn, pstDestChn);
}

CVI_S32 base_ion_alloc(CVI_U64 *p_paddr, void **pp_vaddr, char *buf_name, CVI_U32 buf_len, bool is_cached)
{
    UNUSED(buf_name);
    UNUSED(is_cached);

    if (buf_len == 0) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "can't alloc size 0.\n");
        return -1;
    }

    CVI_VOID *p = rt_malloc_align(buf_len, DEFAULT_ALIGN);
    if (!p) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "alloc failed.\n");
        return -1;
    }

    if (pp_vaddr)
        *pp_vaddr = p;

    *p_paddr = (CVI_U64)p;
    return CVI_SUCCESS;
}

CVI_S32 base_ion_free(CVI_U64 u64PhyAddr)
{
    rt_free_align((void *)u64PhyAddr);
    return CVI_SUCCESS;
}

CVI_S32 base_get_bindbysrc(MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
    return base_ctx_get_bindbysrc(pstSrcChn, pstBindDest);
}

CVI_S32 base_get_bindbydst(MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn)
{
    return base_ctx_get_bindbydst(pstSrcChn, pstSrcChn);
}

int base_core_init(void)
{
    TAILQ_INIT(&binds);
    pthread_mutex_init(&bind_lock, NULL);
    memset(bind_nodes, 0, sizeof(bind_nodes));
    memset(base_m_cb, 0, sizeof(struct base_m_cb_info) * E_MODULE_BUTT);
    return 0;
}

void base_core_exit(void)
{
}
