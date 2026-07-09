#include <stdatomic.h>

#include "cvi_comm_vb.h"
#include "cvi_debug.h"
#include "msg_vb.h"
#include "cvi_msg_client.h"


#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif

#define VB_CHECK_NULL_PTR(ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_VB(CVI_DBG_ERR, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)


static atomic_bool vb_inited = ATOMIC_VAR_INIT(false);

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 platform_vb_init(void)
{
	CVI_S32 s32Ret;
	bool expect = false;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	// Only init once until exit.
	if (!atomic_compare_exchange_strong(&vb_inited, &expect, true))
		return CVI_SUCCESS;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_INIT, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "vb init fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_vb_exit(void)
{
	CVI_S32 s32Ret;
	bool expect = true;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	// Only exit once.
	if (!atomic_compare_exchange_strong(&vb_inited, &expect, false) && !CVI_MSG_IsInited())
		return CVI_SUCCESS;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_EXIT, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "vb exit fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

/* platform_vb_getblock: acquice a vb_blk with specific size from pool.
 *
 * @param pool: the pool to acquice blk. if VB_INVALID_POOLID, go through common-pool to search.
 * @param u32BlkSize: the size of vb_blk to acquire.
 * @return: the vb_blk if available. otherwise, VB_INVALID_HANDLE.
 */
VB_BLK platform_vb_getblock(VB_POOL Pool, CVI_U32 u32BlkSize)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	VB_BLK blk = VB_INVALID_HANDLE;

	stPrivData.as32PrivData[0] = Pool;
	stPrivData.as32PrivData[1] = u32BlkSize;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_GET_BLOCK, &blk, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "get block fail,s32Ret:%x\n", s32Ret);
		return blk;
	}
	return blk;
}

/* platform_vb_releaseblock: release a vb_blk.
 *
 * @param Block: the vb_blk going to be released.
 * @return: CVI_SUCCESS if success; others if fail.
 */
CVI_S32 platform_vb_releaseblock(VB_BLK Block)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_RELEASE_BLOCK, &Block, sizeof(Block), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Release block fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

VB_BLK platform_vb_physaddr2handle(CVI_U64 u64PhyAddr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	VB_BLK blk = VB_INVALID_HANDLE;

	s32Ret = CVI_MSG_SendSync2(u32ModFd, MSG_CMD_VB_PHYS_ADDR2_HANDLE, &u64PhyAddr,
				sizeof(u64PhyAddr), &blk, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "PhysAddr2Handle fail,s32Ret:%x\n", s32Ret);
		return VB_INVALID_HANDLE;
	}
	return blk;
}

CVI_U64 platform_vb_handle2physaddr(VB_BLK Block)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	CVI_U64 u64PhyAddr = 0;

	s32Ret = CVI_MSG_SendSync2(u32ModFd, MSG_CMD_VB_HANDLE2_PHYS_ADDR, &Block, sizeof(Block),
				&u64PhyAddr, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Handle2PhysAddr fail,s32Ret:%x\n", s32Ret);
		return u64PhyAddr;
	}
	return u64PhyAddr;
}

VB_POOL platform_vb_handle2poolid(VB_BLK Block)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	VB_POOL pool = VB_INVALID_POOLID;

	pool = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_HANDLE2_POOL_ID, &Block, sizeof(Block), NULL);
	return pool;
}

CVI_S32 platform_vb_inquireusercnt(VB_BLK Block, CVI_U32 *pCnt)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	VB_CHECK_NULL_PTR(pCnt);

	s32Ret = CVI_MSG_SendSync3(u32ModFd, MSG_CMD_VB_INQUIRE_USER_CNT, &Block, sizeof(Block),
				pCnt);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "InquireUserCnt fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

VB_POOL platform_vb_createpool(VB_POOL_CONFIG_S *pstVbPoolCfg)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	VB_POOL pool = VB_INVALID_POOLID;

	VB_CHECK_NULL_PTR(pstVbPoolCfg);

	pool = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_CREATE_POOL, pstVbPoolCfg,
				sizeof(VB_POOL_CONFIG_S), NULL);
	return pool;
}

VB_POOL platform_vb_createexpool(VB_POOL_CONFIG_EX_S *pstVbPoolExCfg)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	VB_POOL pool = VB_INVALID_POOLID;

	VB_CHECK_NULL_PTR(pstVbPoolExCfg);

	pool = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_CREATE_EX_POOL, pstVbPoolExCfg,
				sizeof(VB_POOL_CONFIG_EX_S), NULL);
	return pool;
}

CVI_S32 platform_vb_destroypool(VB_POOL Pool)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	MSG_PRIV_DATA_S stPrivData;

	stPrivData.as32PrivData[0] = Pool;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_DESTROY_POOL, NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "DestroyPool fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_vb_setconfig(const VB_CONFIG_S *pstVbConfig)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	VB_CHECK_NULL_PTR(pstVbConfig);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_SET_CONFIG, (CVI_VOID *)pstVbConfig,
		sizeof(VB_CONFIG_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Set config fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_vb_getconfig(VB_CONFIG_S *pstVbConfig)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);

	VB_CHECK_NULL_PTR(pstVbConfig);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_GET_CONFIG, pstVbConfig, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Get config fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

/* CVI_VB_MmapPool - mmap the whole pool to get virtual-address
 *
 * @param Pool: pool id
 * @return CVI_SUCCESS if success; others if fail
 */
CVI_S32 platform_vb_mmappool(VB_POOL Pool)
{
	UNUSED(Pool);
	CVI_TRACE_VB(CVI_DBG_WARN, "mmap pool(%d) not supported yet.\n", Pool);
	return CVI_SUCCESS;
}

CVI_S32 platform_vb_munmappoold(VB_POOL Pool)
{
	UNUSED(Pool);
	CVI_TRACE_VB(CVI_DBG_WARN, "munmap pool(%d) not supported yet.\n", Pool);
	return CVI_SUCCESS;
}

/* CVI_VB_GetBlockVirAddr - to get virtual-address of the Block
 *
 * @param Pool: pool id
 * @param Block: block id
 * @param ppVirAddr: virtual-address of the Block, cached if pool create with VB_REMAP_MODE_CACHED
 * @return CVI_SUCCESS if success; others if fail
 */
CVI_S32 platform_vb_getblockviraddr(VB_POOL Pool, VB_BLK Block, void **ppVirAddr)
{
	UNUSED(Pool);
	UNUSED(Block);
	UNUSED(ppVirAddr);
	CVI_TRACE_VB(CVI_DBG_WARN, "GetBlockVirAddr not supported yet.\n");

	return CVI_SUCCESS;
}

CVI_VOID platform_vb_printpool(VB_POOL Pool)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VB, 0, 0);
	MSG_PRIV_DATA_S stPrivData;

	stPrivData.as32PrivData[0] = Pool;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VB_PRINT_POOL, NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "PrintPool fail,s32Ret:%x\n", s32Ret);
	}
}

