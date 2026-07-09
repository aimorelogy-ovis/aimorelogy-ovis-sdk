#include <cvi_type.h>
#include <cvi_comm_vo.h>
#include "cvi_errno.h"
#include "sys_internal.h"
#include "vo_bin.h"
#include "cvi_bin.h"
#include "cvi_vo.h"
#include "stdlib.h"

#undef MOD_CHECK_NULL_PTR
#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)

/**************************************************************************
 *   Bin related APIs.
 **************************************************************************/
#define VO_BIN_GUARDMAGIC 0x12345678
static VO_BIN_INFO_S vo_bin_info[VO_MAX_DEV_NUM] = {
	{
		.gamma_info = {
			.s32VoDev = 0,
			.enable = CVI_FALSE,
			.osd_apply = CVI_FALSE,
			.value = {
				0,   3,   7,   11,  15,  19,  23,  27,
				31,  35,  39,  43,  47,  51,  55,  59,
				63,  67,  71,  75,  79,  83,  87,  91,
				95,  99,  103, 107, 111, 115, 119, 123,
				127, 131, 135, 139, 143, 147, 151, 155,
				159, 163, 167, 171, 175, 179, 183, 187,
				191, 195, 199, 203, 207, 211, 215, 219,
				223, 227, 231, 235, 239, 243, 247, 251,
				255
			}
		},
		.guard_magic = VO_BIN_GUARDMAGIC
	},
};

static CVI_U32 get_vo_bin_guardmagic_code(void)
{
	return VO_BIN_GUARDMAGIC;
}

CVI_S32 vo_bin_getbinsize(enum CVI_BIN_SECTION_ID id, CVI_U32 *size)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, size);
	*size = sizeof(VO_BIN_INFO_S);
	UNUSED(id);
	return CVI_SUCCESS;
}

CVI_S32 vo_bin_getparamfrombin(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 size)
{
	CVI_U32 data_size;
	VO_BIN_INFO_S info_from_bin;
	CVI_S32 ret = CVI_SUCCESS;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, addr);

	vo_bin_getbinsize(id, &data_size);
	if (size > data_size) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Bin size(%d) > max size(%d).\n", size, data_size);
		return CVI_FAILURE;
	}
	memcpy(&info_from_bin, addr, size);

	//check guard pattern
	if (info_from_bin.guard_magic != get_vo_bin_guardmagic_code()) {
		CVI_TRACE_VO(CVI_DBG_ERR, "readback guardpattern incorrect guard_magic(0x%x)\n",
			     info_from_bin.guard_magic);
	} else {
		CVI_TRACE_VO(CVI_DBG_DEBUG, "get param from bin success\n");
		ret = CVI_VO_SetGammaInfo(&(info_from_bin.gamma_info));

		if (ret == CVI_SUCCESS) {
			memcpy(&vo_bin_info[info_from_bin.gamma_info.s32VoDev], &info_from_bin, sizeof(VO_BIN_INFO_S));

		}
	}

	return ret;
}

CVI_S32 vo_bin_setparamtobuf(enum CVI_BIN_SECTION_ID id, CVI_U8 *buffer)
{
	CVI_U32 data_size = 0;
	VO_BIN_INFO_S *pstVoBinInfo = vo_bin_info;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, buffer);

	vo_bin_getbinsize(id, &data_size);
	memcpy(buffer, &pstVoBinInfo[id - CVI_BIN_ID_VO], data_size);

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Index bin related APIs.
 **************************************************************************/
CVI_S32 vo_getIndexBinParam(enum CVI_BIN_SECTION_ID id, CVI_U8 *buf,
					VO_Parameter_Structures *pst, CVI_U32 indexOffset)
{
	CVI_S32 ret = CVI_SUCCESS;

	if ((id < CVI_BIN_ID_MIN) || (id >= CVI_BIN_ID_MAX)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Vo id %d value error\n", id);
		return CVI_FAILURE;
	}

	if (buf == NULL || pst == NULL) {
		return CVI_FAILURE;
	}

	if (indexOffset <= 0) {
		return CVI_FAILURE;
	}

	ret = vo_getBinParam_autogen(id, buf, pst, indexOffset);

	return ret;
}

CVI_S32 vo_index_bin_getparamfrombin(CVI_U8 *buffer, enum CVI_BIN_SECTION_ID id,
										CVI_U32 binSize, CVI_U32 indexOffset)
{
	CVI_U32 data_size;
	CVI_S32 ret = CVI_SUCCESS;

	if ((id < CVI_BIN_ID_MIN) || (id >= CVI_BIN_ID_MAX)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Vo id %d value error\n", id);
		return CVI_FAILURE;
	}

	if (buffer == NULL || indexOffset <= 0) {
		return CVI_FAILURE;
	}

	vo_bin_getbinsize(id, &data_size);
	if (binSize > data_size) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Bin size(%d) > max size(%d).\n", binSize, data_size);
		return CVI_FAILURE;
	}

	VO_Parameter_Structures *param = (VO_Parameter_Structures *)malloc(sizeof(VO_Parameter_Structures));

	vo_getIndexBinParam(id, buffer, param, indexOffset);

	//check guard pattern
	if (param->vo_bin_data.guard_magic != get_vo_bin_guardmagic_code()) {
		CVI_TRACE_VO(CVI_DBG_ERR, "readback guardpattern incorrect guard_magic(0x%x)\n",
			     param->vo_bin_data.guard_magic);
	} else {
		CVI_TRACE_VO(CVI_DBG_DEBUG, "get param from bin success\n");
		memcpy(&vo_bin_info[param->vo_bin_data.gamma_info.s32VoDev],
				&param->vo_bin_data, sizeof(VO_BIN_INFO_S));
		ret = CVI_VO_SetGammaInfo(&(param->vo_bin_data.gamma_info));
	}
	free(param);

	return ret;
}
