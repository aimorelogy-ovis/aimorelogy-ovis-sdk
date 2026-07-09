#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdatomic.h>

#include "cvi_errno.h"
#include "cvi_debug.h"
#include "devmem.h"
#include "hashmap.h"
#include "platform_sys.h"
#include "msg_sys.h"
#include "cvi_msg_client.h"
#include "base_uapi.h"
#include "cvi_msg_client.h"
#include "gdc_mesh.h"


#define BASE_DEV_NAME "/dev/soph-base"


#define SYS_CHECK_NULL_PTR(ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_SYS(CVI_DBG_ERR, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(CVI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)


static atomic_bool sys_inited = ATOMIC_VAR_INIT(false);
static int devm_fd = -1, devm_cached_fd = -1;
static CVI_S32 base_fd = -1;
static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

CVI_S32 *log_levels;
CVI_CHAR const *log_name[8] = {
	(CVI_CHAR *)"EMG", (CVI_CHAR *)"ALT", (CVI_CHAR *)"CRI", (CVI_CHAR *)"ERR",
	(CVI_CHAR *)"WRN", (CVI_CHAR *)"NOT", (CVI_CHAR *)"INF", (CVI_CHAR *)"DBG"
};

static int _open_device(const char *dev_name, CVI_S32 *fd)
{
	struct stat st;

	*fd = open(dev_name, O_RDWR /* required */  | O_NONBLOCK | O_CLOEXEC, 0);
	if (-1 == *fd) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return -1;
	}

	if (-1 == fstat(*fd, &st)) {
		close(*fd);
		CVI_TRACE_SYS(CVI_DBG_ERR, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		close(*fd);
		CVI_TRACE_SYS(CVI_DBG_ERR, "%s is no device\n", dev_name);
		return -ENODEV;
	}
	return 0;
}

static CVI_S32 _close_device(CVI_S32 *fd)
{
	if (*fd == -1)
		return -1;

	if (-1 == close(*fd)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "%s: fd(%d) failure\n", __func__, *fd);
		return -1;
	}

	*fd = -1;

	return CVI_SUCCESS;
}

static CVI_S32 _base_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&fd_lock);
	_close_device(&base_fd);
	pthread_mutex_unlock(&fd_lock);

	return CVI_SUCCESS;
}

static CVI_S32 _get_base_fd(CVI_VOID)
{
	pthread_mutex_lock(&fd_lock);
	if (base_fd <= 0) {
		if (_open_device(BASE_DEV_NAME, &base_fd) == -1) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "base open fail\n");
			base_fd = -1;
		}
	}
	pthread_mutex_unlock(&fd_lock);

	return base_fd;
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

static CVI_S32 _sys_ion_malloc(struct sys_ion_data *para)
{
	CVI_S32 fd = -1;
	CVI_S32 ret;

	if ((fd = _get_base_fd()) == -1)
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

	if ((fd = _get_base_fd()) == -1)
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
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	bool expect = false;

	CVI_TRACE_SYS(CVI_DBG_INFO, "+\n");

	// Only init once until exit.
	if (!atomic_compare_exchange_strong(&sys_inited, &expect, true))
		return CVI_SUCCESS;

	if (_sys_devmem_open() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "devmem open failed.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	s32Ret = CVI_MSG_Init();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "msg init fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_ALIOS_INIT, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "alios sys init fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

	return s32Ret;
}

CVI_S32 platform_sys_exit(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	bool expect = true;

	CVI_TRACE_SYS(CVI_DBG_INFO, "+\n");

	// Only exit once.
	if (!atomic_compare_exchange_strong(&sys_inited, &expect, false) && !CVI_MSG_IsInited())
		return CVI_SUCCESS;

	s32Ret = _sys_devmem_close();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "devmem close failed\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_ALIOS_EXIT, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "alios sys exit fail,s32Ret:%x\n", s32Ret);
	}

	s32Ret = CVI_MSG_Deinit();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "msg deinit fail, s32Ret:%x\n", s32Ret);
	}
	_base_dev_close();
	gdc_free_all_tsk_mesh();

	CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

	return s32Ret;
}

CVI_S32 platform_sys_bind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	MMF_CHN_S stChn[2];

	SYS_CHECK_NULL_PTR(pstSrcChn);
	SYS_CHECK_NULL_PTR(pstDestChn);
	stChn[0] = *pstSrcChn;
	stChn[1] = *pstDestChn;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_BIND, (CVI_VOID *)stChn,
				sizeof(stChn), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Sys bind fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_unbind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	MMF_CHN_S stChn[2];

	SYS_CHECK_NULL_PTR(pstSrcChn);
	SYS_CHECK_NULL_PTR(pstDestChn);
	stChn[0] = *pstSrcChn;
	stChn[1] = *pstDestChn;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_UNBIND, (CVI_VOID *)stChn,
				sizeof(stChn), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Sys unbind fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getbindbydest(const MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	SYS_CHECK_NULL_PTR(pstSrcChn);
	SYS_CHECK_NULL_PTR(pstDestChn);

	s32Ret = CVI_MSG_SendSync2(u32ModFd, MSG_CMD_SYS_GET_BIND_BY_DEST, (CVI_VOID *)pstDestChn,
				sizeof(MMF_CHN_S), (CVI_VOID *)pstSrcChn, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetBindbyDest fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getbindbysrc(const MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);
	MSG_PRIV_DATA_S stPrivData;

	SYS_CHECK_NULL_PTR(pstSrcChn);
	SYS_CHECK_NULL_PTR(pstBindDest);

	s32Ret = CVI_MSG_SendSync2(u32ModFd, MSG_CMD_SYS_GET_BIND_BY_SRC, (CVI_VOID *)pstSrcChn,
				sizeof(MMF_CHN_S), (CVI_VOID *)pstBindDest->astMmfChn, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetBindbySrc fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	pstBindDest->u32Num = stPrivData.as32PrivData[0];
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getversion(MMF_VERSION_S *pstVersion)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	SYS_CHECK_NULL_PTR(pstVersion);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_VERSION, pstVersion, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetVersion fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getchipid(CVI_U32 *pu32ChipId)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	SYS_CHECK_NULL_PTR(pu32ChipId);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_CHIP_ID, pu32ChipId, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetChipId fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getpoweronreason(CVI_U32 *pu32PowerOnReason)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	SYS_CHECK_NULL_PTR(pu32PowerOnReason);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_POWER_REASON, pu32PowerOnReason, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetPowerOnReason fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getchipversion(CVI_U32 *pu32ChipVersion)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	SYS_CHECK_NULL_PTR(pu32ChipVersion);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_CHIP_VERSION, pu32ChipVersion, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetChipVersion fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
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
	SYS_CHECK_NULL_PTR(pu64PhyAddr);

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
	SYS_CHECK_NULL_PTR(pu64PhyAddr);

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

	if ((fd = _get_base_fd()) == -1)
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

	if ((fd = _get_base_fd()) == -1)
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
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	SYS_CHECK_NULL_PTR(pstVIVPSSMode);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_SET_VI_VPSS_MODE,
				(CVI_VOID *)pstVIVPSSMode, sizeof(VI_VPSS_MODE_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Set vi-vpss mode fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getvivpssmode(VI_VPSS_MODE_S *pstVIVPSSMode)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	SYS_CHECK_NULL_PTR(pstVIVPSSMode);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SYS_GET_VI_VPSS_MODE,
				pstVIVPSSMode, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Get vi-vpss mode fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_setlevelconf(LOG_LEVEL_CONF_S *pstConf)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	SYS_CHECK_NULL_PTR(pstConf);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_LOG_SET_LEVEL_CONF,
					(CVI_VOID *)pstConf, sizeof(LOG_LEVEL_CONF_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "SetLevelConf fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getlevelconf(LOG_LEVEL_CONF_S *pstConf)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SYS, 0, 0);

	SYS_CHECK_NULL_PTR(pstConf);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_LOG_GET_LEVEL_CONF,
					(CVI_VOID *)pstConf, sizeof(LOG_LEVEL_CONF_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "GetLevelConf fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_gettimestamp(CVI_U64 *pu64CurPTS)
{
    CVI_S32 fd = -1;
    CVI_S32 ret;

	if ((fd = _get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

    ret = ioctl(fd, BASE_GET_TIMESTAMP, pu64CurPTS);
    if (ret) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl BASE_GET_TIMESTAMP failed\n");
        return ret;
    }

    return CVI_SUCCESS;
}