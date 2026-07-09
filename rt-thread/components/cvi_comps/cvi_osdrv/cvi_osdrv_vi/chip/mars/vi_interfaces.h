#ifndef __VI_INTERFACES_H__
#define __VI_INTERFACES_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include <base_cb.h>


/*******************************************************
 *  File operations for core
 ******************************************************/
// long vi_ioctl(struct file *filp, u_int cmd, u_long arg);
// int vi_open(struct inode *inode, struct file *filp);
// int vi_release(struct inode *inode, struct file *filp);
// int vi_mmap(struct file *filp, struct vm_area_struct *vm);
// unsigned int vi_poll(struct file *filp, struct poll_table_struct *wait);

/*******************************************************
 *  Common interface for core
 ******************************************************/
int vi_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg);
void vi_irq_handler(struct cvi_vi_dev *vdev);
int vi_create_instance(struct cvi_vi_dev *pdev);
int vi_destroy_instance(struct cvi_vi_dev *pdev);

#ifdef __cplusplus
}
#endif

#endif /* __VI_INTERFACES_H__ */
