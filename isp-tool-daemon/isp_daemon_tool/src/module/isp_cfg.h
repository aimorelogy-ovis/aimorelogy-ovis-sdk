
#ifndef __ISP_CFG_H__
#define __ISP_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

int init_isp(uint8_t pipe_id, void *video_src_cfg);
int deinit_isp(uint8_t pipe_id, void *video_src_cfg);

#ifdef __cplusplus
}
#endif
#endif // __ISP_CFG_H__
