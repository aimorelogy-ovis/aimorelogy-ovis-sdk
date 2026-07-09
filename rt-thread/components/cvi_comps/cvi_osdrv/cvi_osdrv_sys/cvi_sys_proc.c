#include <cvi_base_ctx.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define GENERATE_STRING(STRING) (#STRING),

static const char *const MOD_STRING[] = FOREACH_MOD(GENERATE_STRING);

extern struct _BIND_NODE_S bind_nodes[BIND_NODE_MAXNUM];

#define CHN_MATCH(x, y) (((x)->enModId == (y)->enModId) && ((x)->s32DevId == (y)->s32DevId) && ((x)->s32ChnId == (y)->s32ChnId))

/*************************************************************************
 *	sys proc functions
 *************************************************************************/
static bool _is_fisrt_level_bind_node(BIND_NODE_S *node)
{
    int i, j;
    BIND_NODE_S *bindNodes;

    bindNodes = bind_nodes;
    for (i = 0; i < BIND_NODE_MAXNUM; ++i) {
        if ((bindNodes[i].bUsed) && (bindNodes[i].dsts.u32Num != 0) && !CHN_MATCH(&bindNodes[i].src, &node->src)) {
            for (j = 0; j < bindNodes[i].dsts.u32Num; ++j) {
                if (CHN_MATCH(&bindNodes[i].dsts.astMmfChn[j], &node->src))
                    // find other source in front of this node
                    return false;
            }
        }
    }

    return true;
}

static BIND_NODE_S *_find_next_bind_node(const MMF_CHN_S *pstSrcChn)
{
    int i;
    BIND_NODE_S *bindNodes;

    bindNodes = bind_nodes;
    for (i = 0; i < BIND_NODE_MAXNUM; ++i) {
        if ((bindNodes[i].bUsed) && CHN_MATCH(pstSrcChn, &bindNodes[i].src) && (bindNodes[i].dsts.u32Num != 0)) {
            return &bindNodes[i];
        }
    }

    return NULL; // didn't find next bind node
}

static void _show_sys_status()
{
    int i, j, k;
    BIND_NODE_S *bindNodes, *nextBindNode;
    MMF_CHN_S *first, *second, *third;

    bindNodes = bind_nodes;

    printf("-----BIND RELATION TABLE-----------------------------------------------------------------------------------------------------------\n");
    printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
           "1stMod", "1stDev", "1stChn", "2ndMod", "2ndDev", "2ndChn", "3rdMod", "3rdDev", "3rdChn");

    for (i = 0; i < BIND_NODE_MAXNUM; ++i) {
        // Check if the bind node is used / has destination / first level of bind chain
        if ((bindNodes[i].bUsed) && (bindNodes[i].dsts.u32Num != 0) && (_is_fisrt_level_bind_node(&bindNodes[i]))) {

            first = &bindNodes[i].src; // bind chain first level

            for (j = 0; j < bindNodes[i].dsts.u32Num; ++j) {
                second = &bindNodes[i].dsts.astMmfChn[j]; // bind chain second level

                nextBindNode = _find_next_bind_node(second);
                if (nextBindNode != NULL) {
                    for (k = 0; k < nextBindNode->dsts.u32Num; ++k) {
                        third = &nextBindNode->dsts.astMmfChn[k]; // bind chain third level
                        printf("%-10s%-10d%-10d%-10s%-10d%-10d%-10s%-10d%-10d\n",
                               MOD_STRING[first->enModId], first->s32DevId, first->s32ChnId,
                               MOD_STRING[second->enModId], second->s32DevId, second->s32ChnId,
                               MOD_STRING[third->enModId], third->s32DevId, third->s32ChnId);
                    }
                } else { // level 3 node not found
                    printf("%-10s%-10d%-10d%-10s%-10d%-10d%-10s%-10d%-10d\n",
                           MOD_STRING[first->enModId], first->s32DevId, first->s32ChnId,
                           MOD_STRING[second->enModId], second->s32DevId, second->s32ChnId,
                           "null", 0, 0);
                }
            }
        }
    }
    printf("\n-----------------------------------------------------------------------------------------------------------------------------------\n");
}

static void proc_sys(CVI_S32 argc, char **argv)
{
    _show_sys_status();
}

MSH_CMD_EXPORT(proc_sys, sys info);
