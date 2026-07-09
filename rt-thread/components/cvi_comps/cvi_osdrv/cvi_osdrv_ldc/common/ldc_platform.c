#include <base_cb.h>
#include <ldc_cb.h>
#include <vip_common.h>
#include "ldc_debug.h"
#include "ldc_platform.h"
#include "cvi_vip_ldc.h"
#include "cvi_vip_gdc_proc.h"
#include "gdc.h"
#include "ldc.h"
#include "mesh.h"
#include "cvi_reg.h"

// Add type definitions
typedef unsigned char u8;
typedef unsigned int  u32;

// Function declarations
static int ldc_cmd_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg);

#define LDC_DEV_NAME "cvi-ldc"

#define GDC_SHARE_MEM_SIZE (0x8000)

//u32 ldc_log_lv = CVI_DBG_WARN/*CVI_DBG_INFO*/;
struct cvi_ldc_vdev *g_wdev;

int ldc_open(void)
{
    return 0;
}
int ldc_release(void)
{
    return 0;
}

long ldc_ioctl(unsigned int cmd, void *arg)
{
    struct cvi_ldc_vdev *wdev = g_wdev;
    int                  ret  = 0;

    switch (cmd)
    {
    case CVI_LDC_BEGIN_JOB: {
        struct gdc_handle_data *data = (struct gdc_handle_data *)arg;

        CVI_TRACE_LDC(CVI_DBG_DEBUG, "CVILDC_BEGIN_JOB\n");
        ret = gdc_begin_job(wdev, data);
        break;
    }

    case CVI_LDC_END_JOB: {
        struct gdc_handle_data *data = (struct gdc_handle_data *)arg;

        CVI_TRACE_LDC(CVI_DBG_DEBUG, "CVILDC_END_JOB, handle=0x%llx\n",
                      (unsigned long long)data->handle);
        ret = gdc_end_job(wdev, data->handle);
        break;
    }

    case CVI_LDC_ADD_ROT_TASK: {
        struct gdc_task_attr *attr = (struct gdc_task_attr *)arg;

        CVI_TRACE_LDC(CVI_DBG_DEBUG, "CVILDC_ADD_ROT_TASK, handle=0x%llx\n",
                      (unsigned long long)attr->handle);
        ret = gdc_add_rotation_task(wdev, attr);
        break;
    }

    case CVI_LDC_ADD_LDC_TASK: {
        struct gdc_task_attr *attr = (struct gdc_task_attr *)arg;

        CVI_TRACE_LDC(CVI_DBG_DEBUG, "CVILDC_ADD_LDC_TASK, handle=0x%llx\n",
                      (unsigned long long)attr->handle);
        ret = gdc_add_ldc_task(wdev, attr);
        break;
    }

    case CVI_LDC_SET_BUF_WRAP: {
        struct ldc_buf_wrap_cfg *cfg = (struct ldc_buf_wrap_cfg *)arg;

        CVI_TRACE_LDC(CVI_DBG_DEBUG, "CVI_LDC_SET_BUF_WRAP, handle=0x%llx\n",
                      (unsigned long long)cfg->handle);

        ret = gdc_set_buf_wrap(wdev, cfg);
        break;
    }

    case CVI_LDC_GET_BUF_WRAP: {
        struct ldc_buf_wrap_cfg *cfg = (struct ldc_buf_wrap_cfg *)arg;

        CVI_TRACE_LDC(CVI_DBG_DEBUG, "CVI_LDC_GET_BUF_WRAP, handle=0x%llx\n",
                      (unsigned long long)cfg->handle);

        ret = gdc_get_buf_wrap(wdev, cfg);
        break;
    }

    default: {
        CVI_TRACE_LDC(CVI_DBG_ERR, "bad ioctl cmd((%d))\n", cmd);
        ret = -1;
        break;
    }
    }

    return ret;
}

// Static function declarations
static int      _init_resources(struct cvi_ldc_vdev *wdev);
static MOD_ID_E convert_mod_id(enum ENUM_MODULES_ID cbModId);
static int      ldc_rm_cb(void);
static int      ldc_register_cb(struct cvi_ldc_vdev *wdev);

int ldc_create_instance(struct cvi_ldc_vdev *wdev)
{
    int rc = 0;

    // clk_ldc_src_sel default 1(clk_src_vip_sys_2), 600 MHz
    // set 0(clk_src_vip_sys_4), 400MHz for ND
    vip_sys_reg_write_mask(VIP_SYS_VIP_CLK_CTRL1, BIT(20), BIT(20));
    wdev->align = LDC_ADDR_ALIGN;
    strcpy(wdev->dev_name, LDC_DEV_NAME);

    wdev->shared_mem = rt_malloc(GDC_SHARE_MEM_SIZE);
    memset(wdev->shared_mem, 0, GDC_SHARE_MEM_SIZE);
    if (!wdev->shared_mem)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "gdc shared mem alloc fail\n");
        return -1;
    }
    if (gdc_proc_init(wdev->shared_mem) < 0)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "gdc proc init failed\n");
        return -1;
    }

    if (cvi_gdc_init(wdev))
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "gdc init fail\n");
        goto err_work_init;
    }

    return 0;

err_work_init:
    return rc;
}

int ldc_destroy_instance(struct cvi_ldc_vdev *wdev)
{
    gdc_proc_remove();
    cvi_gdc_deinit(wdev);
    if (wdev->shared_mem)
        rt_free(wdev->shared_mem);

    return 0;
}

void ldc_irq_handler(u8 intr_status, struct cvi_ldc_vdev *wdev)
{
    struct cvi_ldc_job *job = NULL;
    struct gdc_task    *tsk;

    if (!dlist_empty(&wdev->jobq))
        job = dlist_first_entry(&wdev->jobq, struct cvi_ldc_job, node);

    if (job)
    {
        if (!dlist_empty(&job->task_list))
        {
            tsk        = dlist_first_entry(&job->task_list, struct gdc_task, node);
            tsk->state = GDC_TASK_STATE_DONE;
        }
        else
            CVI_TRACE_LDC(CVI_DBG_DEBUG, "%s: error, empty tsk\n", __func__);

        gdc_proc_record_hw_end(job);

        /* wakeup worker */
        rt_sem_release(wdev->hw_done_sem);
        CVI_TRACE_LDC(CVI_DBG_DEBUG, "wakeup worker\n");
    }
    else
        CVI_TRACE_LDC(CVI_DBG_DEBUG, "%s: error, empty job\n", __func__);
}

void ldc_isr(int irq, void *_dev)
{
    u8 intr_status = ldc_intr_status();

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "status(0x%x)\n", intr_status);
    ldc_intr_clr(intr_status);
    ldc_irq_handler(intr_status, _dev);
}

// Move static function implementations here
static int _init_resources(struct cvi_ldc_vdev *wdev)
{
    int                       rc       = 0;
    int                       irq_num  = VIP_INT_LDC_WRAP;
    static const char * const irq_name = "ldc";
    void                     *reg_base = REG_LDC_BASE_ADDR;

    ldc_set_base_addr(reg_base);

    wdev->irq_num_ldc = irq_num;
    strcpy(wdev->irq_name, irq_name);

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "irq(%d) for %s get from vip ldc driver.\n", irq_num, irq_name);
    return rc;
}

static MOD_ID_E convert_mod_id(enum ENUM_MODULES_ID cbModId)
{
    if (cbModId == E_MODULE_VI)
        return CVI_ID_VI;
    else if (cbModId == E_MODULE_VO)
        return CVI_ID_VO;
    else if (cbModId == E_MODULE_VPSS)
        return CVI_ID_VPSS;

    return CVI_ID_BUTT;
}

static int ldc_rm_cb(void)
{
    return base_rm_module_cb(E_MODULE_LDC);
}

static int ldc_register_cb(struct cvi_ldc_vdev *wdev)
{
    struct base_m_cb_info reg_cb;

    reg_cb.module_id = E_MODULE_LDC;
    reg_cb.dev       = (void *)wdev;
    reg_cb.cb        = ldc_cmd_cb;

    return base_reg_module_cb(&reg_cb);
}

static int ldc_cmd_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg)
{
    struct cvi_ldc_vdev *wdev = (struct cvi_ldc_vdev *)dev;
    int                  rc   = -1;

    switch (cmd)
    {
    case LDC_CB_MESH_GDC_OP: {
        struct mesh_gdc_cfg *cfg =
            (struct mesh_gdc_cfg *)arg;
        MOD_ID_E enModId = convert_mod_id(caller);

        if (enModId == CVI_ID_BUTT)
            return CVI_FAILURE;

        rc = mesh_gdc_do_op(wdev, cfg->usage, cfg->pUsageParam,
                            cfg->vb_in, cfg->enPixFormat, cfg->mesh_addr,
                            cfg->sync_io, cfg->pcbParam,
                            cfg->cbParamSize, enModId, cfg->enRotation);
    }
    break;

    case LDC_CB_VPSS_SBM_DONE: {
        rc = ldc_vpss_sdm_cb_done(wdev);
        break;
    }

    default:
        break;
    }

    return rc;
}

// Add attribute to mark functions as intentionally unused
__attribute__((unused)) int cvi_ldc_probe(void)
{
    int                  rc = 0;
    struct cvi_ldc_vdev *wdev;

    wdev = rt_malloc(sizeof(*wdev));
    if (!wdev)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "Can not get cvi_ldc_vdev\n");
        return -1;
    }

    // get hw-resources
    rc = _init_resources(wdev);
    if (rc)
        goto err_irq;

    // for ldc
    rc = ldc_create_instance(wdev);
    if (rc)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "Failed to create ldc instance\n");
        goto err_ldc_reg;
    }
    ldc_intr_ctrl(0x01);
    rt_hw_interrupt_mask(wdev->irq_num_ldc);
    rt_hw_interrupt_install(wdev->irq_num_ldc, ldc_isr, (void *)wdev, "CVI_VIP_LDC");
    rt_hw_interrupt_umask(wdev->irq_num_ldc);
    // if (request_irq(wdev->irq_num_ldc, ldc_isr, 0, "CVI_VIP_LDC", (void *)wdev)) {
    // 	CVI_TRACE_LDC(CVI_DBG_ERR, "Unable to request ldc IRQ(%d)\n", wdev->irq_num_ldc);
    // 	return -1;
    // }

    /* ldc register cb */
    if (ldc_register_cb(wdev))
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "Failed to register ldc cb, err %d\n", rc);
        return rc;
    }
    g_wdev = wdev;
    CVI_TRACE_LDC(CVI_DBG_INFO, "done with rc(%d).\n", rc);

    return rc;

err_ldc_reg:
err_irq:
    CVI_TRACE_LDC(CVI_DBG_ERR, "failed with rc(%d).\n", rc);
    return rc;
}

/*
 * cvi_ldc_remove - device remove method.
 * @pdev: Pointer of platform device.
 */
__attribute__((unused)) int cvi_ldc_remove(void)
{
    struct cvi_ldc_vdev *wdev = g_wdev;

    if (!wdev)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "Can not get cvi_ldc_vdev\n");
        return -1;
    }
    ldc_destroy_instance(wdev);

    /* ldc rm cb */
    if (ldc_rm_cb())
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "Failed to rm ldc cb\n");
        return -1;
    }
    rt_free(wdev);

    return 0;
}
