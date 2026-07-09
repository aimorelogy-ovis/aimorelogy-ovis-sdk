
#ifndef __VIDEO_SRC_CFG_H__
#define __VIDEO_SRC_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

int module_video_src_init(uint8_t pipe_id, void *video_src_cfg);
int module_video_src_deinit(uint8_t pipe_id, void *video_src_cfg);
int module_video_src_replay_init(uint8_t pipe_id, void *video_src_cfg);

#ifdef __cplusplus
}
#endif
#endif // __VIDEO_SRC_CFG_H__
