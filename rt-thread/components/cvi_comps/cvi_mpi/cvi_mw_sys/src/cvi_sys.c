#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <queue.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cvi_efuse.h"
#include "cvi_errno.h"
#include "cvi_mw_base.h"
#include "cvi_sys.h"
#include "cvi_tpu_proc.h"
#include "sys_ioctl.h"
#include <cvi_base.h>
#include <cvi_tpu_interface.h>

CVI_CHAR const *log_name[8] = {
    (CVI_CHAR *)"EMG", (CVI_CHAR *)"ALT", (CVI_CHAR *)"CRI", (CVI_CHAR *)"ERR",
    (CVI_CHAR *)"WRN", (CVI_CHAR *)"NOT", (CVI_CHAR *)"INF", (CVI_CHAR *)"DBG"};

static MMF_VERSION_S mmf_version;
VI_VPSS_MODE_S stVIVPSSMode;
VPSS_MODE_S stVPSSMode;
CVI_S32 log_levels[CVI_ID_BUTT] = {[0 ... CVI_ID_BUTT - 1] = CVI_DBG_WARN};

#define _GENERATE_STRING(STRING) (#STRING),
static const char *const MOD_STRING[] = FOREACH_MOD(_GENERATE_STRING);
#define CVI_GET_MOD_NAME(id) (id < CVI_ID_BUTT) ? MOD_STRING[id] : "UNDEF"

extern void soc_dcache_invalid_range(unsigned long addr, uint32_t size);
extern void soc_dcache_clean_invalid_range(unsigned long addr, uint32_t size);

CVI_S32 CVI_SYS_Init(void)
{
    CVI_S32 s32ret = CVI_SUCCESS;
    CVI_U32 sys_init = 0;

    sys_get_sys_init(&sys_init);
    if (sys_init == 0) {
        cvi_sys_open();

        // it always success;
#if 0
		vi_dev_open();

		vpss_dev_open();

		for (CVI_U8 i = 0; i < VI_MAX_PIPE_NUM; ++i)
			stVIVPSSMode.aenMode[i] = VI_OFFLINE_VPSS_OFFLINE;
		CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
		stVPSSMode.enMode = VPSS_MODE_SINGLE;
		for (CVI_U8 i = 0; i < VPSS_IP_NUM; ++i)
			stVPSSMode.aenInput[i] = VPSS_INPUT_MEM;
		memset(&mmf_version, 0, sizeof(mmf_version));
#endif
        CVI_SYS_GetVersion(&mmf_version);
        // CVI_SYS_SetVPSSModeEx(&stVPSSMode);
#ifndef __CV180X__
#if (CONFIG_APP_VO_SUPPORT)
        vo_dev_open();
#endif
#endif
    }

    sys_set_sys_init();
    CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

    return s32ret;
}

CVI_S32 CVI_SYS_Exit(void)
{
    CVI_S32 s32ret = CVI_SUCCESS;

    // vpss_dev_close();

    // vi_dev_close();

    CVI_TRACE_SYS(CVI_DBG_INFO, "+\n");
    cvi_sys_close();

    CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

    return s32ret;
}

CVI_S32 _CVI_SYS_BindIOCtl(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn, CVI_U8 is_bind)
{
    CVI_S32 ret = 0;
    struct sys_bind_cfg bind_cfg;

    memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
    bind_cfg.is_bind = is_bind;
    bind_cfg.mmf_chn_src = *pstSrcChn;
    bind_cfg.mmf_chn_dst = *pstDestChn;

    ret = cvi_sys_ioctl(SYS_SET_BINDCFG, &bind_cfg);
    if (ret)
        CVI_TRACE_SYS(CVI_DBG_ERR, "_CVI_SYS_BindIOCtl()failed\n");

    return ret;
}

CVI_S32 CVI_SYS_Bind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
    return _CVI_SYS_BindIOCtl(pstSrcChn, pstDestChn, 1);
}

CVI_S32 CVI_SYS_UnBind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
    return _CVI_SYS_BindIOCtl(pstSrcChn, pstDestChn, 0);
}

CVI_S32 CVI_SYS_GetBindbyDest(const MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn)
{
    CVI_S32 ret = 0;
    struct sys_bind_cfg bind_cfg;

    memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
    bind_cfg.get_by_src = 0;
    bind_cfg.mmf_chn_dst = *pstDestChn;

    ret = cvi_sys_ioctl(SYS_GET_BINDCFG, &bind_cfg);
    if (ret) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_GetBindbyDest() failed\n");
        return ret;
    }

    memcpy(pstSrcChn, &bind_cfg.mmf_chn_src, sizeof(MMF_CHN_S));
    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetBindbySrc(const MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
    CVI_S32 ret = 0;
    struct sys_bind_cfg bind_cfg;

    memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
    bind_cfg.get_by_src = 1;
    bind_cfg.mmf_chn_src = *pstSrcChn;

    ret = cvi_sys_ioctl(SYS_GET_BINDCFG, &bind_cfg);

    if (ret) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_GetBindbySrc() failed\n");
        return ret;
    }

    memcpy(pstBindDest, &bind_cfg.bind_dst, sizeof(MMF_BIND_DEST_S));
    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetVersion(MMF_VERSION_S *pstVersion)
{
    MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVersion);

#ifndef MMF_VERSION
#define MMF_VERSION (CVI_CHIP_NAME MMF_VER_PRIX MK_VERSION(VER_X, VER_Y, VER_Z) VER_D)
#endif
    snprintf(pstVersion->version, VERSION_NAME_MAXLEN, "%s-%s", MMF_VERSION, "64bit");
    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetChipId(CVI_U32 *pu32ChipId)
{
    static CVI_U32 id = 0xffffffff;

    if (id == 0xffffffff) {
        CVI_U32 tmp = 0xffffffff;

        if (cvi_sys_ioctl(SYS_READ_CHIP_ID, &tmp) < 0) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl SYS_READ_CHIP_ID failed\n");
            return CVI_FAILURE;
        }

        id = tmp;
    }

    *pu32ChipId = id;
    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetPowerOnReason(CVI_U32 *pu32PowerOnReason)
{
    CVI_U32 ret_val = 0x0;
    CVI_U32 reason = 0x0;

    if (cvi_sys_ioctl(SYS_READ_CHIP_PWR_ON_REASON, &reason) < 0) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_READ_CHIP_PWR_ON_REASON failed\n");
        return CVI_FAILURE;
    }

    switch (reason) {
    case E_CHIP_PWR_ON_COLDBOOT:
        ret_val = CVI_COLDBOOT;
        break;
    case E_CHIP_PWR_ON_WDT:
        ret_val = CVI_WDTBOOT;
        break;
    case E_CHIP_PWR_ON_SUSPEND:
        ret_val = CVI_SUSPENDBOOT;
        break;
    case E_CHIP_PWR_ON_WARM_RST:
        ret_val = CVI_WARMBOOT;
        break;
    default:
        CVI_TRACE_SYS(CVI_DBG_ERR, "unknown reason (%#x)\n", reason);
        return CVI_ERR_SYS_NOT_PERM;
        break;
    }

    *pu32PowerOnReason = ret_val;
    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetChipVersion(CVI_U32 *pu32ChipVersion)
{
    static CVI_U32 version = 0xffffffff;

    if (version == 0xffffffff) {
        CVI_U32 tmp = 0;

        if (cvi_sys_ioctl(SYS_READ_CHIP_VERSION, &tmp) < 0) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl SYS_READ_CHIP_VERSION failed\n");
            return CVI_FAILURE;
        }

        switch (tmp) {
        case E_CHIPVERSION_U01:
            version = CVIU01;
            break;
        case E_CHIPVERSION_U02:
            version = CVIU02;
            break;
        default:
            CVI_TRACE_SYS(CVI_DBG_ERR, "unknown version(%#x)\n", tmp);
            return CVI_ERR_SYS_NOT_PERM;
            break;
        }
    }

    *pu32ChipVersion = version;
    return CVI_SUCCESS;
}

static CVI_S32 _SYS_IonAlloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
                             CVI_U32 u32Len, CVI_BOOL cached, const CVI_CHAR *name)
{
    struct sys_ion_data ion_data;

    if (u32Len == 0) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "can't alloc size 0.\n");
        return -1;
    }

    memset(&ion_data, 0, sizeof(struct sys_ion_data));
    ion_data.size = u32Len;
    ion_data.cached = cached ? 1 : 0;
    ion_data.dmabuf_fd = 0; // Not used in this context
	// Set buffer as "anonymous" when user is passing null pointer.
	if (name) {
		strncpy((char *)(ion_data.name), name, MAX_ION_BUFFER_NAME - 1);
		ion_data.name[MAX_ION_BUFFER_NAME - 1] = '\0';
	} else {
		strncpy((char *)(ion_data.name), "anonymous", MAX_ION_BUFFER_NAME);
		ion_data.name[MAX_ION_BUFFER_NAME - 1] = '\0';
	}


    if (cvi_sys_ioctl(SYS_ION_ALLOC, &ion_data) < 0) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_ION_ALLOC failed\n");
        return -1;
    }

    *pu64PhyAddr = ion_data.addr_p;

    if (ppVirAddr)
        *ppVirAddr = (void *)ion_data.addr_p;

    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonAlloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr, const CVI_CHAR *strName, CVI_U32 u32Len)
{
    MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);
    return _SYS_IonAlloc(pu64PhyAddr, ppVirAddr, u32Len, CVI_FALSE, strName);
}

/* CVI_SYS_IonAlloc_Cached - acquire buffer of u32Len from ion
 *
 * @param pu64PhyAddr: the phy-address of the buffer
 * @param ppVirAddr: the cached vir-address of the buffer
 * @param strName: the name of the buffer
 * @param u32Len: the length of the buffer acquire
 * @return CVI_SUCCES if ok
 */
CVI_S32 CVI_SYS_IonAlloc_Cached(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
                                const CVI_CHAR *strName, CVI_U32 u32Len)
{
    MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);
    return _SYS_IonAlloc(pu64PhyAddr, ppVirAddr, u32Len, CVI_TRUE, strName);
}

CVI_S32 CVI_SYS_IonFree(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr)
{
    UNUSED(pVirAddr);
    struct sys_ion_data ion_data;

    ion_data.addr_p = u64PhyAddr;

    if (cvi_sys_ioctl(SYS_ION_FREE, &ion_data) < 0) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_ION_FREE failed\n");
        return -1;
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonFlushCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)u64PhyAddr, u32Len);
    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonInvalidateCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, (void *)u64PhyAddr, u32Len);
    return CVI_SUCCESS;
}

#if 0
CVI_S32 CVI_SYS_GetMemoryStatics(ION_MM_STATICS_S *pstStatics)
{
}
#endif
CVI_S32 CVI_SYS_SetVIVPSSMode(const VI_VPSS_MODE_S *pstVIVPSSMode)
{
    MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

    return sys_set_vivpssmode(pstVIVPSSMode);
}

CVI_S32 CVI_SYS_GetVIVPSSMode(VI_VPSS_MODE_S *pstVIVPSSMode)
{
    MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

    return sys_get_vivpssmode(pstVIVPSSMode);
}

CVI_S32 CVI_SYS_SetVPSSMode(VPSS_MODE_E enVPSSMode)
{
    if (sys_set_vpssmode(enVPSSMode))
        return -1;

    stVPSSMode.enMode = enVPSSMode;
    return CVI_SYS_SetVPSSModeEx(&stVPSSMode);
}

VPSS_MODE_E CVI_SYS_GetVPSSMode(void)
{
    VPSS_MODE_E enMode;

    if (sys_get_vpssmode(&enMode))
        return -1;

    return enMode;
}

CVI_S32 CVI_SYS_SetVPSSModeEx(const VPSS_MODE_S *pstVPSSMode)
{
    MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVPSSMode);

    return sys_set_vpssmodeex(pstVPSSMode);
}

CVI_S32 CVI_SYS_GetVPSSModeEx(VPSS_MODE_S *pstVPSSMode)
{
    VPSS_MODE_S vpss_mode;

    if (sys_get_vpssmodeex(&vpss_mode))
        return -1;

    *pstVPSSMode = vpss_mode;

    return CVI_SUCCESS;
}

const CVI_CHAR *CVI_SYS_GetModName(MOD_ID_E id)
{
    return CVI_GET_MOD_NAME(id);
}

CVI_S32 CVI_LOG_SetLevelConf(LOG_LEVEL_CONF_S *pstConf)
{
    MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

    if (pstConf->enModId >= CVI_ID_BUTT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "Invalid ModId(%d)\n", pstConf->enModId);
        return CVI_ERR_SYS_ILLEGAL_PARAM;
    }

    log_levels[pstConf->enModId] = pstConf->s32Level;
    return CVI_SUCCESS;
}

CVI_S32 CVI_LOG_GetLevelConf(LOG_LEVEL_CONF_S *pstConf)
{
    MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

    if (pstConf->enModId >= CVI_ID_BUTT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "Invalid ModId(%d)\n", pstConf->enModId);
        return CVI_ERR_SYS_ILLEGAL_PARAM;
    }

    pstConf->s32Level = log_levels[pstConf->enModId];
    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetCurPTS(CVI_U64 *pu64CurPTS)
{
    MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64CurPTS);

    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    *pu64CurPTS = (CVI_U64)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_VI_Open(void)
{
    CVI_S32 s32ret = CVI_SUCCESS;

    s32ret = vi_dev_open();
    if (s32ret != CVI_SUCCESS) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "base_vi_open failed\n");
        return CVI_ERR_SYS_NOTREADY;
    }

    return s32ret;
}

CVI_S32 CVI_SYS_VI_Close(void)
{
    CVI_S32 s32ret = CVI_SUCCESS;

    s32ret = vi_dev_close();
    if (s32ret != CVI_SUCCESS) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "base_vi_close failed\n");
        return CVI_ERR_SYS_NOTREADY;
    }

    return s32ret;
}

pthread_mutex_t tdma_pio_seq_lock = PTHREAD_MUTEX_INITIALIZER;
static uint32_t tdma_pio_seq;

CVI_S32 CVI_SYS_TDMACopy(CVI_U64 u64PhyDst, CVI_U64 u64PhySrc, CVI_U32 u32Len)
{
#define TDMA2D_LEN_LIMIT 0xFFFFFFFF

    struct cvi_tpu_device *ndev = NULL;
    struct cvi_tdma_copy_arg tdma_ioctl;
    struct cvi_tdma_wait_arg wait_ioctl;

    if (u32Len >= TDMA2D_LEN_LIMIT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_TDMACopy() input param can't be supported\n");
        return CVI_ERR_SYS_NOT_SUPPORT;
    }

    ndev = cvi_tpu_open();
    if (ndev == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "tpu fd open failed.\n");
        return CVI_ERR_SYS_NOTREADY;
    }

    memset(&tdma_ioctl, 0, sizeof(struct cvi_tdma_copy_arg));

    pthread_mutex_lock(&tdma_pio_seq_lock);
    tdma_pio_seq++;
    tdma_ioctl.seq_no = tdma_pio_seq;
    pthread_mutex_unlock(&tdma_pio_seq_lock);

    tdma_ioctl.paddr_src = u64PhySrc;
    tdma_ioctl.paddr_dst = u64PhyDst;
    tdma_ioctl.leng_bytes = u32Len;
    cvi_tpu_ioctl(ndev, CVITPU_SUBMIT_PIO, (CVI_U64)&tdma_ioctl);

    // wait finished
    wait_ioctl.seq_no = tdma_ioctl.seq_no;
    cvi_tpu_ioctl(ndev, CVITPU_WAIT_PIO, (CVI_U64)&wait_ioctl);

    if (wait_ioctl.ret)
        CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_TDMACopy wait failed\n");

    cvi_tpu_close();
    return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_TDMACopy2D(CVI_TDMA_2D_S *param)
{
#define TDMA2D_W_LIMIT 0x10000
#define TDMA2D_H_LIMIT 0x10000

    struct cvi_tpu_device *ndev = NULL;
    struct cvi_tdma_copy_arg tdma_ioctl;
    struct cvi_tdma_wait_arg wait_ioctl;

    if (param->stride_bytes_src < param->w_bytes ||
        param->stride_bytes_dst < param->w_bytes ||
        param->w_bytes >= TDMA2D_W_LIMIT ||
        param->h >= TDMA2D_H_LIMIT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_TDMACopy2D() input param can't be supported\n");
        return CVI_ERR_SYS_NOT_SUPPORT;
    }

    ndev = cvi_tpu_open();
    if (ndev == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "tpu fd open failed.\n");
        return CVI_ERR_SYS_NOTREADY;
    }

    memset(&tdma_ioctl, 0, sizeof(struct cvi_tdma_copy_arg));

    pthread_mutex_lock(&tdma_pio_seq_lock);
    tdma_pio_seq++;
    tdma_ioctl.seq_no = tdma_pio_seq;
    pthread_mutex_unlock(&tdma_pio_seq_lock);

    tdma_ioctl.enable_2d = 1;
    tdma_ioctl.paddr_src = param->paddr_src;
    tdma_ioctl.paddr_dst = param->paddr_dst;
    tdma_ioctl.h = param->h;
    tdma_ioctl.w_bytes = param->w_bytes;
    tdma_ioctl.stride_bytes_src = param->stride_bytes_src;
    tdma_ioctl.stride_bytes_dst = param->stride_bytes_dst;
    cvi_tpu_ioctl(ndev, CVITPU_SUBMIT_PIO, (CVI_U64)&tdma_ioctl);

    // wait finished
    wait_ioctl.seq_no = tdma_ioctl.seq_no;
    cvi_tpu_ioctl(ndev, CVITPU_WAIT_PIO, (CVI_U64)&wait_ioctl);

    if (wait_ioctl.ret)
        CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_TDMACopy2D wait failed\n");

    cvi_tpu_close();
    return CVI_SUCCESS;
}

void *CVI_SYS_Mmap(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
    return (void *)u64PhyAddr;
}

CVI_S32 CVI_SYS_Munmap(void *pVirAddr, CVI_U32 u32Size)
{
    return CVI_SUCCESS;
}

// CVI_S32 CVI_SYS_IonInvalidateCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr,
// CVI_U32 u32Len)
// {
//  return CVI_SUCCESS;
// }

void *CVI_SYS_MmapCache(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
    return (void *)u64PhyAddr;
}