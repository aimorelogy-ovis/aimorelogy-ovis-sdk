
#ifndef __VENC_CFG_H__
#define __VENC_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

int module_venc_init(int chn_id, void *cfg);
int module_venc_deinit(int chn_id);

#ifdef __cplusplus
}
#endif
#endif // __VENC_CFG_H__
