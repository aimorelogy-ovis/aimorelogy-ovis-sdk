#include <stdlib.h>
#include <cvi_base_ctx.h>
#include <cvi_errno.h>
#include <errno.h>
#include <vip_atomic.h>

#include "cvi_hashmap.h"
#include "proc/rgn_proc.h"
#include "sys.h"
#include <cif_cb.h>
#include <rgn.h>
#include <rgn_cb.h>
#include <vo_cb.h>
#include <vpss_cb.h>
#include "core_rv64.h"
#include "rtthread.h"
#include "rthw.h"

extern void udelay(CVI_U32 us);

/*******************************************************
 *  MACRO definition
 ******************************************************/
#define CHECK_RGN_HANDLE(_ctx, _hdl)                                       \
    do {                                                                   \
        _ctx = hashmapGet(rgnHashmap, (void *)(uintptr_t)_hdl);            \
        if (_ctx == NULL) {                                                \
            CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) not existed.\n", _hdl); \
            return CVI_ERR_RGN_UNEXIST;                                    \
        }                                                                  \
    } while (0)

/*******************************************************
 *  Global variables
 ******************************************************/
// Keep every ctx include rgn_ctx and rgn_proc_ctx
struct cvi_rgn_ctx rgn_prc_ctx[RGN_MAX_NUM];

static RGN_HANDLE gRgnHandle[RGN_MAX_NUM_VPSS];
static RGN_HANDLE gRgnOdecHandle[RGN_MAX_NUM_VPSS];
static RGN_HANDLE gRgnExHandle[RGN_EX_MAX_NUM_VPSS];

static CVI_U32 u32RgnNum;
static pthread_mutex_t hdlslock, g_rgnlock;
static Hashmap *rgnHashmap;
struct cvi_rgn_dev *gRdev;

static rt_atomic_t dev_open_cnt;
static rt_atomic_t rgn_inited;

/*******************************************************
 *  Internal APIs
 ******************************************************/
static int _rgn_hash_key(void *key)
{
    return (uintptr_t)key;
}

static bool _rgn_hash_equals(void *keyA, void *keyB)
{
    return (keyA == keyB);
}

static int _rgn_call_cb(u32 m_id, u32 cmd_id, void *data)
{
    struct base_exe_m_cb exe_cb;

    exe_cb.callee = m_id;
    exe_cb.caller = E_MODULE_RGN;
    exe_cb.cmd_id = cmd_id;
    exe_cb.data = (void *)data;

    return base_exe_module_cb(&exe_cb);
}

CVI_S32 _rgn_init(void)
{
    // Only init once until exit.
    if (!atomic_cmpxchg(&rgn_inited, 0, 1)) {
        CVI_TRACE_RGN(RGN_INFO, "rgn already inited.\n");
        return CVI_SUCCESS;
    }

    rgnHashmap = hashmapCreate(20, _rgn_hash_key, _rgn_hash_equals);
    memset(rgn_prc_ctx, 0, sizeof(rgn_prc_ctx));
    u32RgnNum = 0;
    pthread_mutex_init(&hdlslock, NULL);
    pthread_mutex_init(&g_rgnlock, NULL);
    CVI_TRACE_RGN(RGN_INFO, "-\n");
    return CVI_SUCCESS;
}

CVI_S32 _rgn_exit(void)
{
    // Only exit once.
    if (!atomic_cmpxchg(&rgn_inited, 1, 0)) {
        CVI_TRACE_RGN(RGN_INFO, "rgn already exited.\n");
        return CVI_SUCCESS;
    }

    if (rgnHashmap) {
        hashmapFree(rgnHashmap);
        rgnHashmap = NULL;
    }

    memset(rgn_prc_ctx, 0, sizeof(rgn_prc_ctx));
    u32RgnNum = 0;
    pthread_mutex_destroy(&hdlslock);
    pthread_mutex_destroy(&g_rgnlock);

    CVI_TRACE_RGN(RGN_INFO, "-\n");
    return CVI_SUCCESS;
}

CVI_U32 _rgn_proc_get_idx(RGN_HANDLE hHandle)
{
    CVI_U32 i, idx = 0;

    for (i = 0; i < RGN_MAX_NUM; ++i) {
        if (rgn_prc_ctx[i].Handle == hHandle && rgn_prc_ctx[i].bCreated) {
            idx = i;
            break;
        }
    }

    if (i == RGN_MAX_NUM)
        CVI_TRACE_RGN(RGN_ERR, "cannot find corresponding rgn in rgn_prc_ctx, handle: %d\n", hHandle);

    return idx;
}

static inline CVI_S32 _rgn_get_bytesperline(PIXEL_FORMAT_E enPixelFormat, CVI_U32 width, CVI_U32 *bytesperline)
{
    switch (enPixelFormat) {
    case PIXEL_FORMAT_ARGB_8888:
        *bytesperline = width << 2;
        break;
    case PIXEL_FORMAT_ARGB_4444:
    case PIXEL_FORMAT_ARGB_1555:
        *bytesperline = width << 1;
        break;
    case PIXEL_FORMAT_8BIT_MODE:
        *bytesperline = width;
        break;
    default:
        CVI_TRACE_RGN(RGN_ERR, "not supported pxl-fmt(%d).\n", enPixelFormat);
        return CVI_ERR_RGN_ILLEGAL_PARAM;
    }
    return CVI_SUCCESS;
}

bool is_rect_overlap(RECT_S *r0, RECT_S *r1)
{
    // If one rectangle is on left side of other
    if (((CVI_U32)r0->s32X >= (r1->s32X + r1->u32Width)) || ((CVI_U32)r1->s32X >= (r0->s32X + r0->u32Width)))
        return false;

    // If one rectangle is above other
    if (((CVI_U32)r0->s32Y >= (r1->s32Y + r1->u32Height)) || ((CVI_U32)r1->s32Y >= (r0->s32Y + r0->u32Height)))
        return false;

    return true;
}

static CVI_S32 _rgn_get_rect(struct cvi_rgn_ctx *ctx, RECT_S *rect)
{
    if (ctx->stRegion.enType == OVERLAY_RGN) {
        rect->s32X = ctx->stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
        rect->s32Y = ctx->stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y;
        rect->u32Width = ctx->stRegion.unAttr.stOverlay.stSize.u32Width;
        rect->u32Height = ctx->stRegion.unAttr.stOverlay.stSize.u32Height;
        return CVI_SUCCESS;
    }
    if (ctx->stRegion.enType == OVERLAYEX_RGN) {
        rect->s32X = ctx->stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X;
        rect->s32Y = ctx->stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y;
        rect->u32Width = ctx->stRegion.unAttr.stOverlayEx.stSize.u32Width;
        rect->u32Height = ctx->stRegion.unAttr.stOverlayEx.stSize.u32Height;
        return CVI_SUCCESS;
    }
    if (ctx->stRegion.enType == COVER_RGN) {
        *rect = ctx->stChnAttr.unChnAttr.stCoverChn.stRect;
        return CVI_SUCCESS;
    }
    if (ctx->stRegion.enType == COVEREX_RGN) {
        *rect = ctx->stChnAttr.unChnAttr.stCoverExChn.stRect;
        return CVI_SUCCESS;
    }
    if (ctx->stRegion.enType == MOSAIC_RGN) {
        *rect = ctx->stChnAttr.unChnAttr.stMosaicChn.stRect;
        return CVI_SUCCESS;
    }
    return CVI_FAILURE;
}

static CVI_S32 _rgn_get_layer(struct cvi_rgn_ctx *ctx, CVI_U32 *layer)
{
    if (ctx->stRegion.enType == OVERLAY_RGN)
        *layer = ctx->stChnAttr.unChnAttr.stOverlayChn.u32Layer;
    else if (ctx->stRegion.enType == OVERLAYEX_RGN)
        *layer = ctx->stChnAttr.unChnAttr.stOverlayExChn.u32Layer;
    else if (ctx->stRegion.enType == COVER_RGN)
        *layer = ctx->stChnAttr.unChnAttr.stCoverChn.u32Layer;
    else if (ctx->stRegion.enType == COVEREX_RGN)
        *layer = ctx->stChnAttr.unChnAttr.stCoverExChn.u32Layer;
    else if (ctx->stRegion.enType == MOSAIC_RGN)
        *layer = ctx->stChnAttr.unChnAttr.stMosaicChn.u32Layer;
    else
        return CVI_FAILURE;

    return CVI_SUCCESS;
}

/* _rgn_check_order: check if r1's order should before r0.
 *
 */
CVI_BOOL _rgn_check_order(RECT_S *r0, RECT_S *r1)
{
    if (r0->s32X > r1->s32X)
        return CVI_TRUE;
    if ((r0->s32X == r1->s32X) && (r0->s32Y > r1->s32Y))
        return CVI_TRUE;
    return CVI_FALSE;
}

int _rgn_insert(RGN_HANDLE hdls[], CVI_U8 size, RGN_HANDLE hdl)
{
    struct cvi_rgn_ctx *ctx = NULL;
    RECT_S rect_old, rect_new;
    CVI_U8 i, j;

    CHECK_RGN_HANDLE(ctx, hdl);

    if (ctx->stCanvasInfo[ctx->canvas_idx].bCompressed && hdls[0] != RGN_INVALID_HANDLE) {
        CVI_TRACE_RGN(RGN_ERR, "cannot insert hdl(%d), only allow one ow when odec on\n",
                      hdl);
        return CVI_FAILURE;
    }
    if (_rgn_get_rect(ctx, &rect_new) != CVI_SUCCESS) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) can't get rect.\n", hdl);
        return CVI_FAILURE;
    }
    if (hdls[size - 1] != RGN_INVALID_HANDLE) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) rgn's count is full.\n", hdl);
        return CVI_FAILURE;
    }

    // Check if previously attached rgns overlap new rgn before inserting new rgn
    for (i = 0; i < size; ++i) {
        if (hdls[i] == RGN_INVALID_HANDLE)
            break;

        CHECK_RGN_HANDLE(ctx, hdls[i]);
        if (_rgn_get_rect(ctx, &rect_old) != CVI_SUCCESS) {
            CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) can't get rect.\n", hdls[i]);
            return CVI_FAILURE;
        }

        if (is_rect_overlap(&rect_old, &rect_new)) {
            CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) is overlapped on another RGN_HANDLE(%d).\n", hdl, hdls[i]);
            CVI_TRACE_RGN(RGN_DEBUG, "(%d %d %d %d) <-> (%d %d %d %d)\n", rect_new.s32X, rect_new.s32Y, rect_new.u32Width, rect_new.u32Height, rect_old.s32X, rect_old.s32Y, rect_old.u32Width, rect_old.u32Height);
            return CVI_ERR_RGN_NOT_PERM;
        }
    }

    for (i = 0; i < size; ++i) {
        if (hdls[i] == RGN_INVALID_HANDLE) {
            hdls[i] = hdl;
            CVI_TRACE_RGN(RGN_DEBUG, "RGN_HANDLE(%d) at index(%d).\n", hdl, i);
            return CVI_SUCCESS;
        }

        CHECK_RGN_HANDLE(ctx, hdls[i]);
        if (_rgn_get_rect(ctx, &rect_old) != CVI_SUCCESS) {
            CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) can't get rect.\n", hdls[i]);
            return CVI_FAILURE;
        }

        if (_rgn_check_order(&rect_old, &rect_new)) {
            if ((i + 1) < size)
                for (j = size - 1; j > i; j--)
                    hdls[j] = hdls[j - 1];
            hdls[i] = hdl;
            CVI_TRACE_RGN(RGN_DEBUG, "RGN_HANDLE(%d) at index(%d).\n", hdl, i);
            return CVI_SUCCESS;
        }
    }
    return CVI_FAILURE;
}

int _rgn_ex_insert(RGN_HANDLE hdls[], CVI_U8 size, RGN_HANDLE hdl)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_U32 layer_new, layer_old;
    CVI_U8 i, j;

    CHECK_RGN_HANDLE(ctx, hdl);

    if (hdls[size - 1] != RGN_INVALID_HANDLE) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) rgn_ex's count is full.\n", hdl);
        return CVI_FAILURE;
    }

    if (_rgn_get_layer(ctx, &layer_new) != CVI_SUCCESS) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) can't get layer.\n", hdl);
        return CVI_FAILURE;
    }

    for (i = 0; i < size; ++i) {
        if (hdls[i] == RGN_INVALID_HANDLE) {
            hdls[i] = hdl;
            CVI_TRACE_RGN(RGN_DEBUG, "RGN_HANDLE(%d) at index(%d).\n", hdl, i);
            return CVI_SUCCESS;
        }

        CHECK_RGN_HANDLE(ctx, hdls[i]);
        if (_rgn_get_layer(ctx, &layer_old) != CVI_SUCCESS) {
            CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) can't get layer.\n", hdls[i]);
            return CVI_FAILURE;
        }

        if (layer_new < layer_old) {
            if ((i + 1) < size)
                for (j = size - 1; j > i; j--)
                    hdls[j] = hdls[j - 1];
            hdls[i] = hdl;
            CVI_TRACE_RGN(RGN_DEBUG, "RGN_HANDLE(%d) at index(%d).\n", hdl, i);
            return CVI_SUCCESS;
        }
    }
    return CVI_FAILURE;
}

int _rgn_remove(RGN_HANDLE hdls[], CVI_U8 size, RGN_HANDLE hdl)
{
    CVI_U8 i;

    for (i = 0; i < size; ++i) {
        if (hdls[i] == RGN_INVALID_HANDLE)
            return CVI_ERR_RGN_UNEXIST;

        if (hdl == hdls[i]) {
            if ((i + 1) < size)
                memmove(&hdls[i], &hdls[i + 1], sizeof(hdls[i]) * (size - 1 - i));
            hdls[size - 1] = RGN_INVALID_HANDLE;
            CVI_TRACE_RGN(RGN_DEBUG, "RGN_HANDLE(%d) at index(%d).\n", hdl, i);
            break;
        }
    }
    return CVI_SUCCESS;
}

/* _rgn_fill_pattern: fill buffer of length with pattern.
 *
 * @buf: buffer address
 * @len: buffer length
 * @color: color pattern. It depends on the pixel-format.
 * @bpp: bytes-per-pixel. Only 2 or 4 works.
 */
void _rgn_fill_pattern(void *buf, CVI_U32 len, CVI_U32 color, CVI_U8 bpp)
{
    CVI_U32 *p = buf;
    CVI_U32 i;
    CVI_U32 pattern = color;

    if (bpp == 2)
        pattern |= (pattern << 16);
    for (i = 0; i < len / sizeof(CVI_U32); ++i)
        p[i] = pattern;

    if (len & 0x02)
        *(((CVI_U16 *)&p[i]) + 1) = color;
}

/* _rgn_update_cover_canvas: fill cover/coverex if needed.
 *
 * @ctx: the region ctx to be updated.
 * @pstChnAttr: could be NULL. If not, will be used to check if update needed.
 */
static CVI_S32 _rgn_update_cover_canvas(struct cvi_rgn_ctx *ctx, const RGN_CHN_ATTR_S *pstChnAttr)
{
    PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_ARGB_1555;
    CVI_U32 bytesperline;
    RECT_S rect;
    CVI_U32 buf_len;
    CVI_U32 stride;
    CVI_BOOL bfill = CVI_FALSE;
    CVI_U32 proc_idx;

    if (!ctx || !pstChnAttr)
        return CVI_ERR_RGN_NULL_PTR;

    rect = (pstChnAttr->enType == COVER_RGN)
               ? pstChnAttr->unChnAttr.stCoverChn.stRect
               : pstChnAttr->unChnAttr.stCoverExChn.stRect;

    // check if color changed.
    bfill = (((ctx->stChnAttr.enType == COVER_RGN) && (ctx->stChnAttr.unChnAttr.stCoverChn.u32Color != pstChnAttr->unChnAttr.stCoverChn.u32Color))) ||
            (((ctx->stChnAttr.enType == COVEREX_RGN) && (ctx->stChnAttr.unChnAttr.stCoverExChn.u32Color != pstChnAttr->unChnAttr.stCoverExChn.u32Color)));

    _rgn_get_bytesperline(enPixelFormat, rect.u32Width, &bytesperline);
    stride = ALIGN(bytesperline, 32);
    buf_len = stride * rect.u32Height;

    proc_idx = _rgn_proc_get_idx(ctx->Handle);
    if (ctx->ion_len == 0) {
        ctx->canvas_idx = rgn_prc_ctx[proc_idx].canvas_idx = 0;
        rgn_prc_ctx[proc_idx].stCanvasInfo[0] = *ctx->stCanvasInfo;
    }

    // update canvas
    if (buf_len > ctx->ion_len) {
        if (ctx->ion_len != 0)
            rt_free_align((void *)ctx->stCanvasInfo[0].u64PhyAddr);

        ctx->stCanvasInfo[0].enPixelFormat = enPixelFormat;
        ctx->stCanvasInfo[0].stSize.u32Width = rect.u32Width;
        ctx->stCanvasInfo[0].stSize.u32Height = rect.u32Height;
        ctx->stCanvasInfo[0].u32Stride = stride;
        ctx->ion_len = ctx->u32MaxNeedIon = buf_len;
        ctx->stCanvasInfo[0].u64PhyAddr = (CVI_U64)rt_malloc_align(ctx->ion_len + GOP_ALIGNMENT, GOP_ALIGNMENT);
        if (ctx->stCanvasInfo[0].u64PhyAddr == 0) {
            CVI_TRACE_RGN(RGN_ERR, "Region(%d) Can't acquire ion for Canvas.\n", ctx->Handle);
            return CVI_ERR_RGN_NOBUF;
        }
        ctx->stCanvasInfo[0].pu8VirtAddr = (CVI_U8 *)ctx->stCanvasInfo[0].u64PhyAddr;
        bfill = CVI_TRUE;
    } else if (buf_len <= ctx->ion_len) {
        ctx->stCanvasInfo[0].stSize.u32Width = rect.u32Width;
        ctx->stCanvasInfo[0].stSize.u32Height = rect.u32Height;
        ctx->stCanvasInfo[0].u32Stride = stride;
    }
    rgn_prc_ctx[proc_idx].stCanvasInfo[0] = ctx->stCanvasInfo[0];

    // fill color pattern to cover/coverex.
    if (bfill) {
        CVI_U16 color;

        color = (ctx->stChnAttr.enType == COVEREX_RGN)
                    ? RGB888_2_ARGB1555(pstChnAttr->unChnAttr.stCoverExChn.u32Color)
                    : RGB888_2_ARGB1555(pstChnAttr->unChnAttr.stCoverChn.u32Color);
        _rgn_fill_pattern(ctx->stCanvasInfo[0].pu8VirtAddr, ctx->ion_len, color, 2);
    }

    return CVI_SUCCESS;
}

static CVI_S32 _rgn_update_hw_cfg_to_module(struct cvi_rgn_ctx *ctx, RGN_HANDLE *hdls, void *cfg)
{
    // Set VO module hdls and rgn_cfg
    if (ctx->stChn.enModId == CVI_ID_VO) {
        struct _rgn_hdls_cb_param rgn_hdls_arg;
        struct _rgn_cfg_cb_param rgn_cfg_arg;

        rgn_hdls_arg.stChn = ctx->stChn;
        rgn_hdls_arg.hdls = hdls;
        if (_rgn_call_cb(E_MODULE_VO, VO_CB_SET_RGN_HDLS, &rgn_hdls_arg) != 0) {
            CVI_TRACE_RGN(RGN_ERR, "VO_CB_SET_RGN_HDLS is failed\n");
            return CVI_ERR_RGN_ILLEGAL_PARAM;
        }

        rgn_cfg_arg.stChn = ctx->stChn;
        rgn_cfg_arg.rgn_cfg = *(struct cvi_rgn_cfg *)cfg;
        if (_rgn_call_cb(E_MODULE_VO, VO_CB_SET_RGN_CFG, &rgn_cfg_arg) != 0) {
            CVI_TRACE_RGN(RGN_ERR, "VO_CB_SET_RGN_CFG is failed\n");
            return CVI_ERR_RGN_ILLEGAL_PARAM;
        }
    } else if (ctx->stChn.enModId == CVI_ID_VPSS) {
        // Set VPSS module rgn hdls and rgn_cfg
        if (ctx->stRegion.enType == OVERLAY_RGN || ctx->stRegion.enType == COVER_RGN) {
            struct _rgn_hdls_cb_param rgn_hdls_arg;
            struct _rgn_cfg_cb_param rgn_cfg_arg;
            CVI_U32 osd_layer = (ctx->stCanvasInfo[ctx->canvas_idx].bCompressed) ? RGN_ODEC_LAYER_VPSS : RGN_NORMAL_LAYER_VPSS;

            rgn_hdls_arg.stChn = ctx->stChn;
            rgn_hdls_arg.hdls = hdls;
            rgn_hdls_arg.layer = osd_layer;
            if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_SET_RGN_HDLS, &rgn_hdls_arg) != 0) {
                CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_SET_RGN_HDLS is failed\n");
                return CVI_ERR_RGN_ILLEGAL_PARAM;
            }

            rgn_cfg_arg.stChn = ctx->stChn;
            rgn_cfg_arg.rgn_cfg = *(struct cvi_rgn_cfg *)cfg;
            rgn_cfg_arg.layer = osd_layer;
            if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_SET_RGN_CFG, &rgn_cfg_arg) != 0) {
                CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_SET_RGN_CFG is failed\n");
                return CVI_ERR_RGN_ILLEGAL_PARAM;
            }
        } else {
            // Set VPSS module rgnex hdls and rgn_ex_cfg
            struct _rgn_hdls_cb_param rgnex_hdls_arg;
            struct _rgn_ex_cfg_cb_param rgnex_cfg_arg;

            rgnex_hdls_arg.stChn = ctx->stChn;
            rgnex_hdls_arg.hdls = hdls;
            if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_SET_RGNEX_HDLS, &rgnex_hdls_arg) != 0) {
                CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_SET_RGNEX_HDLS is failed\n");
                return CVI_ERR_RGN_ILLEGAL_PARAM;
            }

            rgnex_cfg_arg.stChn = ctx->stChn;
            rgnex_cfg_arg.rgn_ex_cfg = *(struct cvi_rgn_ex_cfg *)cfg;
            if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_SET_RGNEX_CFG, &rgnex_cfg_arg) != 0) {
                CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_SET_RGNEX_CFG is failed\n");
                return CVI_ERR_RGN_ILLEGAL_PARAM;
            }
        }
    } else {
        CVI_TRACE_RGN(RGN_ERR, "Unsupported mod_id(%d)\n", ctx->stChn.enModId);
        return CVI_ERR_RGN_ILLEGAL_PARAM;
    }

    return CVI_SUCCESS;
}

/* _rgn_set_hw_cfg: update cfg based on the rgn attached.
 *
 * @hdls: rgn handles on chn.
 * @size: size of gop acceptable.
 * @cfg: the cfg to be updated.
 */
static CVI_S32 _rgn_set_hw_cfg(RGN_HANDLE hdls[], CVI_U8 size, struct cvi_rgn_cfg *cfg)
{
    struct cvi_rgn_ctx *ctx = NULL;
    RECT_S rect;
    CVI_U8 rgn_idx = 0;
    CVI_U8 i;

    for (i = 0; i < size; ++i) {
        if (hdls[i] == RGN_INVALID_HANDLE)
            break;

        CHECK_RGN_HANDLE(ctx, hdls[i]);

        if (!ctx->stChnAttr.bShow)
            continue;
        if (_rgn_get_rect(ctx, &rect) != CVI_SUCCESS)
            continue;

        switch (ctx->stCanvasInfo[ctx->canvas_idx].enPixelFormat) {
        case PIXEL_FORMAT_ARGB_8888:
            cfg->param[rgn_idx].fmt = CVI_RGN_FMT_ARGB8888;
            break;
        case PIXEL_FORMAT_ARGB_4444:
            cfg->param[rgn_idx].fmt = CVI_RGN_FMT_ARGB4444;
            break;
        case PIXEL_FORMAT_8BIT_MODE:
            cfg->param[rgn_idx].fmt = CVI_RGN_FMT_256LUT;
            break;
        case PIXEL_FORMAT_ARGB_1555:
        default:
            cfg->param[rgn_idx].fmt = CVI_RGN_FMT_ARGB1555;
            break;
        }
        cfg->param[rgn_idx].rect.left = rect.s32X;
        cfg->param[rgn_idx].rect.top = rect.s32Y;
        cfg->param[rgn_idx].rect.width = ctx->stCanvasInfo[ctx->canvas_idx].stSize.u32Width;
        cfg->param[rgn_idx].rect.height = ctx->stCanvasInfo[ctx->canvas_idx].stSize.u32Height;
        cfg->param[rgn_idx].phy_addr = (CVI_U64)ctx->stCanvasInfo[ctx->canvas_idx].pu8VirtAddr;
        cfg->param[rgn_idx].stride = ctx->stCanvasInfo[ctx->canvas_idx].u32Stride;

        // only one OSD window can be attached.
        if (ctx->stCanvasInfo->bCompressed) {
            cfg->odec.enable = true;
            cfg->odec.attached_ow = rgn_idx;
            cfg->odec.bso_sz = ctx->stCanvasInfo[ctx->canvas_idx].u32CompressedSize;
            break;
        } else
            cfg->odec.enable = false;
        ++rgn_idx;
    }
    cfg->num_of_rgn = (cfg->odec.enable) ? 1 : rgn_idx;

    cfg->hscale_x2 = false;
    cfg->vscale_x2 = false;
    cfg->colorkey_en = false;

    return CVI_SUCCESS;
}

/* _rgn_ex_set_hw_cfg: update cfg based on the rgn_ex attached.
 *
 * @hdls: rgn_ex handles on chn.
 * @size: max num of rgn.
 * @cfg: the cfg to be updated.
 */
static CVI_S32 _rgn_ex_set_hw_cfg(RGN_HANDLE hdls[], CVI_U8 size, struct cvi_rgn_ex_cfg *cfg)
{
    struct cvi_rgn_ctx *ctx = NULL;
    RECT_S rect;
    CVI_U8 rgn_idx = 0;
    CVI_U8 rgn_tile = 0; // 1: left tile, 2: right tile
    CVI_U32 left_w = 0, bpp = 0;
    CVI_U8 i;

    for (i = 0; i < size; ++i) {
        if (hdls[i] == RGN_INVALID_HANDLE)
            break;

        CHECK_RGN_HANDLE(ctx, hdls[i]);
        if (!ctx->stChnAttr.bShow)
            continue;
        if (_rgn_get_rect(ctx, &rect) != CVI_SUCCESS)
            continue;

        if (rect.u32Width > RGN_EX_MAX_WIDTH) {
            CVI_U32 byte_offset;

            left_w = rect.u32Width / 2;
            bpp = (ctx->stCanvasInfo[ctx->canvas_idx].enPixelFormat == PIXEL_FORMAT_ARGB_8888)
                      ? 4
                      : 2;

            byte_offset = (bpp * left_w) & (0x20 - 1);
            if (byte_offset) {
                left_w -= (byte_offset / bpp);
            }
            rgn_tile = 1;
        }

        do {
            switch (ctx->stCanvasInfo[ctx->canvas_idx].enPixelFormat) {
            case PIXEL_FORMAT_ARGB_8888:
                cfg->rgn_ex_param[rgn_idx].fmt = CVI_RGN_FMT_ARGB8888;
                break;
            case PIXEL_FORMAT_ARGB_4444:
                cfg->rgn_ex_param[rgn_idx].fmt = CVI_RGN_FMT_ARGB4444;
                break;
            case PIXEL_FORMAT_ARGB_1555:
            default:
                cfg->rgn_ex_param[rgn_idx].fmt = CVI_RGN_FMT_ARGB1555;
                break;
            }
            cfg->rgn_ex_param[rgn_idx].stride = ctx->stCanvasInfo[ctx->canvas_idx].u32Stride;

            if (rgn_tile == 1) { // left tile
                cfg->rgn_ex_param[rgn_idx].phy_addr = ctx->stCanvasInfo[ctx->canvas_idx].u64PhyAddr;
                cfg->rgn_ex_param[rgn_idx].rect.left = rect.s32X;
                cfg->rgn_ex_param[rgn_idx].rect.top = rect.s32Y;
                cfg->rgn_ex_param[rgn_idx].rect.width = left_w;
                cfg->rgn_ex_param[rgn_idx].rect.height = ctx->stCanvasInfo[ctx->canvas_idx].stSize.u32Height;
                rgn_tile = 2;
            } else if (rgn_tile == 2) { // right tile
                cfg->rgn_ex_param[rgn_idx].phy_addr = ctx->stCanvasInfo[ctx->canvas_idx].u64PhyAddr + (left_w * bpp);
                cfg->rgn_ex_param[rgn_idx].rect.left = rect.s32X + left_w;
                cfg->rgn_ex_param[rgn_idx].rect.top = rect.s32Y;
                cfg->rgn_ex_param[rgn_idx].rect.width = ctx->stCanvasInfo[ctx->canvas_idx].stSize.u32Width - left_w;
                cfg->rgn_ex_param[rgn_idx].rect.height = ctx->stCanvasInfo[ctx->canvas_idx].stSize.u32Height;
                rgn_tile = 0;
            } else { // no tile
                cfg->rgn_ex_param[rgn_idx].phy_addr = ctx->stCanvasInfo[ctx->canvas_idx].u64PhyAddr;
                cfg->rgn_ex_param[rgn_idx].rect.left = rect.s32X;
                cfg->rgn_ex_param[rgn_idx].rect.top = rect.s32Y;
                cfg->rgn_ex_param[rgn_idx].rect.width = ctx->stCanvasInfo[ctx->canvas_idx].stSize.u32Width;
                cfg->rgn_ex_param[rgn_idx].rect.height = ctx->stCanvasInfo[ctx->canvas_idx].stSize.u32Height;
                ++rgn_idx;
                break;
            }
            ++rgn_idx;
        } while (rgn_tile);
    }
    cfg->num_of_rgn_ex = rgn_idx;

    cfg->hscale_x2 = false;
    cfg->vscale_x2 = false;
    cfg->colorkey_en = false;

    return CVI_SUCCESS;
}

/* _rgn_get_chn_info: get info of chn for rgn.
 *
 * @pstChn: the chn want to check.
 * @numOfLayer: number of gop layer supported on the chn.
 * @size: number of ow supported on the chn.
 */
static CVI_S32 _rgn_get_chn_info(struct cvi_rgn_ctx *ctx,
                                 RGN_HANDLE **hdls, CVI_U8 *size)
{
    struct _rgn_hdls_cb_param rgn_hdls_arg;

    if (ctx->stChn.enModId == CVI_ID_VO) {
        rgn_hdls_arg.stChn = ctx->stChn;
        rgn_hdls_arg.hdls = gRgnHandle;
        rgn_hdls_arg.layer = 0; // vo only has one layer gop
        if (_rgn_call_cb(E_MODULE_VO, VO_CB_GET_RGN_HDLS, &rgn_hdls_arg) != 0) {
            CVI_TRACE_RGN(RGN_ERR, "VO_CB_GET_RGN_HDLS is failed\n");
            return CVI_ERR_RGN_ILLEGAL_PARAM;
        }

        *hdls = gRgnHandle;
        *size = RGN_MAX_NUM_VO;
    } else if (ctx->stChn.enModId == CVI_ID_VPSS) {
        if (ctx->stRegion.enType == OVERLAY_RGN || ctx->stRegion.enType == COVER_RGN) {
            rgn_hdls_arg.stChn = ctx->stChn;
            rgn_hdls_arg.hdls = (ctx->stCanvasInfo[ctx->canvas_idx].bCompressed) ? gRgnOdecHandle : gRgnHandle;
            rgn_hdls_arg.layer = (ctx->stCanvasInfo[ctx->canvas_idx].bCompressed) ? RGN_ODEC_LAYER_VPSS : RGN_NORMAL_LAYER_VPSS;
            if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_GET_RGN_HDLS, &rgn_hdls_arg) != 0) {
                CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_GET_RGN_HDLS is failed\n");
                return CVI_ERR_RGN_ILLEGAL_PARAM;
            }

            *hdls = rgn_hdls_arg.hdls;
            *size = RGN_MAX_NUM_VPSS;
        } else {
            rgn_hdls_arg.stChn = ctx->stChn;
            rgn_hdls_arg.hdls = gRgnExHandle;
            rgn_hdls_arg.layer = 0;
            if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_GET_RGNEX_HDLS, &rgn_hdls_arg) != 0) {
                CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_GET_RGNEX_HDLS is failed\n");
                return CVI_ERR_RGN_ILLEGAL_PARAM;
            }

            *hdls = gRgnExHandle;
            *size = RGN_EX_MAX_NUM_VPSS;
        }
    } else {
        return CVI_ERR_RGN_INVALID_DEVID;
    }

    return CVI_SUCCESS;
}

static CVI_S32 _rgn_update_hw(struct cvi_rgn_ctx *ctx, enum RGN_OP op)
{
    CVI_U8 size;
    CVI_S32 ret;
    RGN_HANDLE *hdls;
    struct cvi_rgn_cfg cfg;
    struct cvi_rgn_ex_cfg ex_cfg;

    pthread_mutex_lock(&hdlslock);
    ret = _rgn_get_chn_info(ctx, &hdls, &size);
    if (ret != CVI_SUCCESS) {
        pthread_mutex_unlock(&hdlslock);
        return ret;
    }

    if (op == RGN_OP_INSERT) {
        if (ctx->stRegion.enType == OVERLAY_RGN || ctx->stRegion.enType == COVER_RGN) {
            ret = _rgn_insert(hdls, size, ctx->Handle);
            if (ret != CVI_SUCCESS) {
                pthread_mutex_unlock(&hdlslock);
                return ret;
            }
        } else {
            ret = _rgn_ex_insert(hdls, size, ctx->Handle);
            if (ret != CVI_SUCCESS) {
                pthread_mutex_unlock(&hdlslock);
                return ret;
            }
        }
    } else if (op == RGN_OP_REMOVE) {
        ret = _rgn_remove(hdls, size, ctx->Handle);
        if (ret != CVI_SUCCESS) {
            pthread_mutex_unlock(&hdlslock);
            CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) not at CHN(%s-%d-%d).\n", ctx->Handle, base_get_modname(ctx->stChn.enModId), ctx->stChn.s32DevId, ctx->stChn.s32ChnId);
            return ret;
        }
    } else if (op == RGN_OP_UPDATE) {
        _rgn_remove(hdls, size, ctx->Handle);
        if (ctx->stRegion.enType == OVERLAY_RGN || ctx->stRegion.enType == COVER_RGN) {
            ret = _rgn_insert(hdls, size, ctx->Handle);
            if (ret != CVI_SUCCESS) {
                pthread_mutex_unlock(&hdlslock);
                return ret;
            }
        } else {
            ret = _rgn_ex_insert(hdls, size, ctx->Handle);
            if (ret != CVI_SUCCESS) {
                pthread_mutex_unlock(&hdlslock);
                return ret;
            }
        }
    }

    if (ctx->stRegion.enType == OVERLAY_RGN || ctx->stRegion.enType == COVER_RGN) {
        memset(&cfg, 0, sizeof(cfg));
        ret = _rgn_set_hw_cfg(hdls, size, &cfg);
        if (ret != CVI_SUCCESS) {
            CVI_TRACE_RGN(RGN_ERR, "_rgn_set_hw_cfg failed\n");
            pthread_mutex_unlock(&hdlslock);
            return ret;
        }
        ret = _rgn_update_hw_cfg_to_module(ctx, hdls, (void *)&cfg);
        if (ret != CVI_SUCCESS) {
            CVI_TRACE_RGN(RGN_ERR, "_rgn_update_hw_cfg_to_module failed\n");
            pthread_mutex_unlock(&hdlslock);
            return ret;
        }
    } else {
        memset(&ex_cfg, 0, sizeof(ex_cfg));
        ret = _rgn_ex_set_hw_cfg(hdls, size, &ex_cfg);
        if (ret != CVI_SUCCESS) {
            CVI_TRACE_RGN(RGN_ERR, "_rgn_ex_set_hw_cfg failed\n");
            pthread_mutex_unlock(&hdlslock);
            return ret;
        }
        ret = _rgn_update_hw_cfg_to_module(ctx, hdls, (void *)&ex_cfg);
        if (ret != CVI_SUCCESS) {
            CVI_TRACE_RGN(RGN_ERR, "_rgn_update_hw_cfg_to_module failed\n");
            pthread_mutex_unlock(&hdlslock);
            return ret;
        }
    }

    //rt_hw_cpu_dcache_clean_and_invalidate_local((uint64_t *)ctx->stCanvasInfo[ctx->canvas_idx].pu8VirtAddr, ctx->ion_len);

    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH | RT_HW_CACHE_INVALIDATE, (void *)ctx->stCanvasInfo[ctx->canvas_idx].pu8VirtAddr, ctx->ion_len);

    pthread_mutex_unlock(&hdlslock);

    return CVI_SUCCESS;
}

CVI_S32 _rgn_check_chn_attr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
    RECT_S rect_chn, rect_rgn;
    struct cvi_rgn_ctx *ctx;

    CHECK_RGN_HANDLE(ctx, Handle);

    if (pstChn->enModId == CVI_ID_VO) {
        struct _rgn_chn_size_cb_param rgn_chn_arg;

        rgn_chn_arg.stChn = *pstChn;

        if (_rgn_call_cb(E_MODULE_VO, VO_CB_GET_CHN_SIZE, &rgn_chn_arg)) {
            CVI_TRACE_RGN(RGN_ERR, "VO_CB_GET_CHN_SIZE failed!\n");
            return CVI_ERR_RGN_INVALID_CHNID;
        }
        rect_chn = rgn_chn_arg.rect;
    } else if (pstChn->enModId == CVI_ID_VPSS) {
        struct _rgn_chn_size_cb_param rgn_chn_arg;

        rgn_chn_arg.stChn = *pstChn;

        if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_GET_CHN_SIZE, &rgn_chn_arg)) {
            CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_GET_CHN_SIZE failed!\n");
            return CVI_ERR_RGN_INVALID_CHNID;
        }
        rect_chn = rgn_chn_arg.rect;
    } else
        return CVI_ERR_RGN_ILLEGAL_PARAM;

    if (pstChnAttr->enType == COVEREX_RGN) {
        if (pstChnAttr->unChnAttr.stCoverExChn.enCoverType != AREA_RECT) {
            CVI_TRACE_RGN(RGN_ERR, "AREA_RECT only now.\n");
            return CVI_ERR_RGN_ILLEGAL_PARAM;
        }
        rect_rgn = pstChnAttr->unChnAttr.stCoverExChn.stRect;
    } else if (pstChnAttr->enType == COVER_RGN) {
        if (pstChnAttr->unChnAttr.stCoverChn.enCoverType != AREA_RECT) {
            CVI_TRACE_RGN(RGN_ERR, "AREA_RECT only now.\n");
            return CVI_ERR_RGN_ILLEGAL_PARAM;
        }
        if (pstChnAttr->unChnAttr.stCoverChn.enCoordinate != RGN_ABS_COOR) {
            CVI_TRACE_RGN(RGN_ERR, "abs-coordinate only now.\n");
            return CVI_ERR_RGN_ILLEGAL_PARAM;
        }
        rect_rgn = pstChnAttr->unChnAttr.stCoverChn.stRect;
    } else if (pstChnAttr->enType == OVERLAY_RGN) {
        rect_rgn.s32X = pstChnAttr->unChnAttr.stOverlayChn.stPoint.s32X;
        rect_rgn.s32Y = pstChnAttr->unChnAttr.stOverlayChn.stPoint.s32Y;
        rect_rgn.u32Width = ctx->stRegion.unAttr.stOverlay.stSize.u32Width;
        rect_rgn.u32Height = ctx->stRegion.unAttr.stOverlay.stSize.u32Height;
    } else if (pstChnAttr->enType == OVERLAYEX_RGN) {
        rect_rgn.s32X = pstChnAttr->unChnAttr.stOverlayExChn.stPoint.s32X;
        rect_rgn.s32Y = pstChnAttr->unChnAttr.stOverlayExChn.stPoint.s32Y;
        rect_rgn.u32Width = ctx->stRegion.unAttr.stOverlayEx.stSize.u32Width;
        rect_rgn.u32Height = ctx->stRegion.unAttr.stOverlayEx.stSize.u32Height;
    } else {
        return CVI_ERR_RGN_NOT_SUPPORT;
    }

    if ((rect_rgn.u32Width + rect_rgn.s32X) > rect_chn.u32Width || (rect_rgn.u32Height + rect_rgn.s32Y) > rect_chn.u32Height) {
        CVI_TRACE_RGN(RGN_ERR, "size(%d %d %d %d) larger than chnsize(%d %d).\n",
                      rect_rgn.s32X, rect_rgn.u32Width, rect_rgn.s32Y, rect_rgn.u32Height,
                      rect_chn.u32Width, rect_chn.u32Height);
        return CVI_ERR_RGN_ILLEGAL_PARAM;
    }

    return CVI_SUCCESS;
}

/**************************************************************************
 *   Sinking RGN APIs related functions.
 **************************************************************************/
static CVI_S32 _rgn_create(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_S32 ret;
    CVI_U32 proc_idx;
    CVI_U8 i;

    pthread_mutex_lock(&g_rgnlock);

    ctx = hashmapGet(rgnHashmap, (void *)(uintptr_t)Handle);
    if (ctx != NULL) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) is already existing.\n", Handle);
        pthread_mutex_unlock(&g_rgnlock);
        return CVI_ERR_RGN_EXIST;
    }

    if (u32RgnNum >= RGN_MAX_NUM) {
        CVI_TRACE_RGN(RGN_ERR, "over max rgn number %d\n", RGN_MAX_NUM);
        pthread_mutex_unlock(&g_rgnlock);
        return CVI_ERR_RGN_NOT_PERM;
    }

    if (pstRegion->enType == COVEREX_RGN || pstRegion->enType == OVERLAYEX_RGN || pstRegion->enType == MOSAIC_RGN) {
        if (sys_get_vpssmode() != VPSS_MODE_RGNEX) {
            CVI_TRACE_RGN(RGN_ERR, "rgn extension only support on vpss rgnex mode.\n");
            pthread_mutex_unlock(&g_rgnlock);
            return CVI_ERR_RGN_NOT_SUPPORT;
        }
    }

    // check parameters.
    if (pstRegion->enType == OVERLAY_RGN || pstRegion->enType == OVERLAYEX_RGN) {
        PIXEL_FORMAT_E pixelFormat;
        CVI_U32 canvasNum;
        CVI_BOOL check_fail = CVI_FALSE;

        if (pstRegion->enType == OVERLAY_RGN) {
            pixelFormat = pstRegion->unAttr.stOverlay.enPixelFormat;
            canvasNum = pstRegion->unAttr.stOverlay.u32CanvasNum;
        } else {
            pixelFormat = pstRegion->unAttr.stOverlayEx.enPixelFormat;
            canvasNum = pstRegion->unAttr.stOverlayEx.u32CanvasNum;
        }

        if (canvasNum == 0 || canvasNum > RGN_MAX_BUF_NUM) {
            CVI_TRACE_RGN(RGN_ERR, "invalid u32CanvasNum(%d).\n", canvasNum);
            check_fail = CVI_TRUE;
        }

        if ((pixelFormat != PIXEL_FORMAT_ARGB_8888) && (pixelFormat != PIXEL_FORMAT_ARGB_4444) && (pixelFormat != PIXEL_FORMAT_ARGB_1555) && (pixelFormat != PIXEL_FORMAT_8BIT_MODE)) {
            CVI_TRACE_RGN(RGN_ERR, "unsupported pxl-fmt(%d).\n", pixelFormat);
            check_fail = CVI_TRUE;
        }

        if (check_fail) {
            pthread_mutex_unlock(&g_rgnlock);
            return CVI_ERR_RGN_ILLEGAL_PARAM;
        }
    } else if (pstRegion->enType == MOSAIC_RGN) {
        CVI_TRACE_RGN(RGN_ERR, "Mosaic not supported yet.\n");
        pthread_mutex_unlock(&g_rgnlock);
        return CVI_ERR_RGN_NOT_SUPPORT;
    }

    ctx = calloc(1, sizeof(*ctx));
    if (ctx == CVI_NULL) {
        CVI_TRACE_RGN(RGN_ERR, "malloc failed.\n");
        ret = CVI_ERR_RGN_NOMEM;
        goto RGN_CTX_MALLOC_FAIL;
    }
    ctx->Handle = Handle;
    ctx->stRegion = *pstRegion;

    if (pstRegion->enType == OVERLAY_RGN || pstRegion->enType == OVERLAYEX_RGN) {
        PIXEL_FORMAT_E pixelFormat;
        CVI_U32 canvasNum, bgColor, compressed_size;
        SIZE_S rgnSize;
        CVI_U32 bytesperline = 0;

        if (pstRegion->enType == OVERLAY_RGN) {
            pixelFormat = pstRegion->unAttr.stOverlay.enPixelFormat;
            canvasNum = pstRegion->unAttr.stOverlay.u32CanvasNum;
            rgnSize = pstRegion->unAttr.stOverlay.stSize;
            bgColor = pstRegion->unAttr.stOverlay.u32BgColor;
            switch (pstRegion->unAttr.stOverlay.stCompressInfo.enOSDCompressMode) {
            case OSD_COMPRESS_MODE_SW:
                compressed_size = pstRegion->unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
                break;
            case OSD_COMPRESS_MODE_HW:
                compressed_size = pstRegion->unAttr.stOverlay.stCompressInfo.u32CompressedSize;
                break;
            default:
                compressed_size = 0;
                break;
            }
        } else {
            pixelFormat = pstRegion->unAttr.stOverlayEx.enPixelFormat;
            canvasNum = pstRegion->unAttr.stOverlayEx.u32CanvasNum;
            rgnSize = pstRegion->unAttr.stOverlayEx.stSize;
            bgColor = pstRegion->unAttr.stOverlayEx.u32BgColor;
            switch (pstRegion->unAttr.stOverlayEx.stCompressInfo.enOSDCompressMode) {
            case OSD_COMPRESS_MODE_SW:
                compressed_size = pstRegion->unAttr.stOverlayEx.stCompressInfo.u32EstCompressedSize;
                break;
            case OSD_COMPRESS_MODE_HW:
                compressed_size = pstRegion->unAttr.stOverlayEx.stCompressInfo.u32CompressedSize;
                break;
            default:
                compressed_size = 0;
                break;
            }
        }

        ctx->canvas_idx = 0;

        ret = _rgn_get_bytesperline(pixelFormat, rgnSize.u32Width, &bytesperline);
        if (ret != CVI_SUCCESS) {
            goto RGN_FMT_INCORRECT;
        }

        ctx->stCanvasInfo[0].enPixelFormat = pixelFormat;
        ctx->stCanvasInfo[0].stSize = rgnSize;
        ctx->stCanvasInfo[0].u32Stride = ALIGN(bytesperline, 32);
        if (pstRegion->unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_NONE) {
            ctx->stCanvasInfo[0].bCompressed = false;
            ctx->stCanvasInfo[0].u32CompressedSize = 0;
            ctx->ion_len = ctx->u32MaxNeedIon = ctx->stCanvasInfo[0].u32Stride * ctx->stCanvasInfo[0].stSize.u32Height;
        } else {
            ctx->stCanvasInfo[0].bCompressed = true;
            ctx->stCanvasInfo[0].u32CompressedSize = compressed_size;
            ctx->ion_len = ctx->u32MaxNeedIon = compressed_size;
        }

        CVI_TRACE_RGN(RGN_INFO, "RGN_HANDLE(%d) canvas fmt(%d) size(%d * %d) stride(%d).\n", Handle, ctx->stCanvasInfo[0].enPixelFormat, ctx->stCanvasInfo[0].stSize.u32Width, ctx->stCanvasInfo[0].stSize.u32Height, ctx->stCanvasInfo[0].u32Stride);

        if (ctx->stCanvasInfo[0].bCompressed)
            CVI_TRACE_RGN(RGN_INFO, "RGN_HANDLE(%d) compressed canvas size(%d).\n", Handle, ctx->stCanvasInfo[0].u32CompressedSize);

        if (canvasNum > 1)
            ctx->stCanvasInfo[1] = ctx->stCanvasInfo[0];

        for (i = 0; i < canvasNum; ++i) {
            ctx->stCanvasInfo[i].u64PhyAddr = (CVI_U64)rt_malloc_align(ctx->ion_len + GOP_ALIGNMENT, GOP_ALIGNMENT);
			if (ctx->stCanvasInfo[i].u64PhyAddr == 0) {
				CVI_TRACE_RGN(RGN_ERR, "Region(%d) Can't acquire ion for Canvas-%d.\n", Handle, i);
				ret = CVI_ERR_RGN_NOBUF;
				goto RGN_ION_ALLOC_FAIL;
			}
			ctx->stCanvasInfo[i].pu8VirtAddr = (CVI_U8 *)ctx->stCanvasInfo[i].u64PhyAddr;

            _rgn_fill_pattern(ctx->stCanvasInfo[i].pu8VirtAddr, ctx->ion_len, bgColor, (pixelFormat == PIXEL_FORMAT_ARGB_8888) ? 4 : ((pixelFormat == PIXEL_FORMAT_8BIT_MODE) ? 1 : 2));
        }
    }

    hashmapPut(rgnHashmap, (void *)(uintptr_t)Handle, ctx);
    u32RgnNum++;

    // update rgn proc info
    for (proc_idx = 0; proc_idx < RGN_MAX_NUM; ++proc_idx) {
        if (!rgn_prc_ctx[proc_idx].bCreated) {
            rgn_prc_ctx[proc_idx].bCreated = true;
            rgn_prc_ctx[proc_idx].Handle = ctx->Handle;
            rgn_prc_ctx[proc_idx].stRegion = ctx->stRegion;
            if (ctx->stRegion.enType == OVERLAY_RGN) {
                rgn_prc_ctx[proc_idx].canvas_idx = ctx->canvas_idx;
                for (i = 0; i < ctx->stRegion.unAttr.stOverlay.u32CanvasNum; ++i)
                    rgn_prc_ctx[proc_idx].stCanvasInfo[i] = ctx->stCanvasInfo[i];
            } else if (ctx->stRegion.enType == OVERLAYEX_RGN) {
                rgn_prc_ctx[proc_idx].canvas_idx = ctx->canvas_idx;
                for (i = 0; i < ctx->stRegion.unAttr.stOverlayEx.u32CanvasNum; ++i)
                    rgn_prc_ctx[proc_idx].stCanvasInfo[i] = ctx->stCanvasInfo[i];
            }
            break;
        }
		rgn_prc_ctx[proc_idx].u32MaxNeedIon = ctx->u32MaxNeedIon;
    }

    pthread_mutex_unlock(&g_rgnlock);
    return CVI_SUCCESS;

RGN_ION_ALLOC_FAIL:
    if (ctx->stRegion.enType == OVERLAY_RGN) {
        for (i = 0; i < ctx->stRegion.unAttr.stOverlay.u32CanvasNum; ++i)
            if (ctx->stCanvasInfo[i].u64PhyAddr)
                rt_free_align((void *)ctx->stCanvasInfo[i].u64PhyAddr);
    } else if (ctx->stRegion.enType == OVERLAYEX_RGN) {
        for (i = 0; i < ctx->stRegion.unAttr.stOverlayEx.u32CanvasNum; ++i)
            if (ctx->stCanvasInfo[i].u64PhyAddr)
                rt_free_align((void *)ctx->stCanvasInfo[i].u64PhyAddr);
    }
RGN_FMT_INCORRECT:
    free(ctx);
RGN_CTX_MALLOC_FAIL:
    pthread_mutex_unlock(&g_rgnlock);
    return ret;
}

static CVI_S32 _rgn_destory(RGN_HANDLE Handle)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_U32 proc_idx;
    CVI_U8 i;

    pthread_mutex_lock(&g_rgnlock);

    if (!rgnHashmap) {
        CVI_TRACE_RGN(RGN_ERR, "rgnHashmap NULL.\n");
        pthread_mutex_unlock(&g_rgnlock);
        return CVI_ERR_RGN_SYS_NOTREADY;
    }

    ctx = hashmapRemove(rgnHashmap, (void *)(uintptr_t)Handle);
    if (ctx == NULL) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) is non-existent.\n", Handle);
        pthread_mutex_unlock(&g_rgnlock);
        return CVI_ERR_RGN_UNEXIST;
    }

    if (ctx->stChn.enModId != CVI_ID_BASE) {
        _rgn_update_hw(ctx, RGN_OP_REMOVE);
    }

    CVI_TRACE_RGN(RGN_INFO, "RGN_HANDLE(%d) canvas fmt(%d) size(%d * %d) stride(%d).\n", Handle, ctx->stCanvasInfo[0].enPixelFormat, ctx->stCanvasInfo[0].stSize.u32Width, ctx->stCanvasInfo[0].stSize.u32Height, ctx->stCanvasInfo[0].u32Stride);

    // update rgn proc info
    proc_idx = _rgn_proc_get_idx(Handle);
    memset(&rgn_prc_ctx[proc_idx], 0, sizeof(rgn_prc_ctx[proc_idx]));
    u32RgnNum--;

    pthread_mutex_unlock(&g_rgnlock);

    if (ctx->stChnAttr.enType == OVERLAY_RGN) {
        for (i = 0; i < ctx->stRegion.unAttr.stOverlay.u32CanvasNum; ++i)
            if (ctx->stCanvasInfo[i].u64PhyAddr)
                rt_free_align((void *)ctx->stCanvasInfo[i].u64PhyAddr);
    } else if (ctx->stChnAttr.enType == OVERLAYEX_RGN) {
        for (i = 0; i < ctx->stRegion.unAttr.stOverlayEx.u32CanvasNum; ++i)
            if (ctx->stCanvasInfo[i].u64PhyAddr)
                rt_free_align((void *)ctx->stCanvasInfo[i].u64PhyAddr);
    } else {
        if (ctx->stCanvasInfo[0].u64PhyAddr)
            rt_free_align((void *)ctx->stCanvasInfo[0].u64PhyAddr);
    }

    free(ctx);

    return CVI_SUCCESS;
}

static CVI_S32 _rgn_get_attr(RGN_HANDLE Handle, RGN_ATTR_S *pstRegion)
{
    struct cvi_rgn_ctx *ctx = NULL;

    CHECK_RGN_HANDLE(ctx, Handle);

    *pstRegion = ctx->stRegion;

    return CVI_SUCCESS;
}

static CVI_S32 _rgn_set_attr(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_U32 proc_idx;

    CHECK_RGN_HANDLE(ctx, Handle);

    proc_idx = _rgn_proc_get_idx(Handle);
    ctx->stRegion = rgn_prc_ctx[proc_idx].stRegion = *pstRegion;

    return CVI_SUCCESS;
}

static CVI_S32 _rgn_set_bit_map(RGN_HANDLE Handle, const BITMAP_S *pstBitmap)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_U32 bytesperline;
    RGN_CANVAS_INFO_S *pstCanvasInfo;
    CVI_U32 proc_idx;
    PIXEL_FORMAT_E pixelFormat;
    CVI_U32 canvasNum;
    SIZE_S rgnSize;
    CVI_U16 h;
    struct _rgn_get_ow_addr_cb_param cb_param;
    CVI_S32 cnt = 0;

    CHECK_RGN_HANDLE(ctx, Handle);

    proc_idx = _rgn_proc_get_idx(Handle);

    if (ctx->stRegion.enType != OVERLAY_RGN && ctx->stRegion.enType != OVERLAYEX_RGN) {
        CVI_TRACE_RGN(RGN_ERR, "Only Overlay/OverlayEx support. type(%d).\n", ctx->stRegion.enType);
        return CVI_ERR_RGN_NOT_SUPPORT;
    }

    if (ctx->canvas_get) {
        CVI_TRACE_RGN(RGN_ERR, "Need CVI_RGN_UpdateCanvas() first.\n");
        return CVI_ERR_RGN_NOT_PERM;
    }

    if (ctx->stChn.enModId == CVI_ID_BASE) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) not attached to any chn yet.\n", Handle);
        return CVI_ERR_RGN_NOT_CONFIG;
    }

    if (ctx->stRegion.enType == OVERLAY_RGN) {
        pixelFormat = ctx->stRegion.unAttr.stOverlay.enPixelFormat;
        canvasNum = ctx->stRegion.unAttr.stOverlay.u32CanvasNum;
        rgnSize = ctx->stRegion.unAttr.stOverlay.stSize;
    } else {
        pixelFormat = ctx->stRegion.unAttr.stOverlayEx.enPixelFormat;
        canvasNum = ctx->stRegion.unAttr.stOverlayEx.u32CanvasNum;
        rgnSize = ctx->stRegion.unAttr.stOverlayEx.stSize;
    }

    if ((pstBitmap->u32Width > rgnSize.u32Width) || (pstBitmap->u32Height > rgnSize.u32Height)) {
        CVI_TRACE_RGN(RGN_ERR, "size of bitmap (%d * %d) > region (%d * %d).\n", pstBitmap->u32Width, pstBitmap->u32Height, rgnSize.u32Width, rgnSize.u32Height);
        return CVI_ERR_RGN_ILLEGAL_PARAM;
    }

    if (pstBitmap->enPixelFormat != pixelFormat) {
        CVI_TRACE_RGN(RGN_ERR, "pxl-fmt of bitmap(%d) != region(%d).\n", pstBitmap->enPixelFormat, pixelFormat);
        return CVI_ERR_RGN_ILLEGAL_PARAM;
    }

    CVI_TRACE_RGN(RGN_INFO, "bitmap size(%d * %d) fmt(%d), region size(%d * %d) fmt(%d).\n", pstBitmap->u32Width, pstBitmap->u32Height, pstBitmap->enPixelFormat, rgnSize.u32Width, rgnSize.u32Height, pixelFormat);

    if (_rgn_get_bytesperline(pstBitmap->enPixelFormat, pstBitmap->u32Width, &bytesperline) != CVI_SUCCESS)
        return CVI_ERR_RGN_ILLEGAL_PARAM;

    // single/double buffer update per u32CanvasNum.
    if (canvasNum == 1)
        pstCanvasInfo = &ctx->stCanvasInfo[0];
    else {
        ctx->canvas_idx = 1 - ctx->canvas_idx;
        pstCanvasInfo = &ctx->stCanvasInfo[ctx->canvas_idx];
    }

    pstCanvasInfo->enPixelFormat = pstBitmap->enPixelFormat;
    pstCanvasInfo->stSize.u32Width = pstBitmap->u32Width;
    pstCanvasInfo->stSize.u32Height = pstBitmap->u32Height;
    pstCanvasInfo->u32Stride = ALIGN(bytesperline, 32);

    CVI_TRACE_RGN(RGN_INFO, "canvas fmt(%d) size(%d * %d) stride(%d).\n", pstCanvasInfo->enPixelFormat, pstCanvasInfo->stSize.u32Width, pstCanvasInfo->stSize.u32Height, pstCanvasInfo->u32Stride);

    CVI_TRACE_RGN(RGN_INFO, "canvas v_addr(0x%lx), p_addr(0x%llx) size(%x)\n", (uintptr_t)pstCanvasInfo->pu8VirtAddr, pstCanvasInfo->u64PhyAddr, pstCanvasInfo->u32Stride * pstCanvasInfo->stSize.u32Height);

    CVI_TRACE_RGN(RGN_INFO, "pstBitmap->pData(0x%lx)\n", (uintptr_t)pstBitmap->pData);

    if ((canvasNum > 1) && (ctx->stChn.enModId == CVI_ID_VPSS)) {
        cb_param.stChn = ctx->stChn;
        cb_param.handle = Handle;
        cb_param.layer = (ctx->stCanvasInfo[ctx->canvas_idx].bCompressed) ? RGN_ODEC_LAYER_VPSS : RGN_NORMAL_LAYER_VPSS;

        do {
            if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_GET_RGN_OW_ADDR, &cb_param) != 0) {
                CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_GET_RGN_OW_ADDR is failed\n");
                return CVI_ERR_RGN_ILLEGAL_PARAM;
            }
            cnt++;
            udelay(1000);
        } while ((cb_param.addr == pstCanvasInfo->u64PhyAddr) && (cnt <= 500));

        CVI_TRACE_RGN(RGN_INFO, "VPSS_CB_GET_RGN_OW_ADDR PhyAaddr:%lx cnt:%d.\n",
                      cb_param.addr, cnt);
        if (cb_param.addr == pstCanvasInfo->u64PhyAddr) {
            CVI_TRACE_RGN(RGN_INFO, "get a using canvas!\n");
        }
    }

    if (pstCanvasInfo->bCompressed) {
        if (memcpy(pstCanvasInfo->pu8VirtAddr, pstBitmap->pData,
                   pstCanvasInfo->u32CompressedSize) == NULL) {
            CVI_TRACE_RGN(RGN_ERR, "Compressed pstBitmap->pData, memcpy failed.\n");
            return CVI_ERR_RGN_ILLEGAL_PARAM;
        }
    } else {
        for (h = 0; h < pstCanvasInfo->stSize.u32Height; ++h) {
            if (memcpy(pstCanvasInfo->pu8VirtAddr + pstCanvasInfo->u32Stride * h,
                       pstBitmap->pData + bytesperline * h, bytesperline) == NULL) {
                CVI_TRACE_RGN(RGN_ERR, "pstBitmap->pData, memcpy failed.\n");
                return CVI_ERR_RGN_ILLEGAL_PARAM;
            }
        }
    }

    // update rgn proc info
    if (canvasNum == 1)
        rgn_prc_ctx[proc_idx].stCanvasInfo[0] = ctx->stCanvasInfo[0];
    else {
        rgn_prc_ctx[proc_idx].canvas_idx = ctx->canvas_idx;
        rgn_prc_ctx[proc_idx].stCanvasInfo[ctx->canvas_idx] = ctx->stCanvasInfo[ctx->canvas_idx];
    }

    // update hw.
    return _rgn_update_hw(ctx, RGN_OP_UPDATE);
}

static CVI_S32 _rgn_attach_to_chn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_U32 proc_idx;
    CVI_S32 s32Ret;

    CHECK_RGN_HANDLE(ctx, Handle);

    proc_idx = _rgn_proc_get_idx(Handle);

    if (ctx->stRegion.enType != pstChnAttr->enType) {
        CVI_TRACE_RGN(RGN_ERR, "type(%d) is different with chn type(%d).\n", ctx->stRegion.enType, pstChnAttr->enType);
        return CVI_ERR_RGN_ILLEGAL_PARAM;
    }

    if (pstChn->enModId != CVI_ID_VPSS && pstChn->enModId != CVI_ID_VO) {
        CVI_TRACE_RGN(RGN_ERR, "rgn can only be attached to vpss or vo.\n");
        return CVI_ERR_RGN_NOT_SUPPORT;
    }

    if (ctx->stChn.enModId != CVI_ID_BASE) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) has been attached to CHN(%s-%d-%d).\n", Handle, base_get_modname(ctx->stChn.enModId), ctx->stChn.s32DevId, ctx->stChn.s32ChnId);
        return CVI_ERR_RGN_NOT_CONFIG;
    }

    if (ctx->stRegion.enType == COVEREX_RGN || ctx->stRegion.enType == OVERLAYEX_RGN || ctx->stRegion.enType == MOSAIC_RGN) {
        if (pstChn->enModId != CVI_ID_VPSS || (sys_get_vpssmode() != VPSS_MODE_RGNEX)) {
            CVI_TRACE_RGN(RGN_ERR, "rgn extension only support on vpss rgnex mode.\n");
            return CVI_ERR_RGN_NOT_SUPPORT;
        }
    }

    if (_rgn_check_chn_attr(Handle, pstChn, pstChnAttr) != CVI_SUCCESS)
        return CVI_ERR_RGN_ILLEGAL_PARAM;

    pthread_mutex_lock(&g_rgnlock);
    if ((pstChnAttr->enType == COVEREX_RGN) || (pstChnAttr->enType == COVER_RGN)) {
        CVI_S32 ret = _rgn_update_cover_canvas(ctx, pstChnAttr);

        if (ret != CVI_SUCCESS) {
            CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) fill cover failed.\n", Handle);
            pthread_mutex_unlock(&g_rgnlock);
            return ret;
        }
    }
    ctx->stChnAttr = rgn_prc_ctx[proc_idx].stChnAttr = *pstChnAttr;
    ctx->stChn = rgn_prc_ctx[proc_idx].stChn = *pstChn;

    s32Ret = _rgn_update_hw(ctx, RGN_OP_INSERT);
    if (s32Ret == CVI_SUCCESS) {
        // only update rgn_prc_ctx after _rgn_update_hw success
        rgn_prc_ctx[proc_idx].bUsed = true;
    }
    pthread_mutex_unlock(&g_rgnlock);

    return s32Ret;
}

static CVI_S32 _rgn_detach_from_chn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_S32 ret = -EINVAL;
    CVI_U32 proc_idx;

    CHECK_RGN_HANDLE(ctx, Handle);

    proc_idx = _rgn_proc_get_idx(Handle);

    if (ctx->stChn.enModId == CVI_ID_BASE) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) not attached to any chn yet.\n", Handle);
        return CVI_ERR_RGN_NOT_CONFIG;
    }

    pthread_mutex_lock(&g_rgnlock);
    ret = _rgn_update_hw(ctx, RGN_OP_REMOVE);
    if (ret == CVI_SUCCESS) {
        ctx->stChn.enModId = rgn_prc_ctx[proc_idx].stChn.enModId = CVI_ID_BASE;
        rgn_prc_ctx[proc_idx].bUsed = false;
    }
    pthread_mutex_unlock(&g_rgnlock);

    return ret;
}

static CVI_S32 _rgn_set_display_attr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_U32 proc_idx;
    RGN_HANDLE *hdls;
    CVI_U8 size;
    RGN_HANDLE rgn_handle[32] = {0};
    RGN_CHN_ATTR_S stChnAttr;
    CVI_S32 ret = -EINVAL;

    CHECK_RGN_HANDLE(ctx, Handle);

    proc_idx = _rgn_proc_get_idx(Handle);

    if (ctx->stChn.enModId == CVI_ID_BASE || (ctx->stChn.enModId != pstChn->enModId || ctx->stChn.s32DevId != pstChn->s32DevId || ctx->stChn.s32ChnId != pstChn->s32ChnId)) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) is not attached on ModId(%d) s32DevId(%d) s32ChnId(%d)\n",
                      Handle, pstChn->enModId, pstChn->s32DevId, pstChn->s32ChnId);
        return CVI_ERR_RGN_NOT_CONFIG;
    }
    if (ctx->stChnAttr.enType != pstChnAttr->enType) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) enType not allowed to changed.\n", Handle);
        return CVI_ERR_RGN_NOT_PERM;
    }

    if (_rgn_check_chn_attr(Handle, pstChn, pstChnAttr) != CVI_SUCCESS)
        return CVI_ERR_RGN_ILLEGAL_PARAM;

    if ((ctx->stChnAttr.enType == COVEREX_RGN) || (ctx->stChnAttr.enType == COVER_RGN)) {
        ret = _rgn_update_cover_canvas(ctx, pstChnAttr);

        if (ret != CVI_SUCCESS) {
            CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) fill cover failed.\n", Handle);
            return ret;
        }
    }

    ret = _rgn_get_chn_info(ctx, &hdls, &size);
    if (ret != CVI_SUCCESS)
        return ret;

    memcpy(rgn_handle, hdls, sizeof(RGN_HANDLE) * size);
    memcpy(&stChnAttr, &ctx->stChnAttr, sizeof(RGN_CHN_ATTR_S));

    ctx->stChnAttr = rgn_prc_ctx[proc_idx].stChnAttr = *pstChnAttr;

    ret = _rgn_update_hw(ctx, RGN_OP_UPDATE);
    if (ret != CVI_SUCCESS) {
        memcpy(hdls, rgn_handle, sizeof(RGN_HANDLE) * size);
        ctx->stChnAttr = rgn_prc_ctx[proc_idx].stChnAttr = stChnAttr;
    }

    return ret;
}

static CVI_S32 _rgn_get_display_attr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr)
{
    struct cvi_rgn_ctx *ctx = NULL;

    CHECK_RGN_HANDLE(ctx, Handle);

    *pstChnAttr = ctx->stChnAttr;

    return CVI_SUCCESS;
}

static CVI_S32 _rgn_get_canvas_info(RGN_HANDLE Handle, RGN_CANVAS_INFO_S *pstCanvasInfo)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_U32 proc_idx;
    CVI_U32 canvasNum;
    struct _rgn_get_ow_addr_cb_param cb_param;
    CVI_S32 cnt = 0;

    CHECK_RGN_HANDLE(ctx, Handle);

    proc_idx = _rgn_proc_get_idx(Handle);

    if (ctx->stRegion.enType != OVERLAY_RGN && ctx->stRegion.enType != OVERLAYEX_RGN) {
        CVI_TRACE_RGN(RGN_ERR, "Only Overlay/OverlayEx support. type(%d).\n", ctx->stRegion.enType);
        return CVI_ERR_RGN_NOT_SUPPORT;
    }
    if (ctx->canvas_get) {
        CVI_TRACE_RGN(RGN_ERR, "Need CVI_RGN_UpdateCanvas() first.\n");
        return CVI_ERR_RGN_NOT_PERM;
    }

    if (ctx->stRegion.enType == OVERLAY_RGN)
        canvasNum = ctx->stRegion.unAttr.stOverlay.u32CanvasNum;
    else
        canvasNum = ctx->stRegion.unAttr.stOverlayEx.u32CanvasNum;

    if (canvasNum == 1)
        *pstCanvasInfo = ctx->stCanvasInfo[0];
    else {
        ctx->canvas_idx = rgn_prc_ctx[proc_idx].canvas_idx = 1 - ctx->canvas_idx;
        *pstCanvasInfo = ctx->stCanvasInfo[ctx->canvas_idx];
    }
    CVI_TRACE_RGN(RGN_INFO, "RGN_HANDLE(%d) canvas fmt(%d) size(%d * %d) stride(%d) compressed(%d).\n", Handle, pstCanvasInfo->enPixelFormat, pstCanvasInfo->stSize.u32Width, pstCanvasInfo->stSize.u32Height, pstCanvasInfo->u32Stride, pstCanvasInfo->bCompressed);

    if ((canvasNum > 1) && (ctx->stChn.enModId == CVI_ID_VPSS)) {
        CVI_TRACE_RGN(RGN_INFO, "canvas p_addr(%llx) v_addr(%lx).\n", pstCanvasInfo->u64PhyAddr, (uintptr_t)pstCanvasInfo->pu8VirtAddr);

        cb_param.stChn = ctx->stChn;
        cb_param.handle = Handle;
        cb_param.layer = (ctx->stCanvasInfo[ctx->canvas_idx].bCompressed) ? RGN_ODEC_LAYER_VPSS : RGN_NORMAL_LAYER_VPSS;

        do {
            if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_GET_RGN_OW_ADDR, &cb_param) != 0) {
                CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_GET_RGN_OW_ADDR is failed\n");
                return CVI_ERR_RGN_ILLEGAL_PARAM;
            }
            cnt++;
            udelay(1000);
        } while ((cb_param.addr == pstCanvasInfo->u64PhyAddr) && (cnt <= 500));

        CVI_TRACE_RGN(RGN_INFO, "VPSS_CB_GET_RGN_OW_ADDR PhyAaddr:%lx cnt:%d.\n",
                      cb_param.addr, cnt);
        CVI_TRACE_RGN(RGN_INFO, "ow addr(%lx).\n", cb_param.addr);
        if (cb_param.addr == pstCanvasInfo->u64PhyAddr) {
            CVI_TRACE_RGN(RGN_INFO, "get a using canvas!\n");
        }
    }

    ctx->canvas_get = CVI_TRUE;

    return CVI_SUCCESS;
}

static CVI_S32 _rgn_update_canvas(RGN_HANDLE Handle)
{
    struct cvi_rgn_ctx *ctx = NULL;
    CVI_S32 ret = -EINVAL;

    CHECK_RGN_HANDLE(ctx, Handle);

	if (ctx->stRegion.enType != OVERLAY_RGN && ctx->stRegion.enType != OVERLAYEX_RGN) {
		CVI_TRACE_RGN(RGN_ERR, "Only Overlay/OverlayEx support. type(%d).\n", ctx->stRegion.enType);
		return CVI_ERR_RGN_NOT_SUPPORT;
	}
	if (!ctx->canvas_get) {
		CVI_TRACE_RGN(RGN_ERR, "CVI_RGN_GetCanvasInfo() first.\n");
		return CVI_ERR_RGN_NOT_SUPPORT;
	}

	if (ctx->stCanvasInfo[0].bCompressed) {
		RGN_CANVAS_INFO_S *pstCanvasInfo = NULL;
		RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr = NULL;

		if (ctx->stCanvasInfo[ctx->canvas_idx].pu8VirtAddr == NULL) {
			CVI_TRACE_RGN(RGN_ERR, "ctx->stCanvasInfo[ctx->canvas_idx].pu8VirtAddr NULL!\n");
				return CVI_ERR_RGN_SYS_NOTREADY;
		}
		pstCanvasInfo = &ctx->stCanvasInfo[ctx->canvas_idx];
		pstCanvasCmprAttr = (RGN_CANVAS_CMPR_ATTR_S *)pstCanvasInfo->pu8VirtAddr;
		// if C906L osdc status error,
		// status and real bs_size will be returned in pstCanvasInfo->pu8VirtAddr
		//rt_hw_cpu_dcache_invalidate_local((uint64_t *)pstCanvasInfo->pu8VirtAddr, 8);
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, (void *)pstCanvasInfo->pu8VirtAddr, 8);
		pstCanvasInfo->u32CompressedSize =
				*((CVI_U32 *)pstCanvasInfo->pu8VirtAddr + 1);
		// restore bitstream header to origin format
		*((CVI_U32 *)pstCanvasInfo->pu8VirtAddr + 1) =
		((((pstCanvasCmprAttr->u32Width - 1) >> 1) & 0x7FFF) |
		(((pstCanvasCmprAttr->u32Height - 1) << 15) & 0x7FFF8000));
		//rt_hw_cpu_dcache_clean_and_invalidate_local((uint64_t *)pstCanvasInfo->pu8VirtAddr, 8);
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH | RT_HW_CACHE_INVALIDATE, (void *)pstCanvasInfo->pu8VirtAddr, 8);

		ctx->odec_data_valid = true;
	}

    ret = _rgn_update_hw(ctx, RGN_OP_UPDATE);
    ctx->canvas_get = CVI_FALSE;
    return ret;
}

/* CVI_RGN_Invert_Color - invert color per luma statistics of video content
 *   Chns' pixel-format should be YUV.
 *   RGN's pixel-format should be ARGB1555.
 *
 * @param Handle: RGN Handle
 * @param pstChn: the chn which rgn attached
 * @param pu32Color: rgn's content
 */
static CVI_S32 _rgn_invert_color(RGN_HANDLE Handle, MMF_CHN_S *pstChn, CVI_U32 *pu32Color)
{
    CVI_S32 ret = CVI_ERR_RGN_NOT_SUPPORT;

    return ret;
}

RGN_COMP_INFO_S s_ConvertInfo[RGN_COLOR_FMT_BUTT] = {
    {0, 4, 4, 4}, /*RGB444*/
    {4, 4, 4, 4}, /*ARGB4444*/
    {0, 5, 5, 5}, /*RGB555*/
    {0, 5, 6, 5}, /*RGB565*/
    {1, 5, 5, 5}, /*ARGB1555*/
    {0, 0, 0, 0}, /*RESERVED*/
    {0, 8, 8, 8}, /*RGB888*/
    {8, 8, 8, 8}, /*ARGB8888*/
    {4, 4, 4, 4}, /*ARGB4444*/
    {1, 5, 5, 5}, /*ARGB1555*/
    {8, 8, 8, 8}  /*ARGB8888*/
};

CVI_U16 RGN_MAKECOLOR_U16_A(CVI_U8 a, CVI_U8 r, CVI_U8 g, CVI_U8 b, RGN_COMP_INFO_S input_fmt)
{
    CVI_U8 a1, r1, g1, b1;
    CVI_U16 pixel;

    pixel = a1 = r1 = g1 = b1 = 0;
    a1 = ((input_fmt.alen - 4) > 0) ? a >> (input_fmt.alen - 4) : (input_fmt.alen ? a << (4 - input_fmt.alen) : a);
    r1 = ((input_fmt.rlen - 4) > 0) ? r >> (input_fmt.rlen - 4) : r;
    g1 = ((input_fmt.glen - 4) > 0) ? g >> (input_fmt.glen - 4) : g;
    b1 = ((input_fmt.blen - 4) > 0) ? b >> (input_fmt.blen - 4) : b;

    pixel = (b1 | (g1 << 4) | (r1 << 8 | (a1 << 12)));

    return pixel;
}

static CVI_S32 _rgn_set_chn_palette(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_PALETTE_S *pstPalette,
                                    RGN_RGBQUARD_S *pstInputPixelTable)
{
    struct cvi_rgn_ctx *ctx;
    struct cvi_rgn_lut_cfg lut_cfg;
    CVI_S32 ret = CVI_SUCCESS;
    CVI_U32 idx, u32Pixel, osd_layer;
    CVI_U16 u16Pixel;
    CVI_U8 r, g, b, a;
    CVI_U16 *pstOutputPixelTable;
    struct _rgn_lut_cb_param rgn_lut_arg;

    CHECK_RGN_HANDLE(ctx, Handle);

    if (pstPalette->lut_length > 256) {
        CVI_TRACE_RGN(RGN_ERR, "RGN_LUT_index(%d) is over maximum(256).\n", pstPalette->lut_length);
        return CVI_FAILURE;
    }

    osd_layer = (ctx->stCanvasInfo[ctx->canvas_idx].bCompressed) ? RGN_ODEC_LAYER_VPSS : RGN_NORMAL_LAYER_VPSS;

    pstOutputPixelTable = calloc(pstPalette->lut_length, sizeof(CVI_U16));
    if (pstOutputPixelTable == CVI_NULL) {
        CVI_TRACE_RGN(RGN_ERR, "malloc failed.\n");
        ret = CVI_ERR_RGN_NOMEM;
        return ret;
    }

    /* start color convert to ARGB4444 */
    for (idx = 0; idx < pstPalette->lut_length; idx++) {
        switch (pstPalette->pixelFormat) {
        case RGN_COLOR_FMT_RGB444:
        case RGN_COLOR_FMT_RGB555:
        case RGN_COLOR_FMT_RGB565:
        case RGN_COLOR_FMT_RGB888:
            a = 0xf; /*alpha*/
            r = pstInputPixelTable[idx].argbRed;
            g = pstInputPixelTable[idx].argbGreen;
            b = pstInputPixelTable[idx].argbBlue;
            break;

        case RGN_COLOR_FMT_RGB1555:
        case RGN_COLOR_FMT_RGB4444:
        case RGN_COLOR_FMT_RGB8888:
        case RGN_COLOR_FMT_ARGB8888:
        case RGN_COLOR_FMT_ARGB4444:
        case RGN_COLOR_FMT_ARGB1555:
        default:
            a = pstInputPixelTable[idx].argbAlpha;
            r = pstInputPixelTable[idx].argbRed;
            g = pstInputPixelTable[idx].argbGreen;
            b = pstInputPixelTable[idx].argbBlue;
            break;
        }

        u32Pixel = (b | g << 8 | r << 16 | a << 24);
        CVI_TRACE_RGN(RGN_INFO, "Input Table index(%d) (0x%x).\n", idx, u32Pixel);

        u16Pixel = RGN_MAKECOLOR_U16_A(a, r, g, b, s_ConvertInfo[pstPalette->pixelFormat]);
        CVI_TRACE_RGN(RGN_INFO, "Output data = (0x%x).\n", u16Pixel);

        pstOutputPixelTable[idx] = u16Pixel;
    }

    /* Write output_pixel_table and lut_length into cfg */
    lut_cfg.lut_addr = (void *)pstOutputPixelTable;
    lut_cfg.lut_length = pstPalette->lut_length;

    /* Get related device number to update LUT but RGNEX use device 0. */
    if (ctx->stRegion.enType == OVERLAY_RGN || ctx->stRegion.enType == COVER_RGN)
        lut_cfg.rgnex_en = false;
    else
        lut_cfg.rgnex_en = true;

    // Need to record for gop0 or gop1
    lut_cfg.lut_layer = osd_layer;

    /* Uupdate device LUT in kernel space. */
    rgn_lut_arg.stChn = ctx->stChn;
    rgn_lut_arg.lut_cfg = lut_cfg;
    if (_rgn_call_cb(E_MODULE_VPSS, VPSS_CB_SET_RGN_LUT_CFG, &rgn_lut_arg) != 0) {
        CVI_TRACE_RGN(RGN_ERR, "VPSS_CB_SET_LUT_CFG is failed\n");
        free(pstOutputPixelTable);
        return CVI_ERR_RGN_ILLEGAL_PARAM;
    }

    free(pstOutputPixelTable);

    return ret;
}

static int _rgn_sw_init(struct cvi_rgn_dev *rdev)
{
    UNUSED(rdev);
    return _rgn_init();
}

static int _rgn_release_op(struct cvi_rgn_dev *rdev)
{
    UNUSED(rdev);
    return _rgn_exit();
}

static int _rgn_create_proc(struct cvi_rgn_dev *rdev)
{
    int ret = -EINVAL;

    UNUSED(rdev);

    if (rgn_proc_init() < 0) {
        CVI_TRACE_RGN(RGN_ERR, "rgn proc init failed\n");
        goto err;
    }
    ret = CVI_SUCCESS;

err:
    return ret;
}

static void _rgn_destroy_proc(struct cvi_rgn_dev *rdev)
{
    rgn_proc_remove();
}

/*******************************************************
 *  File operations for core
 ******************************************************/
static long _rgn_s_ctrl(struct cvi_rgn_dev *rdev, struct rgn_ext_control *p)
{
    int ret = -EINVAL;
    RGN_ATTR_S stRegion;
    BITMAP_S stBitmap;
    MMF_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    CVI_U32 u32Color, id, sdk_id;
    RGN_PALETTE_S stPalette;
    RGN_HANDLE Handle;

    id = p->id;
    sdk_id = p->sdk_id;
    Handle = p->handle;

    switch (id) {
    case RGN_IOCTL_SC_SET_RGN: {
#if 0 // copy from cvi_vip_sc.c for V4L2_CID_DV_VIP_SC_SET_RGN
		if (memcpy(&sdev->vpss_chn_cfg[0].rgn_cfg, ext_ctrls[i].ptr,
						   sizeof(sdev->vpss_chn_cfg[0].rgn_cfg)) == NULL) {
			CVI_TRACE_VPSS(RGN_ERR, "ioctl-%#x, memcpy failed.\n", ext_ctrls[i].id);
			rc = -ENOMEM;
			break;
		}
		rc = 0;
#endif
    } // RGN_IOCTL_SC_SET_RGN:
    break;

    case RGN_IOCTL_DISP_SET_RGN: {
#if 0 // copy from cvi_vip_disp.c for V4L2_CID_DV_VIP_DISP_SET_RGN
		struct sclr_disp_timing *timing = sclr_disp_get_timing();
		struct sclr_size size;
		struct cvi_rgn_cfg cfg;

		if (memcpy(&cfg, ext_ctrls[i].ptr, sizeof(struct cvi_rgn_cfg)) == NULL) {
			dprintk(VIP_ERR, "ioctl-%#x, memcpy failed.\n", ext_ctrls[i].id);
			break;
		}

		size.w = timing->hfde_end - timing->hfde_start + 1;
		size.h = timing->vfde_end - timing->vfde_start + 1;
		rc = cvi_vip_set_rgn_cfg(SCL_GOP_DISP, &cfg, &size);
#endif
    } // RGN_IOCTL_DISP_SET_RGN:
    break;

    case RGN_IOCTL_SDK_CTRL: {
        switch (sdk_id) {
        case RGN_SDK_CREATE: {
            if (memcpy(&stRegion, p->ptr1, sizeof(RGN_ATTR_S)) == NULL)
                break;

            ret = _rgn_create(Handle, &stRegion);
        } break;

        case RGN_SDK_DESTORY: {
            ret = _rgn_destory(Handle);
        } break;

        case RGN_SDK_SET_ATTR: {
            if (memcpy(&stRegion, p->ptr1, sizeof(RGN_ATTR_S)) == NULL)
                break;

            ret = _rgn_set_attr(Handle, &stRegion);
        } break;

        case RGN_SDK_SET_BIT_MAP: {
            if (memcpy(&stBitmap, p->ptr1, sizeof(BITMAP_S)) == NULL)
                break;

            ret = _rgn_set_bit_map(Handle, &stBitmap);
        } break;

        case RGN_SDK_ATTACH_TO_CHN: {
            if ((memcpy(&stChn, p->ptr1, sizeof(MMF_CHN_S)) == NULL) ||
                (memcpy(&stChnAttr, p->ptr2, sizeof(RGN_CHN_ATTR_S)) == NULL))
                break;

            ret = _rgn_attach_to_chn(Handle, &stChn, &stChnAttr);
        } break;

        case RGN_SDK_DETACH_FROM_CHN: {
            if (memcpy(&stChn, p->ptr1, sizeof(MMF_CHN_S)) == NULL)
                break;

            ret = _rgn_detach_from_chn(Handle, &stChn);
        } break;

        case RGN_SDK_SET_DISPLAY_ATTR: {
            if ((memcpy(&stChn, p->ptr1, sizeof(MMF_CHN_S)) == NULL) ||
                (memcpy(&stChnAttr, p->ptr2, sizeof(RGN_CHN_ATTR_S)) == NULL))
                break;

            ret = _rgn_set_display_attr(Handle, &stChn, &stChnAttr);
        } break;

        case RGN_SDK_UPDATE_CANVAS: {
            ret = _rgn_update_canvas(Handle);
        } break;

        case RGN_SDK_INVERT_COLOR: {
            if ((memcpy(&stChn, p->ptr1, sizeof(MMF_CHN_S)) == NULL) ||
                (memcpy(&u32Color, p->ptr2, sizeof(CVI_U32)) == NULL))
                break;

            ret = _rgn_invert_color(Handle, &stChn, &u32Color);
        } break;

        case RGN_SDK_SET_CHN_PALETTE: {
            RGN_RGBQUARD_S *pstInputPixelTable;

            if ((memcpy(&stChn, p->ptr1, sizeof(MMF_CHN_S)) == NULL) ||
                (memcpy(&stPalette, p->ptr2, sizeof(RGN_PALETTE_S)) == NULL))
                break;

            pstInputPixelTable = calloc(stPalette.lut_length, sizeof(RGN_RGBQUARD_S));
            if (pstInputPixelTable == CVI_NULL) {
                CVI_TRACE_RGN(RGN_ERR, "malloc failed.\n");
                return CVI_ERR_RGN_NOMEM;
            }
            if (memcpy(pstInputPixelTable, (void *)(stPalette.pstPaletteTable),
                       stPalette.lut_length * sizeof(RGN_RGBQUARD_S)) == NULL) {
                CVI_TRACE_RGN(RGN_ERR, "lut memcpy failed.\n");
                ret = CVI_ERR_RGN_NOMEM;
                free(pstInputPixelTable);
                break;
            }

            ret = _rgn_set_chn_palette(Handle, &stChn, &stPalette, pstInputPixelTable);
            free(pstInputPixelTable);
        } break;

        default:
            break;
        } // switch (sdk_id)

    } // case RGN_IOCTL_SDK_CTRL:
    break;

    default:
        break;
    } // switch (id)

    return ret;
}

static long _rgn_g_ctrl(struct cvi_rgn_dev *rdev, struct rgn_ext_control *p)
{
    int ret = -EINVAL;
    RGN_ATTR_S stRegion;
    MMF_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_CANVAS_INFO_S stCanvasInfo;
	struct cvi_rgn_ctx *pCtx;
    CVI_U32 id, sdk_id;
    RGN_HANDLE Handle;

    id = p->id;
    sdk_id = p->sdk_id;
    Handle = p->handle;

    switch (id) {
    case RGN_IOCTL_SDK_CTRL: {
        switch (sdk_id) {
        case RGN_SDK_GET_ATTR: {
            ret = _rgn_get_attr(Handle, &stRegion);

            if (memcpy(p->ptr1, &stRegion, sizeof(RGN_ATTR_S)) == NULL)
                break;
        } break;

        case RGN_SDK_GET_DISPLAY_ATTR: {
            if (memcpy(&stChn, p->ptr1, sizeof(MMF_CHN_S)) == NULL)
                break;

            ret = _rgn_get_display_attr(Handle, &stChn, &stChnAttr);

            if (memcpy(p->ptr2, &stChnAttr, sizeof(RGN_CHN_ATTR_S)) == NULL)
                break;
        } break;

        case RGN_SDK_GET_CTX: {
            pCtx = hashmapGet(rgnHashmap, (void *)(uintptr_t)Handle);
            if (pCtx == NULL) {
                CVI_TRACE_RGN(RGN_ERR, "RGN_HANDLE(%d) not existed.\n", Handle);
                ret = CVI_ERR_RGN_UNEXIST;
                break;
			}
            ret = CVI_SUCCESS;
            if (memcpy(p->ptr1, pCtx, sizeof(struct cvi_rgn_ctx)) == NULL)
                break;
        } break;

        case RGN_SDK_GET_CANVAS_INFO: {
            ret = _rgn_get_canvas_info(Handle, &stCanvasInfo);

            if (memcpy(p->ptr1, &stCanvasInfo, sizeof(RGN_CANVAS_INFO_S)) == NULL)
                break;
        } break;

        default:
            break;
        } // switch (sdk_id)

    } // case RGN_IOCTL_SDK_CTRL:
    break;

    default:
        break;
    } // switch (id)

    return ret;
}

long rgn_ioctl(u_int cmd, u_long arg)
{
    int ret = -EINVAL;
    struct cvi_rgn_dev *rdev = gRdev;
    struct rgn_ext_control p;

    if (memcpy(&p, (void *)arg, sizeof(struct rgn_ext_control)) == NULL)
        return ret;

    switch (cmd) {
    case RGN_IOC_S_CTRL:
        ret = _rgn_s_ctrl(rdev, &p);
        break;
    case RGN_IOC_G_CTRL:
        ret = _rgn_g_ctrl(rdev, &p);
        break;
    default:
        ret = -1;
        break;
    }

    if (memcpy((void *)arg, &p, sizeof(struct rgn_ext_control)) == NULL)
        return ret;

    return ret;
}

int rgn_open(void)
{
    int ret = 0;

    if (!atomic_read(&dev_open_cnt)) {
        struct cvi_rgn_dev *rdev = gRdev;

        _rgn_sw_init(rdev);

        CVI_TRACE_RGN(RGN_INFO, "-\n");
        ret = CVI_SUCCESS;
    }

    atomic_inc(&dev_open_cnt);

    return ret;
}

int rgn_close(void)
{
    int ret = 0;

    atomic_dec(&dev_open_cnt);

    if (!atomic_read(&dev_open_cnt)) {
        struct cvi_rgn_dev *rdev = gRdev;

        /* This should move to stop streaming */
        _rgn_release_op(rdev);

        CVI_TRACE_RGN(RGN_INFO, "-\n");

        ret = CVI_SUCCESS;
    }

    return ret;
}

int rgn_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg)
{
    int ret = 0;

    switch (cmd) {
    default:
        break;
    }

    return ret;
}

/*******************************************************
 *  Common interface for core
 ******************************************************/
int rgn_create_instance(struct cvi_rgn_dev *prdev)
{
    int ret = -EINVAL;
    struct cvi_rgn_dev *rdev = prdev;

    if (_rgn_create_proc(rdev)) {
        CVI_TRACE_RGN(RGN_ERR, "Failed to create proc\n");
        goto err;
    }
    gRdev = prdev;
    ret = CVI_SUCCESS;

err:
    return ret;
}

int rgn_destroy_instance(struct cvi_rgn_dev *prdev)
{
    int ret = -EINVAL;
    struct cvi_rgn_dev *rdev = prdev;

    _rgn_destroy_proc(rdev);
    ret = CVI_SUCCESS;

    return ret;
}
