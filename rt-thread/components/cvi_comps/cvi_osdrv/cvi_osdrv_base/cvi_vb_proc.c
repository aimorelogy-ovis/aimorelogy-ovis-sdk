#include "vb.h"
#include <cvi_base_ctx.h>
#include <cvi_common.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*************************************************************************
 *	VB proc functions
 *************************************************************************/
static CVI_S32 _get_vb_modIds(struct vb_pool *pool, CVI_U32 blk_idx, CVI_U64 *modIds)
{
    CVI_U64 phyAddr;
    VB_BLK blk;

    phyAddr = pool->memBaseAlign + (blk_idx * pool->blk_size);
    blk = vb_physAddr2Handle(phyAddr);
    if (blk == VB_INVALID_HANDLE)
        return -1;

    *modIds = ((struct vb_s *)blk)->mod_ids;
    return 0;
}

static void _show_vb_status(void)
{
    CVI_U32 i, j, k;
    CVI_U32 mod_sum[CVI_ID_BUTT];
    CVI_S32 ret;
    CVI_U64 modIds;
    struct vb_pool *pstVbPool = NULL;
    char *buf = malloc(8192);
    int pos = 0;
    if (!buf) {
        printf("pos is %d \n", pos);
        return;
    }
    printf("-----VB PUB CONFIG-----------------------------------------------------------------------------------------------------------------\n");
    printf("%10s(%3d), %10s(%3d)\n", "MaxPoolCnt", vb_max_pools, "MaxBlkCnt", vb_pool_max_blk);
    ret = vb_get_pool_info(&pstVbPool);
    if (ret != 0) {
        printf("vb_pool has not inited yet\n");
        return;
    }

    printf("\n-----COMMON POOL CONFIG------------------------------------------------------------------------------------------------------------\n");
    for (i = 0; i < vb_max_pools; ++i) {
        if (pstVbPool[i].memBase != 0) {
            printf("%10s(%3d)\t%10s(%12d)\t%10s(%3d)\n", "PoolId", i, "Size", pstVbPool[i].blk_size, "Count", pstVbPool[i].blk_cnt);
        }
    }

    printf("\n-----------------------------------------------------------------------------------------------------------------------------------\n");
    for (i = 0; i < vb_max_pools; ++i) {
        if (pstVbPool[i].memBase != 0) {
            pthread_mutex_lock(&pstVbPool[i].lock);
            pos += sprintf(buf + pos, "%-10s: %s\n", "PoolName", pstVbPool[i].acPoolName);
            pos += sprintf(buf + pos, "%-10s: %d\n", "PoolId", pstVbPool[i].poolID);
            pos += sprintf(buf + pos, "%-10s: 0x%lx\n", "PhysAddr", pstVbPool[i].memBaseAlign);
            pos += sprintf(buf + pos, "%-10s: 0x%lx\n", "VirtAddr", (uintptr_t)pstVbPool[i].vmemBase);
            pos += sprintf(buf + pos, "%-10s: %d\n", "IsComm", pstVbPool[i].bIsCommPool);
            pos += sprintf(buf + pos, "%-10s: %d\n", "Owner", pstVbPool[i].ownerID);
            pos += sprintf(buf + pos, "%-10s: %d\n", "BlkSz", pstVbPool[i].blk_size);
            pos += sprintf(buf + pos, "%-10s: %d\n", "BlkCnt", pstVbPool[i].blk_cnt);
            pos += sprintf(buf + pos, "%-10s: %d\n", "Free", pstVbPool[i].u32FreeBlkCnt);
            pos += sprintf(buf + pos, "%-10s: %d\n", "MinFree", pstVbPool[i].u32MinFreeBlkCnt);
            pos += sprintf(buf + pos, "\n");

            memset(mod_sum, 0, sizeof(mod_sum));
            pos += sprintf(buf + pos, "BLK\tBASE\tVB\tSYS\tRGN\tCHNL\tVDEC\tVPSS\tVENC\tH264E\tJPEGE\tMPEG4E\tH265E\tJPEGD\tVO\tVI\tDIS\n");
            pos += sprintf(buf + pos, "RC\tAIO\tAI\tAO\tAENC\tADEC\tAUD\tVPU\tISP\tIVE\tUSER\tPROC\tLOG\tH264D\tGDC\tPHOTO\tFB\n");
            for (j = 0; j < pstVbPool[i].blk_cnt; ++j) {
                pos += sprintf(buf + pos, "%s%d\t", "#", j);
                if (_get_vb_modIds(&pstVbPool[i], j, &modIds) != 0) {
                    for (k = 0; k < CVI_ID_BUTT; ++k) {
                        pos += sprintf(buf + pos, "e\t");
                        if (k == CVI_ID_DIS || k == CVI_ID_FB)
                            pos += sprintf(buf + pos, "\n");
                    }
                    continue;
                }

                for (k = 0; k < CVI_ID_BUTT; ++k) {
                    if (modIds & BIT(k)) {
                        pos += sprintf(buf + pos, "1\t");
                        mod_sum[k]++;
                    } else
                        pos += sprintf(buf + pos, "0\t");

                    if (k == CVI_ID_DIS || k == CVI_ID_FB)
                        pos += sprintf(buf + pos, "\n");
                }
            }

            pos += sprintf(buf + pos, "Sum\t");
            for (k = 0; k < CVI_ID_BUTT; ++k) {
                pos += sprintf(buf + pos, "%d\t", mod_sum[k]);
                if (k == CVI_ID_DIS || k == CVI_ID_FB)
                    pos += sprintf(buf + pos, "\n");
            }
            pthread_mutex_unlock(&pstVbPool[i].lock);
            pos += sprintf(buf + pos, "\n-----------------------------------------------------------------------------------------------------------------------------------\n");
        }
    }
    printf("%s", buf);
    free(buf);
}

static void proc_vb(CVI_S32 argc, char **argv)
{
    _show_vb_status();
}

MSH_CMD_EXPORT(proc_vb, vb info);