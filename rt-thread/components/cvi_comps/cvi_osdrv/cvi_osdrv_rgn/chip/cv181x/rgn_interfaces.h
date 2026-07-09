#ifndef __RGN_INTERFACES_H__
#define __RGN_INTERFACES_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include <base_cb.h>

/*******************************************************
 *  Common interface for core
 ******************************************************/
int rgn_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg);
int rgn_create_instance(struct cvi_rgn_dev *pdev);
int rgn_destroy_instance(struct cvi_rgn_dev *pdev);

#ifdef __cplusplus
}
#endif

#endif /* __RGN_INTERFACES_H__ */
