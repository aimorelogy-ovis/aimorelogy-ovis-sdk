/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: module/vpu/include/gdc_mesh.h
 * Description:
 *   GDC's mesh generator for hw.
 */
#include <sys/queue.h>
#include "cvi_comm_gdc.h"

#ifndef __GDC_MESH_H__
#define __GDC_MESH_H__

#define GDC_MAX_TSK_MESH (32)

typedef struct _TSK_MESH_ATTR_S {
	CVI_CHAR Name[32];
	CVI_U64 paddr;
	CVI_VOID *vaddr;
} TSK_MESH_ATTR_S;

int get_mesh_size(int *p_mesh_hor, int *p_mesh_ver);
int set_mesh_size(int mesh_hor, int mesh_ver);
void mesh_gen_get_1st_size(SIZE_S in_size, CVI_U32 *mesh_1st_size);
void mesh_gen_get_2nd_size(SIZE_S in_size, CVI_U32 *mesh_2nd_size);
void mesh_gen_get_size(SIZE_S in_size, SIZE_S out_size, CVI_U32 *mesh_id_size, CVI_U32 *mesh_tbl_size);
void mesh_gen_rotation(SIZE_S in_size, SIZE_S out_size, ROTATION_E rot, uint64_t mesh_phy_addr, void *mesh_vir_addr);
CVI_S32 mesh_gen_ldc(SIZE_S in_size, SIZE_S out_size, const LDC_ATTR_S *pstLDCAttr,
		     uint64_t mesh_phy_addr, void *mesh_vir_addr, ROTATION_E rot);

void gdc_free_cur_tsk_mesh(CVI_CHAR *meshName);
void gdc_free_all_tsk_mesh(void);
int gdc_set_tsk_mesh_by_name(const char *tskName, CVI_U64 paddr, CVI_VOID *vaddr);
int gdc_get_tsk_mesh_by_name(TSK_MESH_ATTR_S* tskMeshAttr);

CVI_S32 gdc_gen_ldcmesh(CVI_U32 u32Width, CVI_U32 u32Height, const LDC_ATTR_S *pstLDCAttr,
		const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);
CVI_S32 gdc_load_ldcmesh(CVI_U32 u32Width, CVI_U32 u32Height, const char *fileNname
	, const char *tskName, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);

extern TSK_MESH_ATTR_S tskMesh[GDC_MAX_TSK_MESH];


#endif // __GDC_MESH_H__
