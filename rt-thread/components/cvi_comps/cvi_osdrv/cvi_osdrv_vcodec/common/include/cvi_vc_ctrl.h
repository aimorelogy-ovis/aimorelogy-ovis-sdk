#ifndef __CVI_VC_CTRL_H__
#define __CVI_VC_CTRL_H__

#ifndef NULL
#define NULL 0
#endif

typedef enum
{
    REG_CTRL = 0,
    REG_SBM,
    REG_REMAP,
} REG_TYPE;

unsigned int cvi_vc_drv_read_vc_reg(REG_TYPE eRegType, unsigned long addr);
void         cvi_vc_drv_write_vc_reg(REG_TYPE eRegType, unsigned long addr,
                                     unsigned int data);

#define CtrlWriteReg(CORE, ADDR, DATA) \
    cvi_vc_drv_write_vc_reg(REG_CTRL, ADDR, DATA)
#define CtrlReadReg(CORE, ADDR) cvi_vc_drv_read_vc_reg(REG_CTRL, ADDR)
#define RemapWriteReg(CORE, ADDR, DATA) \
    cvi_vc_drv_write_vc_reg(REG_REMAP, ADDR, DATA)
#define RemapReadReg(CORE, ADDR) cvi_vc_drv_read_vc_reg(REG_REMAP, ADDR)

#endif /* __CVI_VC_CTRL_H__ */
