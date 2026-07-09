#ifndef __VI_CORE_H__
#define __VI_CORE_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include <vi_common.h>
#include <vi_defines.h>
#include <vi_interfaces.h>

extern int request_irq(unsigned int irqn, void *handler, unsigned long flags, const char *name, void *priv);

#ifdef __cplusplus
}
#endif

#endif /* __VI_CORE_H__ */
