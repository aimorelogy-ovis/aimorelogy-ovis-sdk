#include "cvi_vc_ctrl.h"

#define VC_REG_CTRL_BASE  0x0B030000
#define VC_REG_SBM_BASE   0x0B058000
#define VC_REG_REMAP_BASE 0x0B050000

unsigned int cvi_vc_drv_read_vc_reg(REG_TYPE eRegType, unsigned long addr)
{
    unsigned int *reg_addr = NULL;
    // TODO: enable CCF?

    switch (eRegType)
    {
    case REG_CTRL:
        reg_addr = (unsigned int *)(addr + VC_REG_CTRL_BASE);
        break;
    case REG_SBM:
        reg_addr = (unsigned int *)(addr + VC_REG_SBM_BASE);
        break;
    case REG_REMAP:
        reg_addr = (unsigned int *)(addr + VC_REG_REMAP_BASE);
        break;
    default:
        break;
    }

    if (!reg_addr) return 0;

    // printf("vc read, 0x%x, 0x%x\n", addr, *reg_addr);

    return *(volatile unsigned int *)reg_addr;
}

void cvi_vc_drv_write_vc_reg(REG_TYPE eRegType, unsigned long addr,
                             unsigned int data)
{
    unsigned int *reg_addr = NULL;
    // TODO: enable CCF?

    switch (eRegType)
    {
    case REG_CTRL:
        reg_addr = (unsigned int *)(addr + VC_REG_CTRL_BASE);
        break;
    case REG_SBM:
        reg_addr = (unsigned int *)(addr + VC_REG_SBM_BASE);
        break;
    case REG_REMAP:
        reg_addr = (unsigned int *)(addr + VC_REG_REMAP_BASE);
        break;
    default:
        break;
    }

    if (!reg_addr) return;

    // printf("vc write, 0x%x = 0x%x\n", addr, data);

    *(volatile unsigned int *)reg_addr = data;
}
