#include <vi_core.h>
#include <vi_uapi.h>

#define CVI_VI_IRQ_NAME   "isp"
#define CVI_VI_CLASS_NAME "cvi-vi"
#define CVI_VI_DEV_NAME   "cvi-vi"

/* Runtime to enable vi write reg info
 * Ctrl:
 *	0: Disable to dump addr/val info of writing reg.
 *	1: Enable to dump addr/val info of writing reg.
 */
int vi_dump_reg;
// module_param(vi_dump_reg, int, 0644);

// static DEFINE_RAW_SPINLOCK(__io_lock);
static struct cvi_vi_dev *g_dev;

const char *const clk_sys_name[] = {"clk_sys_0", "clk_sys_1", "clk_sys_2", "clk_sys_3"};
const char *const clk_isp_name[] = {"clk_axi", "clk_csi_be", "clk_raw", "clk_isp_top"};
const char *const clk_mac_name[] = {"clk_csi_mac0", "clk_csi_mac1", "clk_csi_mac2"};

#if 0
void _reg_write_mask(uintptr_t addr, u32 mask, u32 data)
{
	unsigned long flags;
	u32 value;

	raw_spin_lock_irqsave(&__io_lock, flags);
	value = readl_relaxed((void __iomem *)addr) & ~mask;
	value |= (data & mask);
	writel(value, (void __iomem *)addr);
	raw_spin_unlock_irqrestore(&__io_lock, flags);
}
#endif
// TODO vfs
#if 0
static long vi_core_ioctl(struct file *filp, u_int cmd, u_long arg)
{
	return vi_ioctl(filp, cmd, arg);
}

#ifdef CONFIG_COMPAT
static long compat_ptr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	if (!file->f_op->unlocked_ioctl)
		return -ENOIOCTLCMD;

	return file->f_op->unlocked_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
}
#endif

static int vi_core_open(struct inode *inode, struct file *filp)
{
	return vi_open(inode, filp);
}

static int vi_core_release(struct inode *inode, struct file *filp)
{
	return vi_release(inode, filp);
}

static int vi_core_mmap(struct file *filp, struct vm_area_struct *vm)
{
	return vi_mmap(filp, vm);
}

static unsigned int vi_core_poll(struct file *filp, struct poll_table_struct *wait)
{
	return vi_poll(filp, wait);
}

const struct file_operations vi_fops = {
	.owner = THIS_MODULE,
	.open = vi_core_open,
	.unlocked_ioctl = vi_core_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_ptr_ioctl,
#endif
	.release = vi_core_release,
	.mmap = vi_core_mmap,
	.poll = vi_core_poll,
};
#endif

int vi_core_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg)
{
    return vi_cb(dev, caller, cmd, arg);
}

static int vi_core_rm_cb(void)
{
    return base_rm_module_cb(E_MODULE_VI);
}

static int vi_core_register_cb(struct cvi_vi_dev *dev)
{
    struct base_m_cb_info reg_cb;

    reg_cb.module_id = E_MODULE_VI;
    reg_cb.dev       = (void *)dev;
    reg_cb.cb        = vi_core_cb;

    return base_reg_module_cb(&reg_cb);
}

static void vi_core_isr(int irq, void *priv)
{
    struct cvi_vi_dev *vdev = (struct cvi_vi_dev *)priv;

    vi_irq_handler(vdev);
}

// TODO  maybe need clk ctrl,  use vfs
#if 0
static int vi_core_register_cdev(struct cvi_vi_dev *dev)
{
	struct device *dev_t;
	int err = 0;

	dev->vi_class = class_create(THIS_MODULE, CVI_VI_CLASS_NAME);
	if (IS_ERR(dev->vi_class)) {
		dev_err(dev->dev, "create class failed\n");
		return PTR_ERR(dev->vi_class);
	}

	/* get the major number of the character device */
	if ((alloc_chrdev_region(&dev->cdev_id, 0, 1, CVI_VI_DEV_NAME)) < 0) {
		err = -EBUSY;
		dev_err(dev->dev, "allocate chrdev failed\n");
		return err;
	}

	/* initialize the device structure and register the device with the kernel */
	dev->cdev.owner = THIS_MODULE;
	cdev_init(&dev->cdev, &vi_fops);

	if ((cdev_add(&dev->cdev, dev->cdev_id, 1)) < 0) {
		err = -EBUSY;
		dev_err(dev->dev, "add chrdev failed\n");
		return err;
	}

	dev_t = device_create(dev->vi_class, dev->dev, dev->cdev_id, NULL, "%s", CVI_VI_DEV_NAME);
	if (IS_ERR(dev_t)) {
		dev_err(dev->dev, "device create failed error code(%ld)\n", PTR_ERR(dev_t));
		err = PTR_ERR(dev_t);
		return err;
	}

	return err;
}

static int vi_core_clk_init(struct platform_device *pdev)
{
	struct cvi_vi_dev *dev;
	u8 i = 0;

	dev = dev_get_drvdata(&pdev->dev);
	if (!dev) {
		dev_err(&pdev->dev, "Can not get cvi_vi drvdata\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(clk_sys_name); ++i) {
		dev->clk_sys[i] = devm_clk_get(&pdev->dev, clk_sys_name[i]);
		if (IS_ERR(dev->clk_sys[i])) {
			dev_err(&pdev->dev, "Cannot get clk for %s\n", clk_sys_name[i]);
			return PTR_ERR(dev->clk_sys[i]);
		}
	}

	for (i = 0; i < ARRAY_SIZE(clk_isp_name); ++i) {
		dev->clk_isp[i] = devm_clk_get(&pdev->dev, clk_isp_name[i]);
		if (IS_ERR(dev->clk_isp[i])) {
			dev_err(&pdev->dev, "Cannot get clk for %s\n", clk_isp_name[i]);
			return PTR_ERR(dev->clk_isp[i]);
		}
	}

	for (i = 0; i < ARRAY_SIZE(clk_mac_name); ++i) {
		dev->clk_mac[i] = devm_clk_get(&pdev->dev, clk_mac_name[i]);
		if (IS_ERR(dev->clk_mac[i])) {
			dev_err(&pdev->dev, "Cannot get clk for %s\n", clk_mac_name[i]);
			return PTR_ERR(dev->clk_mac[i]);
		}
	}

	return 0;
}
#endif

int vi_core_init(void)
{
    int ret = 0;

    // g_dev = calloc(1,(sizeof(struct cvi_vi_dev)));
    g_dev = rt_calloc(1, (sizeof(struct cvi_vi_dev)));
    if (!g_dev) {
        vi_pr(VI_ERR, "fail to malloc\n");
        return -ENOMEM;
    }

    /* reg_base*/
    g_dev->reg_base = (void *)ISP_BASE_ADDR;

    /* Interrupt */
    g_dev->irq_num = ISP_TOP_INT;

// TODO maybe need clk ctrl, so need init clk ctrl
#if 0
	ret = vi_core_clk_init(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to init clk, err %d\n", ret);
		goto err_clk_init;
	}
#endif

// TODO use vfs, if yes, this function will be use?
#if 0
	ret = vi_core_register_cdev(dev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register dev, err %d\n", ret);
		goto err_dev_register;
	}
#endif
    ret = vi_create_instance(g_dev);
    if (ret) {
        vi_pr(VI_ERR, "Failed to create instance, err %d\n", ret);
        goto err_create_instance;
    }

    // request_irq(g_dev->irq_num, vi_core_isr, 0, "vi_isr", g_dev);
    rt_hw_interrupt_install(g_dev->irq_num, vi_core_isr, (void *)g_dev, "vi_isr");
    rt_hw_interrupt_umask(g_dev->irq_num);

    ret = vi_core_register_cb(g_dev);
    if (ret) {
        vi_pr(VI_ERR, "Failed to register vi cb, err %d\n", ret);
        goto err_create_instance;
    }

    vi_pr(VI_INFO, "isp registered as %s\n", CVI_VI_DEV_NAME);

err_create_instance:
    return ret;
}

int vi_core_deinit(void)
{
    int ret = 0;

    struct cvi_vi_dev *dev = g_dev;

    ret = vi_destroy_instance(dev);
    if (ret) {
        printf("Failed to destroy instance, err %d\n", ret);
        goto err_destroy_instance;
    }

    ret = vi_core_rm_cb();
    if (ret) {
        printf("Failed to rm vi cb, err %d\n", ret);
    }

err_destroy_instance:
    vi_pr(VI_INFO, "%s -\n", __func__);

    return ret;
}
