#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "cvi_errno.h"
#include "cvi_debug.h"
#include "devmem.h"
#include "hashmap.h"
#include "platform_sys.h"
#include "base_uapi.h"
#include "sys_uapi.h"
#include "sys_internal.h"
#include "gdc_mesh.h"

#define MMF_VERSION  (CVI_CHIP_NAME MMF_VER_PRIX MK_VERSION(VER_X, VER_Y, VER_Z) VER_D)


static atomic_bool sys_inited = ATOMIC_VAR_INIT(false);
static int devm_fd = -1, devm_cached_fd = -1;
static int ionFd = -1;
static void *shared_mem;
static MMF_VERSION_S *mmf_version;
CVI_S32 *log_levels;
CVI_CHAR const *log_name[8] = {
	(CVI_CHAR *)"EMG", (CVI_CHAR *)"ALT", (CVI_CHAR *)"CRI", (CVI_CHAR *)"ERR",
	(CVI_CHAR *)"WRN", (CVI_CHAR *)"NOT", (CVI_CHAR *)"INF", (CVI_CHAR *)"DBG"
};

static CVI_S32 _sys_mmap(void)
{
	if (shared_mem != NULL) {
		CVI_TRACE_SYS(CVI_DBG_INFO, "already done mmap\n");
		return CVI_SUCCESS;
	}

	shared_mem = base_get_shm();
	if (shared_mem == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "base_get_shm failed!\n");
		return CVI_ERR_SYS_NOMEM;
	}

	log_levels = (CVI_S32 *)(shared_mem + BASE_LOG_LEVEL_OFFSET);

	mmf_version = (MMF_VERSION_S *)(shared_mem + BASE_VERSION_INFO_OFFSET);
	memset(mmf_version, 0, VERSION_INFO_RSV_SIZE);
	platform_sys_getversion(mmf_version);

	return CVI_SUCCESS;
}

static CVI_S32 _sys_unmmap(void)
{
	if (shared_mem == NULL) {
		CVI_TRACE_SYS(CVI_DBG_DEBUG, "No need to unmap\n");
		return CVI_SUCCESS;
	}

	base_release_shm();
	shared_mem = NULL;
	log_levels = NULL;
	mmf_version = NULL;

	return CVI_SUCCESS;
}

static CVI_S32 _sys_devmem_open(void)
{
	if (devm_fd < 0)
		devm_fd = devm_open();

	if (devm_cached_fd < 0)
		devm_cached_fd = devm_open_cached();

	if (devm_fd < 0 || devm_cached_fd < 0) {
		perror("devmem open failed\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _sys_devmem_close(void)
{
	if (devm_fd < 0 || devm_cached_fd < 0)
		return CVI_SUCCESS;

	devm_close(devm_fd);
	devm_fd = -1;
	devm_close(devm_cached_fd);
	devm_cached_fd = -1;
	return CVI_SUCCESS;
}

static CVI_S32 _sys_bind_ioctl(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn, CVI_U8 is_bind)
{
	CVI_S32 fd = 0;
	CVI_S32 ret = 0;
	struct sys_bind_cfg bind_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
	bind_cfg.is_bind = is_bind;
	bind_cfg.mmf_chn_src = *pstSrcChn;
	bind_cfg.mmf_chn_dst = *pstDestChn;

	ret = ioctl(fd, BASE_SET_BINDCFG, &bind_cfg);

	if (ret)
		CVI_TRACE_SYS(CVI_DBG_ERR, "_sys_bind_ioctl()failed\n");

	return ret;
}

static CVI_S32 _sys_ion_malloc(struct sys_ion_data *para)
{
	CVI_S32 fd = -1;
	CVI_S32 ret;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;


	ret = ioctl(fd, BASE_ION_ALLOC, para);
	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl BASE_ION_ALLOC failed\n");
		return ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _sys_ion_free(struct sys_ion_data *para)
{
	CVI_S32 fd = -1;
	CVI_S32 ret;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	ret = ioctl(fd, BASE_ION_FREE, para);
	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl BASE_ION_FREE failed\n");
		return ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _sys_ion_alloc_cache(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
			     CVI_U32 u32Len, CVI_BOOL cached, const CVI_CHAR *name)
{
	struct sys_ion_data ion_data;

	ion_data.size = u32Len;
	ion_data.cached = cached;
	// Set buffer as "anonymous" when user is passing null pointer.
	if (name) {
		strncpy((char *)(ion_data.name), name, MAX_ION_BUFFER_NAME - 1);
		ion_data.name[MAX_ION_BUFFER_NAME - 1] = '\0';
	} else {
		strncpy((char *)(ion_data.name), "anonymous", MAX_ION_BUFFER_NAME);
	}

	if (_sys_ion_malloc(&ion_data) != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "alloc failed.\n");
		return CVI_ERR_SYS_NOMEM;
	}

	*pu64PhyAddr = ion_data.addr_p;

	if (ppVirAddr) {
		if (cached)
			*ppVirAddr = platform_sys_mmapcache(*pu64PhyAddr, u32Len);
		else
			*ppVirAddr = platform_sys_mmap(*pu64PhyAddr, u32Len);
		if (*ppVirAddr == NULL) {
			_sys_ion_free(&ion_data);
			CVI_TRACE_SYS(CVI_DBG_ERR, "mmap failed. (%s)\n", strerror(errno));
			return CVI_ERR_SYS_REMAPPING;
		}
	}
	return CVI_SUCCESS;
}


CVI_S32 platform_sys_init(CVI_VOID)
{
	CVI_S32 s32ret = CVI_SUCCESS;
	bool expect = false;

	// Only init once until exit.
	if (!atomic_compare_exchange_strong(&sys_inited, &expect, true))
		return CVI_SUCCESS;

	CVI_TRACE_SYS(CVI_DBG_INFO, "+\n");

	if (_sys_devmem_open() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "devmem open failed.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	if (_sys_mmap() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "_sys_mmap failed.\n");
		return CVI_ERR_SYS_NOMEM;
	}

	CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

	return s32ret;
}

CVI_S32 platform_sys_exit(CVI_VOID)
{
	CVI_S32 s32ret = CVI_SUCCESS;
	bool expect = true;

	CVI_TRACE_SYS(CVI_DBG_INFO, "+\n");

	// Only exit once.
	if (!atomic_compare_exchange_strong(&sys_inited, &expect, false))
		return CVI_SUCCESS;

	if (ionFd > 0) {
		close(ionFd);
		ionFd = -1;
	}

	_sys_unmmap();
	s32ret = _sys_devmem_close();
	if (s32ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "devmem close failed\n");
		return CVI_ERR_SYS_NOTREADY;
	}
	sys_dev_close();
	base_dev_close();
	gdc_free_all_tsk_mesh();

	CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

	return s32ret;
}

CVI_S32 platform_sys_bind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	return _sys_bind_ioctl(pstSrcChn, pstDestChn, 1);
}

CVI_S32 platform_sys_unbind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	return _sys_bind_ioctl(pstSrcChn, pstDestChn, 0);
}

CVI_S32 platform_sys_getbindbydest(const MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn)
{
	CVI_S32 fd = 0;
	CVI_S32 ret = 0;
	struct sys_bind_cfg bind_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
	bind_cfg.get_by_src = 0;
	bind_cfg.mmf_chn_dst = *pstDestChn;

	ret = ioctl(fd, BASE_GET_BINDCFG, &bind_cfg);

	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_GetBindbyDest() failed\n");
		return ret;
	}

	memcpy(pstSrcChn, &bind_cfg.mmf_chn_src, sizeof(MMF_CHN_S));
	return CVI_SUCCESS;

}

CVI_S32 platform_sys_getbindbysrc(const MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
	CVI_S32 fd = 0;
	CVI_S32 ret = 0;
	struct sys_bind_cfg bind_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
	bind_cfg.get_by_src = 1;
	bind_cfg.mmf_chn_src = *pstSrcChn;

	ret = ioctl(fd, BASE_GET_BINDCFG, &bind_cfg);

	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_GetBindbySrc() failed\n");
		return ret;
	}

	memcpy(pstBindDest, &bind_cfg.bind_dst, sizeof(MMF_BIND_DEST_S));
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getversion(MMF_VERSION_S *pstVersion)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVersion);

	snprintf(pstVersion->version, VERSION_NAME_MAXLEN, "%s-%s", MMF_VERSION, SDK_VER);
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getchipid(CVI_U32 *pu32ChipId)
{
	static CVI_U32 id = 0xffffffff;
	int fd;

	if (id == 0xffffffff) {
		CVI_U32 tmp = 0;

		fd = get_sys_fd();
		if (fd == -1) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "Can't open device, cvi-sys.\n");
			return CVI_ERR_SYS_NOTREADY;
		}

		if (ioctl(fd, SYS_IOC_READ_CHIP_ID, &tmp) < 0) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl SYS_IOC_READ_CHIP_ID failed\n");
			return CVI_FAILURE;
		}

		id = tmp;
	}

	*pu32ChipId = id;
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getpoweronreason(CVI_U32 *pu32PowerOnReason)
{
	int fd;
	CVI_U32 ret_val = 0x0;
	CVI_U32 reason = 0x0;

	fd = get_sys_fd();
	if (fd == -1) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Can't open device, cvi-sys.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	if (ioctl(fd, SYS_IOC_READ_CHIP_PWR_ON_REASON, &reason) < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_IOC_READ_CHIP_PWR_ON_REASON failed\n");
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

CVI_S32 platform_sys_getchipversion(CVI_U32 *pu32ChipVersion)
{
	static CVI_U32 version = 0xffffffff;
	int fd;

	if (version == 0xffffffff) {
		CVI_U32 tmp = 0;

		fd = get_sys_fd();
		if (fd == -1) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "Can't open device, cvi-sys.\n");
			return CVI_ERR_SYS_NOTREADY;
		}

		if (ioctl(fd, SYS_IOC_READ_CHIP_VERSION, &tmp) < 0) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl SYS_IOC_READ_CHIP_VERSION failed\n");
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

void *platform_sys_mmap(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	_sys_devmem_open();

	return devm_map(devm_fd, u64PhyAddr, u32Size);
}

/* CVI_SYS_MmapCache - mmap the physical address to cached virtual-address
 *
 * @param pu64PhyAddr: the phy-address of the buffer
 * @param u32Size: the length of the buffer
 * @return virtual-address if success; 0 if fail.
 */
void *platform_sys_mmapcache(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	_sys_devmem_open();

	void *addr = devm_map(devm_cached_fd, u64PhyAddr, u32Size);

	if (addr)
		platform_sys_ioninvalidatecache(u64PhyAddr, addr, u32Size);
	return addr;
}

CVI_S32 platform_sys_munmap(void *pVirAddr, CVI_U32 u32Size)
{
	devm_unmap(pVirAddr, u32Size);
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_ionalloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr, const CVI_CHAR *strName, CVI_U32 u32Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);

	return _sys_ion_alloc_cache(pu64PhyAddr, ppVirAddr, u32Len, CVI_FALSE, strName);
}

/* CVI_SYS_IonAlloc_Cached - acquire buffer of u32Len from ion
 *
 * @param pu64PhyAddr: the phy-address of the buffer
 * @param ppVirAddr: the cached vir-address of the buffer
 * @param strName: the name of the buffer
 * @param u32Len: the length of the buffer acquire
 * @return CVI_SUCCES if ok
 */
CVI_S32 platform_sys_ionalloc_cached(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
				 const CVI_CHAR *strName, CVI_U32 u32Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);

	return _sys_ion_alloc_cache(pu64PhyAddr, ppVirAddr, u32Len, CVI_TRUE, strName);
}

CVI_S32 platform_sys_ionfree(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr)
{
	struct sys_ion_data ion_data;
	int ret;

	ion_data.addr_p = u64PhyAddr;
	ret = _sys_ion_free(&ion_data);
	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ionFree failed\n");
		return ret;
	}
	if (pVirAddr)
		devm_unmap(pVirAddr, ion_data.size);

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_ionflushcache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_S32 fd = -1;
	CVI_S32 ret = CVI_SUCCESS;
	struct sys_cache_op cache_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	if (pVirAddr == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "pVirAddr Null.\n");
		return CVI_ERR_SYS_NULL_PTR;
	}

	cache_cfg.addr_p = u64PhyAddr;
	cache_cfg.addr_v = pVirAddr;
	cache_cfg.size = u32Len;

	ret = ioctl(fd, BASE_CACHE_FLUSH, &cache_cfg);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ion flush err.\n");
		ret = CVI_ERR_SYS_NOTREADY;
	}
	return ret;
}

CVI_S32 platform_sys_ioninvalidatecache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_S32 fd = -1;
	CVI_S32 ret = CVI_SUCCESS;
	struct sys_cache_op cache_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	if (pVirAddr == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "pVirAddr Null.\n");
		return CVI_ERR_SYS_NULL_PTR;
	}

	cache_cfg.addr_p = u64PhyAddr;
	cache_cfg.addr_v = pVirAddr;
	cache_cfg.size = u32Len;

	ret = ioctl(fd, BASE_CACHE_INVLD, &cache_cfg);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ion invalid err.\n");
		ret = CVI_ERR_SYS_NOTREADY;
	}
	return ret;
}

CVI_S32 platform_sys_setvivpssmode(const VI_VPSS_MODE_S *pstVIVPSSMode)
{
	CVI_S32 fd = 0;
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

	if ((fd = get_sys_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	return ioctl(fd, SYS_IOC_SET_VIVPSSMODE, pstVIVPSSMode);
}

CVI_S32 platform_sys_getvivpssmode(VI_VPSS_MODE_S *pstVIVPSSMode)
{
	CVI_S32 fd = 0;
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

	if ((fd = get_sys_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	return ioctl(fd, SYS_IOC_GET_VIVPSSMODE, pstVIVPSSMode);
}

CVI_S32 platform_sys_setlevelconf(LOG_LEVEL_CONF_S *pstConf)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

	if (pstConf->enModId >= CVI_ID_BUTT) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Invalid ModId(%d)\n", pstConf->enModId);
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}

	if (_sys_mmap() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "_sys_mmap failed.\n");
		return CVI_FAILURE;
	}

	log_levels[pstConf->enModId] = pstConf->s32Level;
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getlevelconf(LOG_LEVEL_CONF_S *pstConf)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

	if (pstConf->enModId >= CVI_ID_BUTT) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Invalid ModId(%d)\n", pstConf->enModId);
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}

	if (_sys_mmap() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "_sys_mmap failed.\n");
		return CVI_FAILURE;
	}

	pstConf->s32Level = log_levels[pstConf->enModId];
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_gettimestamp(CVI_U64 *pu64CurPTS)
{
    CVI_S32 fd = -1;
    CVI_S32 ret;

    if ((fd = get_base_fd()) == -1)
        return CVI_ERR_SYS_NOTREADY;

    ret = ioctl(fd, BASE_GET_TIMESTAMP, pu64CurPTS);
    if (ret) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl BASE_GET_TIMESTAMP failed\n");
        return ret;
    }

    return CVI_SUCCESS;
}