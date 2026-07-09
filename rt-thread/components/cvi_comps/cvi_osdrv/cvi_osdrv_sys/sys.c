#include "sys.h"

#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "base_cb.h"
#include "cvi_base.h"
#include "cvi_comm_sys.h"
#include "cvi_debug.h"
#include "cvi_defines.h"
#include "cvi_reg.h"
#include "sys_context.h"
#include "sys_uapi.h"
#include "vi_cb.h"
#include "vip_common.h"
#include "vpss_cb.h"

/* register bank */
#define TOP_BASE            0x03000000
#define TOP_REG_BANK_SIZE   0x10000
#define GP_REG3_OFFSET      0x8C
#define GP_REG_CHIP_ID_MASK 0xFFFF

#define CV182X_RTC_BASE   0x05026000
#define RTC_REG_BANK_SIZE 0x140
#define RTC_ST_ON_REASON  0xF8

extern void vip_set_base_addr(void *base);

/* sensor cmm extern function. */
enum vip_sys_cmm {
    VIP_CMM_I2C = 0,
    VIP_CMM_SSP,
    VIP_CMM_BUTT,
};

struct vip_sys_cmm_ops {
    long (*cb)(void *hdlr, unsigned int cmd, void *arg);
};

struct vip_sys_cmm_dev {
    enum vip_sys_cmm       cmm_type;
    void                  *hdlr;
    struct vip_sys_cmm_ops ops;
};

static struct vip_sys_cmm_dev cmm_ssp;
static struct vip_sys_cmm_dev cmm_i2c;
static struct base_m_cb_info *sys_m_cb;

static unsigned int _sys_read_by_kernel(unsigned int chip_id)
{
#ifdef __riscv
    switch (chip_id) {
    case 0x1810C:
        return E_CHIPID_CV1810C;
    case 0x1811C:
        return E_CHIPID_CV1811C;
    case 0x1812C:
        return E_CHIPID_CV1812C;
    case 0X1810F:
        return E_CHIPID_CV1810H;
    case 0x1811F:
        return E_CHIPID_CV1811H;
    case 0x1812F:
        return E_CHIPID_CV1812H;
    case 0x1813F:
        return E_CHIPID_CV1813H;
    }
#else
    switch (chip_id) {
    case 0x1810C:
        return E_CHIPID_CV1820A;
    case 0x1811C:
        return E_CHIPID_CV1821A;
    case 0x1812C:
        return E_CHIPID_CV1822A;
    case 0x1811F:
        return E_CHIPID_CV1823A;
    case 0x1812F:
        return E_CHIPID_CV1825A;
    case 0x1813F:
        return E_CHIPID_CV1826A;
    }
#endif

    // mars default CV1810C
    return E_CHIPID_CV1810C;
}

unsigned int sys_read_chip_id(void)
{
    unsigned int chip_id = _reg_read(TOP_BASE + GP_REG3_OFFSET);

    switch (chip_id) {
    case 0x1821:
        return E_CHIPID_CV1821;
    case 0x1822:
        return E_CHIPID_CV1822;
    case 0x1826:
        return E_CHIPID_CV1826;
    case 0x1832:
        return E_CHIPID_CV1832;
    case 0x1838:
        return E_CHIPID_CV1838;
    case 0x1829:
        return E_CHIPID_CV1829;
    case 0x1820:
        return E_CHIPID_CV1820;
    case 0x1823:
        return E_CHIPID_CV1823;
    case 0x1825:
        return E_CHIPID_CV1825;
    case 0x1835:
        return E_CHIPID_CV1835;

    case 0x1810C:
    case 0x1811C:
    case 0x1812C:
    case 0X1810F:
    case 0x1811F:
    case 0x1812F:
    case 0x1813F:
        return _sys_read_by_kernel(chip_id);

    // cv180x
    case 0x1800B:
        return E_CHIPID_CV1800B;
    case 0x1801B:
        return E_CHIPID_CV1801B;
    case 0x1800C:
        return E_CHIPID_CV1800C;
    case 0x1801C:
        return E_CHIPID_CV1801C;

    // default cv1835
    default:
        return E_CHIPID_CV1835;
    }
}

unsigned int sys_read_chip_version(void)
{
    unsigned int chip_version = 0;

    chip_version = _reg_read(TOP_BASE) & 0xFFF;

    return (chip_version + 1);
}

unsigned int sys_read_chip_pwr_on_reason(void)
{
    unsigned int reason = 0;

    reason = _reg_read(CV182X_RTC_BASE + RTC_ST_ON_REASON);

    switch (reason) {
    case 0x800d0000:
    case 0x800f0000:
        return E_CHIP_PWR_ON_COLDBOOT;
    case 0x880d0003:
    case 0x880f0003:
        return E_CHIP_PWR_ON_WDT;
    case 0x80050009:
    case 0x800f0009:
        return E_CHIP_PWR_ON_SUSPEND;
    case 0x840d0003:
    case 0x840f0003:
        return E_CHIP_PWR_ON_WARM_RST;
    default:
        return E_CHIP_PWR_ON_COLDBOOT;
    }
}

CVI_U32 sys_get_chipid(void)
{
    struct sys_info *p_info = (struct sys_info *)sys_ctx_get_sysinfo();
    return p_info->chip_id;
}

CVI_U8 *sys_get_version(void)
{
    struct sys_info *p_info = (struct sys_info *)sys_ctx_get_sysinfo();
    return (CVI_U8 *)p_info->version;
}

int vip_sys_register_cmm_cb(unsigned long cmm, void *hdlr, void *cb)
{
    struct vip_sys_cmm_dev *cmm_dev;

    if ((cmm >= VIP_CMM_BUTT) || !hdlr || !cb) return -1;

    cmm_dev = (cmm == VIP_CMM_I2C) ? &cmm_i2c : &cmm_ssp;

    cmm_dev->cmm_type = cmm;
    cmm_dev->hdlr     = hdlr;
    cmm_dev->ops.cb   = cb;

    return 0;
}

int vip_sys_cmm_cb_i2c(unsigned int cmd, void *arg)
{
    struct vip_sys_cmm_dev *cmm_dev = &cmm_i2c;

    if (cmm_dev->cmm_type != VIP_CMM_I2C) return -1;

    return (cmm_dev->ops.cb) ? cmm_dev->ops.cb(cmm_dev->hdlr, cmd, arg) : (-1);
}

int vip_sys_cmm_cb_ssp(unsigned int cmd, void *arg)
{
    struct vip_sys_cmm_dev *cmm_dev = &cmm_ssp;

    if (cmm_dev->cmm_type != VIP_CMM_SSP) return -1;

    return (cmm_dev->ops.cb) ? cmm_dev->ops.cb(cmm_dev->hdlr, cmd, arg) : (-1);
}

int _sys_exe_module_cb(struct base_exe_m_cb *exe_cb)
{
    struct base_m_cb_info *cb_info;

    if (exe_cb->caller < 0 || exe_cb->caller >= E_MODULE_BUTT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "sys exe cb error: wrong caller\n");
        return -1;
    }

    if (exe_cb->callee < 0 || exe_cb->callee >= E_MODULE_BUTT) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "sys exe cb error: wrong callee\n");
        return -1;
    }

    if (!sys_m_cb) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "sys_m_cb/base_m_cb not ready yet\n");
        return -1;
    }

    cb_info = &sys_m_cb[exe_cb->callee];
    if (!cb_info->cb) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "sys exe cb error\n");
        return -1;
    }

    return cb_info->cb(cb_info->dev, exe_cb->caller, exe_cb->cmd_id, exe_cb->data);
}

static int _sys_call_cb(u32 m_id, u32 cmd_id, void *data)
{
    struct base_exe_m_cb exe_cb;

    exe_cb.callee = m_id;
    exe_cb.caller = E_MODULE_SYS;
    exe_cb.cmd_id = cmd_id;
    exe_cb.data   = (void *)data;

    return _sys_exe_module_cb(&exe_cb);
}

int _sys_s_ctrl(struct sys_ext_control *p)
{
    u32                  id      = p->id;
    int                  rc      = -1;
    struct sys_ctx_info *sys_ctx = NULL;

    sys_ctx = sys_get_ctx();

    switch (id) {
    case SYS_IOCTL_SET_VIVPSSMODE: {
        VI_VPSS_MODE_S *stVIVPSSMode;

        stVIVPSSMode = &sys_ctx->mode_cfg.vivpss_mode;
        memcpy(stVIVPSSMode, p->ptr, sizeof(VI_VPSS_MODE_S));

        if (_sys_call_cb(E_MODULE_VI, VI_CB_SET_VIVPSSMODE, stVIVPSSMode) != 0) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "VI_CB_SET_VIVPSSMODE failed\n");
            break;
        }

        if (_sys_call_cb(E_MODULE_VPSS, VPSS_CB_SET_VIVPSSMODE, stVIVPSSMode) != 0) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "VPSS_CB_SET_VIVPSSMODE failed\n");
            break;
        }

        rc = 0;
        break;
    }
    case SYS_IOCTL_SET_VPSSMODE: {
        VPSS_MODE_E enVPSSMode;

        sys_ctx->mode_cfg.vpss_mode.enMode = enVPSSMode = (VPSS_MODE_E)p->value;

        if (_sys_call_cb(E_MODULE_VPSS, VPSS_CB_SET_VPSSMODE, (void *)&enVPSSMode) != 0) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "VPSS_CB_SET_VPSSMODE failed\n");
            break;
        }

        rc = 0;
        break;
    }
    case SYS_IOCTL_SET_VPSSMODE_EX: {
        VPSS_MODE_S *stVPSSMode;

        stVPSSMode = &sys_ctx->mode_cfg.vpss_mode;
        memcpy(stVPSSMode, p->ptr, sizeof(VPSS_MODE_S));

        if (_sys_call_cb(E_MODULE_VPSS, VPSS_CB_SET_VPSSMODE_EX, (void *)stVPSSMode) != 0) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "VPSS_CB_SET_VPSSMODE_EX failed\n");
            break;
        }

        rc = 0;
        break;
    }
    case SYS_IOCTL_SET_SYS_INIT: {
        sys_ctx->sys_inited = 1;

        rc = 0;
        break;
    }
    default:
        break;
    }

    return rc;
}

int _sys_g_ctrl(struct sys_ext_control *p)
{
    u32                  id      = p->id;
    int                  rc      = -1;
    struct sys_ctx_info *sys_ctx = NULL;

    sys_ctx = sys_get_ctx();

    switch (id) {
    case SYS_IOCTL_GET_VIVPSSMODE: {
        memcpy(p->ptr, &sys_ctx->mode_cfg.vivpss_mode, sizeof(VI_VPSS_MODE_S));
        rc = 0;
        break;
    }
    case SYS_IOCTL_GET_VPSSMODE: {
        p->value = sys_ctx->mode_cfg.vpss_mode.enMode;

        rc = 0;
        break;
    }
    case SYS_IOCTL_GET_VPSSMODE_EX: {
        memcpy(p->ptr, &sys_ctx->mode_cfg.vpss_mode, sizeof(VPSS_MODE_S));
        rc = 0;
        break;
    }
    case SYS_IOCTL_GET_SYS_INIT: {
        p->value = sys_ctx->sys_inited;

        rc = 0;
        break;
    }
    default:
        break;
    }

    return rc;
}

static int _cvi_sys_sg_ctrl(unsigned int cmd, void *arg)
{
    int                    ret = 0;
    struct sys_ext_control p;

    memcpy(&p, (void *)arg, sizeof(struct sys_ext_control));

    switch (cmd) {
    case SYS_IOC_S_CTRL:
        ret = _sys_s_ctrl(&p);
        break;
    case SYS_IOC_G_CTRL:
        ret = _sys_g_ctrl(&p);
        break;
    default:
        ret = -1;
        break;
    }

    memcpy((void *)arg, &p, sizeof(struct sys_ext_control));

    return ret;
}

static CVI_S32 sys_set_bind_cfg(struct sys_bind_cfg *ioctl_arg)
{
    CVI_S32 ret = 0;

    if (ioctl_arg->is_bind)
        ret = base_ctx_bind(&ioctl_arg->mmf_chn_src, &ioctl_arg->mmf_chn_dst);
    else
        ret = base_ctx_unbind(&ioctl_arg->mmf_chn_src, &ioctl_arg->mmf_chn_dst);

    return ret;
}

static CVI_S32 sys_get_bind_cfg(struct sys_bind_cfg *ioctl_arg)
{
    CVI_S32 ret = 0;

    if (ioctl_arg->get_by_src)
        ret = base_ctx_get_bindbysrc(&ioctl_arg->mmf_chn_src, &ioctl_arg->bind_dst);
    else
        ret = base_ctx_get_bindbydst(&ioctl_arg->mmf_chn_dst, &ioctl_arg->mmf_chn_src);

    if (ret) CVI_TRACE_SYS(CVI_DBG_ERR, "sys_ctx_getbind failed\n");

    return ret;
}

static sys_ion_alloc_user(struct sys_ion_data *alloc)
{
    CVI_U64 phy_addr;
    void *vaddr;

    if (base_ion_alloc(&phy_addr, &vaddr, alloc->name, alloc->size, alloc->cached) != CVI_SUCCESS) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "base_ion_alloc failed\n");
        return -1;
    }

    alloc->addr_p = phy_addr;
    alloc->dmabuf_fd = 0; // Not used in this context

    return 0;
}

static sys_ion_free_user(struct sys_ion_data *free)
{
    if (base_ion_free(free->addr_p) != CVI_SUCCESS) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "base_ion_free failed\n");
        return -1;
    }

    return 0;
}

int cvi_sys_ioctl(unsigned int cmd, void *arg)
{
    int ret = 0;

    switch (cmd) {
    case SYS_IOC_S_CTRL:
    case SYS_IOC_G_CTRL:
        ret = _cvi_sys_sg_ctrl(cmd, arg);
        break;
    case SYS_ION_ALLOC:
        ret = sys_ion_alloc_user((struct sys_ion_data *)arg);
        break;
    case SYS_ION_FREE:
        ret = sys_ion_free_user((struct sys_ion_data *)arg);
        break;
    case SYS_SET_BINDCFG:
        ret = sys_set_bind_cfg((struct sys_bind_cfg *)arg);
        break;
    case SYS_GET_BINDCFG:
        ret = sys_get_bind_cfg((struct sys_bind_cfg *)arg);
        break;
    case SYS_READ_CHIP_ID:
        *((CVI_U32 *)arg) = sys_get_chipid();
        break;
    case SYS_READ_CHIP_VERSION:
        *((CVI_U32 *)arg) = sys_read_chip_version();
        break;
    case SYS_READ_CHIP_PWR_ON_REASON:
        *((CVI_U32 *)arg) = sys_read_chip_pwr_on_reason();
        break;

    default:
        return -1;
    }
    return ret;
}

int cvi_sys_open()
{
    return 0;
}

int cvi_sys_close()
{
    struct sys_ctx_info *sys_ctx = NULL;

    sys_ctx = sys_get_ctx();
    base_ctx_release_bind();
    sys_ctx->sys_inited = 0;
    return 0;
}

int sys_core_init()
{
    struct sys_ctx_info *sys_ctx = sys_get_ctx();

    sys_ctx_init();
    base_core_init();
    base_save_modules_cb(&sys_m_cb);
    vip_set_base_addr(REG_VIP_BASE_ADDR);
    vip_sys_set_offline(VIP_SYS_AXI_BUS_SC_TOP, true);
    vip_sys_set_offline(VIP_SYS_AXI_BUS_ISP_RAW, true);
    vip_sys_set_offline(VIP_SYS_AXI_BUS_ISP_YUV, true);
    sys_ctx->sys_info.chip_id = sys_read_chip_id();
    CVI_TRACE_SYS(CVI_DBG_DEBUG, "CVITEK CHIP ID = %d\n", sys_ctx->sys_info.chip_id);

    return 0;
}

int sys_core_exit()
{
    base_core_exit();
    return 0;
}
