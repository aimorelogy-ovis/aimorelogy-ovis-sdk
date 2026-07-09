#ifndef __BASE_H__
#define __BASE_H__

#include <base_cb.h>
#include <cvi_base.h>
#include <cvi_comm_sys.h>
#include <cvi_common.h>
#include <cvi_type.h>
#include <stdbool.h>

const char *base_get_modname(MOD_ID_E id);

int  base_rm_module_cb(enum ENUM_MODULES_ID module_id);
int  base_reg_module_cb(struct base_m_cb_info *cb_info);
int  base_exe_module_cb(struct base_exe_m_cb *exe_cb);
void base_save_modules_cb(struct base_m_cb_info **sys_m_cb);
void base_ctx_release_bind(void);

CVI_S32 base_ctx_bind(MMF_CHN_S *pstSrcChn, MMF_CHN_S *pstDestChn);
CVI_S32 base_ctx_unbind(MMF_CHN_S *pstSrcChn, MMF_CHN_S *pstDestChn);
CVI_S32 base_ctx_get_bindbysrc(MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest);
CVI_S32 base_ctx_get_bindbydst(MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn);
CVI_S32 base_bind(MMF_CHN_S *pstSrcChn, MMF_CHN_S *pstDestChn);
CVI_S32 base_unbind(MMF_CHN_S *pstSrcChn, MMF_CHN_S *pstDestChn);
CVI_S32 base_ion_alloc(CVI_U64 *p_paddr, void **pp_vaddr, char *buf_name, CVI_U32 buf_len,
                       bool is_cached);
CVI_S32 base_ion_free(CVI_U64 u64PhyAddr);
CVI_S32 base_get_bindbysrc(MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest);
CVI_S32 base_get_bindbydst(MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn);

int  base_core_init(void);
void base_core_exit(void);

#endif /* __BASE_H__ */
