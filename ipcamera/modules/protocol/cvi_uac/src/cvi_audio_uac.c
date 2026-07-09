#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include "asoundlib.h"
#include "cvi_audio_uac.h"
#include "cyclebuffer.h"
#include "app_ipcam_comm.h"
// #include "cvi_audio_vqe.h"
#include "cvi_audio.h"
#include "app_ipcam_gpio.h"

#define __VERSION_TAG__   "CVI_UAC_20210412_dual_RefineCode"
#define CV18XX_AS_HOST 0//1 ic as usb host, 0 ic as usb device(slave)

#define USE_CVI_API	1
#define SPEAKER_GPIO        CVI_GPIOE_02
#ifdef THIS_IS_32
#define AUDIO_PERIOD_SIZE 320
#define CAP_PERIOD_COUNT 4
#else
#define AUDIO_PERIOD_SIZE 960
#define CAP_PERIOD_COUNT 10
#endif

#define SAMPLE_RATE 48000
#define PLAY_PERIOD_COUNT 4
#define CHANNEL_COUNT 2
#define SAVE_FILE_AUDIO_FROM_MIC 0
#define SAVE_FILE_AUDIO_TO_SPK 0
#define BYTES_PER_SAMPLE 2
#define DOWNLINK_USE_CYCLE_BUFFER 1
#define UPLINK_USE_CYCLE_BUFFER 1
int bUplinkEnable;
int  bDownlinkEnable;

//define print level -------------------------------------[start]
#define CVI_UAC_MASK_ERR	(0x01)
#define CVI_UAC_MASK_INFO	(0x01)
#define CVI_UAC_MASK_WARN	(0x02)
#define CVI_UAC_MASK_DBG	(0x04)
#define CVI_UAC_MASK_TRACE	(0x08)

int  gUAC_level;
static int  cviUacGetEnv(char *env, char *fmt, void *param);
static void cvi_audio_uac_getDbgMask(void);
//define print level .................................................[end]


#if DOWNLINK_USE_CYCLE_BUFFER
#define DOWNLINK_CYCLE_BUFFER_SIZE (1024 * 30 * 4)
#define DOWNLINK_SRC_OPEN_STATE 0x41
#define DOWNLINK_SRC_DETECT_STATE 0x42
#define DOWNLINK_SRC_SEND_DATA_STATE 0x43
#define DOWNLINK_SRC_CLOSE_AND_RETRY_STATE 0x44
#define DOWNLINK_DST_OPEN_STATE 0x51
#define DOWNLINK_DST_DETECT_STATE 0x52
#define DOWNLINK_DST_SEND_DATA_STATE 0x53
#define DOWNLINK_DST_SEND_DATA_MUTE_STATE 0x54
#define DOWNLINK_DST_CLOSE_AND_RETRY_STATE 0x55
pthread_t *pcm_downlink_src_usb_thread;
pthread_t *pcm_downlink_dst_spk_thread;
static void *thread_downlink_src_audio(void *arg);
static void *thread_downlink_dst_audio(void *arg);
void *gpstDownlinkCB;
char *pDownlinkCBuffer;
char *buffer_dl_src;
char *buffer_dl_dst;
#else
pthread_t *pcm_pc_to_mic_thread;
char *buffer; //downlink
#endif

#if UPLINK_USE_CYCLE_BUFFER
#define UPLINK_CYCLE_BUFFER_SIZE (960 * 16*4)
#define UPLINK_SRC_OPEN_STATE 0x61
#define UPLINK_SRC_DETECT_STATE 0x62
#define UPLINK_SRC_SEND_DATA_STATE 0x63
#define UPLINK_SRC_FORCE_FLUSH_STATE 0x64
#define UPLINK_SRC_CLOSE_AND_RETRY_STATE 0x65
#define UPLINK_DST_OPEN_STATE 0x71
#define UPLINK_DST_DETECT_STATE 0x72
#define UPLINK_DST_SEND_DATA_STATE 0x73
#define UPLINK_DST_SEND_DATA_MUTE_STATE 0x74
#define UPLINK_DST_CLOSE_AND_RETRY_STATE 0x75
pthread_t *pcm_uplink_src_mic_thread;
pthread_t *pcm_uplink_dst_usb_thread;
static void *thread_uplink_src_audio(void *arg);
static void *thread_uplink_dst_audio(void *arg);
char *buffer_up_src;
char *buffer_up_dst;
void *gpstUplinkCB;
char *pUplinkCBuffer;
char  *pflush_up_buffer;
#else
pthread_t *pcm_mic_to_pc_thread;
char *bufferup; //uplink
char *bufferup2;
#endif

static bool s_uac_init = false;
#if 0
static struct pcm_config stPcmCapConfigUp = {
	.channels = 2,
	.rate = SAMPLE_RATE,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = CAP_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};
#endif
static struct pcm_config stPcmPlayConfigUp = {
	.channels = 2,
	.rate = SAMPLE_RATE,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = PLAY_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};

#if USE_CVI_API
static AUDIO_DEV AiDevId = 0;
static AI_CHN AiChn = 0;
static CVI_S32 AoChn = 0;
static CVI_S32 AoDev = 0;

static AIO_ATTR_S s_stAiAttr = {
	.enBitwidth = AUDIO_BIT_WIDTH_16,
	.enI2sType = AIO_I2STYPE_INNERCODEC,
	.enSamplerate = AUDIO_SAMPLE_RATE_16000,
	.enSoundmode = AUDIO_SOUND_MODE_MONO,
	.enWorkmode = AIO_MODE_I2S_MASTER,
	.u32ChnCnt = 1,
	.u32ClkSel = 0,
	.u32EXFlag = 0,
	.u32FrmNum = 10,
	.u32PtNumPerFrm = 320,
};

static AIO_ATTR_S s_stAoAttr = {
	.enBitwidth = AUDIO_BIT_WIDTH_16,
	.enI2sType = AIO_I2STYPE_INNERCODEC,
	.enSamplerate = AUDIO_SAMPLE_RATE_16000,
	.enSoundmode = AUDIO_SOUND_MODE_MONO,
	.enWorkmode = AIO_MODE_I2S_MASTER,
	.u32ChnCnt = 1,
	.u32ClkSel = 0,
	.u32EXFlag = 0,
	.u32FrmNum = 10,
	.u32PtNumPerFrm = AUDIO_PERIOD_SIZE,
};

static AI_TALKVQE_CONFIG_S s_stVqeAttr = {
	.u32OpenMask = (NR_ENABLE | AGC_ENABLE | DCREMOVER_ENABLE),
	// .u32OpenMask = LP_AEC_ENABLE | NLP_AES_ENABLE |
	// 				NR_ENABLE | AGC_ENABLE,
	.para_notch_freq = 0,
	.s32WorkSampleRate = AUDIO_SAMPLE_RATE_16000,

	.stAgcCfg.para_agc_max_gain = 0,
	.stAgcCfg.para_agc_target_high = 2,
	.stAgcCfg.para_agc_target_low = 72,
	.stAgcCfg.para_agc_vad_ena = CVI_TRUE,

	.stAnrCfg.para_nr_snr_coeff = 15,
	.stAnrCfg.para_nr_init_sile_time = 0,

	// .stAecCfg.para_aec_filter_len = 13,
	// .stAecCfg.para_aes_std_thrd = 37,
	// .stAecCfg.para_aes_supp_coeff = 60,
};

static CVI_S32 audio_ai_init()
{
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_AI_SetPubAttr(AiDevId, &s_stAiAttr);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AI_SetPubAttr failed with %#x\n", s32Ret);
		return s32Ret;
	}

	// s32Ret = CVI_AI_SetVolume(AiDevId, 12);
	// if(s32Ret != CVI_SUCCESS){
	// 	APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AI_SetVolume failed with %#x\n");
	// 	return s32Ret;
	// }

	s32Ret = CVI_AI_EnableChn(AiDevId, AiChn);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AI_EnableChn failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_AI_SetTalkVqeAttr(AiDevId, AiChn, 0, 0, &s_stVqeAttr);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AI_SetTalkVqeAttr failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_AI_EnableVqe(AiDevId, AiChn);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AI_EnableVqe failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_AI_EnableReSmp(AiDevId, AiChn, AUDIO_SAMPLE_RATE_48000);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AI_EnableReSmp failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_AI_Enable(AiDevId);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AI_Enable failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_VOID audio_ai_exit()
{
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_AI_DisableChn(AiDevId, AiChn);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AI_DisableChn failed with %#x\n", s32Ret);
	}

	s32Ret = CVI_AI_Disable(AiDevId);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AI_Disable failed with %#x\n", s32Ret);
	}
}

static CVI_S32 audio_ao_init()
{
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_AO_SetPubAttr(AoDev, &s_stAoAttr);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AO_SetPubAttr failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_AO_Enable(AoDev);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AO_Enable failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_AO_EnableChn(AoDev, AoChn);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AO_EnableChn failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_AO_EnableReSmp(AoDev, AoChn, AUDIO_SAMPLE_RATE_48000);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AO_EnableChn failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_VOID audio_ao_exit()
{
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_AO_DisableChn(AoDev, AoChn);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AO_DisableChn failed with %#x\n", s32Ret);
	}

	s32Ret = CVI_AO_Disable(AoDev);
	if(s32Ret != CVI_SUCCESS){
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_AO_Disable failed with %#x\n", s32Ret);
	}
}

static void mono_2_stereo(const int16_t *src_audio,
            int src_len, int16_t *dst_audio) {
    for (int i = 0; i < src_len; i++) {
		// dst_audio[2*i] = src_audio[i];
		// dst_audio[2*i+1] = 0;
        dst_audio[2*i] = dst_audio[2*i+1] = src_audio[i];
    }
}

#define COMBINE(l,r) (((int32_t)(l) + (r)) >> 1)

void stereo_2_mono(const int16_t *src_audio,
        int dst_len, int16_t *dst_audio) {
    for (int i = 0; i < dst_len; i++) {
        dst_audio[i] = COMBINE(src_audio[2*i], src_audio[2*i+1]);
		// dst_audio[i] = src_audio[(2 * i)];
    }
}

#endif

#if UPLINK_USE_CYCLE_BUFFER
static void *thread_uplink_src_audio(void *arg)
{
#if 0
#define UPLINK_SRC_OPEN_STATE 0x61
#define UPLINK_SRC_DETECT_STATE 0x62
#define UPLINK_SRC_SEND_DATA_STATE 0x63
#define UPLINK_SRC_FORCE_FLUSH_STATE 0x64
#define UPLINK_SRC_CLOSE_AND_RETRY_STATE 0x65
#endif
	int sizebytes = 0;
	int ret_b;
	int ret_b_count = 0;
	int flush_count = 0;
#ifndef USE_CVI_API
	struct pcm *pcm_recordhandleup = NULL;
#endif
	char zero_bufferup[AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2];
	int uplink_srcstate = UPLINK_SRC_OPEN_STATE;
#define EDGE_AS_SRC_STABLE_THRESHOLD 2

	sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
	memset(zero_bufferup, 0, (AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2));

	while (bUplinkEnable) {
		switch (uplink_srcstate) {
		case UPLINK_SRC_OPEN_STATE:
		{
		#if USE_CVI_API
			uplink_srcstate = UPLINK_SRC_DETECT_STATE;
		#else
			if (err_read != 0) {
				pcm_recordhandleup = pcm_open(0, 0, PCM_IN, &stPcmCapConfigUp);
				if (!pcm_recordhandleup || !pcm_is_ready(pcm_recordhandleup)) {
					APP_PROF_LOG_PRINT(LEVEL_ERROR,"Unable to openpcm_recordhandle (%s)\n",
							pcm_get_error(pcm_recordhandleup));
					APP_PROF_LOG_PRINT(LEVEL_ERROR,"\n");
					usleep(500*1000);
					sleep(1);
					//force return for the open failure;
				} else {
					err_read = 0;
				}
			}
			uplink_srcstate = UPLINK_SRC_DETECT_STATE;
		#endif
		}
		break;

		case UPLINK_SRC_DETECT_STATE:
		{
		#if USE_CVI_API
			uplink_srcstate = UPLINK_SRC_SEND_DATA_STATE;
		#else
			sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
			err_read = pcm_read(pcm_recordhandleup, buffer_up_src, sizebytes);

			if (err_read == 0) {
				detect_mic_count++;
				if (detect_mic_count > EDGE_AS_SRC_STABLE_THRESHOLD) {
					uplink_srcstate = UPLINK_SRC_SEND_DATA_STATE;
					detect_mic_count = 0;
				} else {
					uplink_srcstate = UPLINK_SRC_DETECT_STATE;
				}
			} else {
				detect_mic_count = 0;
				uplink_srcstate = UPLINK_SRC_CLOSE_AND_RETRY_STATE;
				 sleep(1);
				 usleep(500*1000);
			}
			uplink_srcstate = UPLINK_SRC_SEND_DATA_STATE;
		#endif
		}
		break;

		case UPLINK_SRC_SEND_DATA_STATE:
		{
		#if USE_CVI_API
			CVI_S32 s32Ret = CVI_FAILURE;
			AUDIO_FRAME_S stFrame;
			AEC_FRAME_S   stAecFrm;
			s32Ret = CVI_AI_GetFrame(AiDevId, AiChn, &stFrame, &stAecFrm, -1);
			if (s32Ret != CVI_SUCCESS) {
				APP_PROF_LOG_PRINT(LEVEL_DEBUG, "CVI_AI_GetFrame failed with %#x\n", s32Ret);
				usleep(1000);
				continue;
			}
			sizebytes = stFrame.u32Len * BYTES_PER_SAMPLE * 1; // MONO
			mono_2_stereo((int16_t *)stFrame.u64VirAddr[0], sizebytes / BYTES_PER_SAMPLE, (int16_t *)buffer_up_src);
			sizebytes *= 2;
		#else
			sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
			err_read = pcm_read(pcm_recordhandleup, buffer_up_src, sizebytes);
			if (err_read != 0) {
				APP_PROF_LOG_PRINT(LEVEL_ERROR,"card 0[%d][%s]\n", __LINE__, pcm_get_error(pcm_recordhandleup));
				APP_PROF_LOG_PRINT(LEVEL_ERROR,"[uplinksrc_audio src] pcm read NG, go close and retry....\n");
				sleep(1);
				usleep(500*1000);
				uplink_srcstate = UPLINK_SRC_CLOSE_AND_RETRY_STATE;
				break;
			}
		#endif
			ret_b =  CycleBufferWrite(gpstUplinkCB, buffer_up_src, sizebytes);
			if (!ret_b) {
				//buffer abnormal
				ret_b_count++;
				usleep(1000*sizebytes/(64));
				if (ret_b_count >= 8) {
				ret_b = CycleBufferDataLen(gpstUplinkCB);
				//cannot write into buffer/ cycle buffer  overflow
				//cycle buffer not consuming or usb did not connect in uplink dst
				//go to force flush
				uplink_srcstate = UPLINK_SRC_FORCE_FLUSH_STATE;
				ret_b_count = 0;
				}
			} else {
				//buffer ok
				uplink_srcstate = UPLINK_SRC_SEND_DATA_STATE;
				ret_b_count = 0;
				flush_count = 0;
			}
		}
		break;

		case UPLINK_SRC_FORCE_FLUSH_STATE:
		{
			int retsize = 0;

			ret_b = CycleBufferDataLen(gpstUplinkCB);
			if (ret_b != 0)
				retsize = CycleBufferRead(gpstUplinkCB, pflush_up_buffer, ret_b);
			if (retsize != ret_b)
				APP_PROF_LOG_PRINT(LEVEL_ERROR,"[Error]size mismatch[%d]\n", __LINE__);


			flush_count++;
			if (flush_count > 10) {
				uplink_srcstate = UPLINK_SRC_CLOSE_AND_RETRY_STATE;
				flush_count = 0;
				//usb as dst has close for a while
			} else {
				//retry to send data into cycle buffer
				uplink_srcstate = UPLINK_SRC_SEND_DATA_STATE;
			}

		}
		break;

		case UPLINK_SRC_CLOSE_AND_RETRY_STATE:
		{
			free(buffer_up_src);
			#if USE_CVI_API
				// audio_ai_exit();
			#else
				if (err_read != 0) {
					pcm_close(pcm_recordhandleup);
					pcm_recordhandleup = NULL;
				}
			#endif
			buffer_up_src = (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
			sleep(1);
			uplink_srcstate = UPLINK_SRC_OPEN_STATE;
		}
		break;

		default:
			APP_PROF_LOG_PRINT(LEVEL_ERROR,"[Error]uplink edge(mic in) as src abnormal...[%d]\n", uplink_srcstate);
			sleep(2);
			break;
		}
	}
	return (void *)0;
}


static void *thread_uplink_dst_audio(void *arg)
{
#if 0
#define UPLINK_DST_OPEN_STATE 0x71
#define UPLINK_DST_DETECT_STATE 0x72
#define UPLINK_DST_SEND_DATA_STATE 0x73
#define UPLINK_DST_SEND_DATA_MUTE_STATE 0x74
#define UPLINK_DST_CLOSE_AND_RETRY_STATE 0x75
#endif
	int err_write = -1;
	int ret_b = 0;
	struct pcm *pcm_playhandleup = NULL;
	int detect_usb_count = 0;
	char zero_bufferup[AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2];
	int sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
#define  USB_AS_SPK_STABLE_THRESHOLD 2
	memset(zero_bufferup, 0, (AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2));

	int uplink_dststate = UPLINK_DST_OPEN_STATE;

	while (bUplinkEnable) {
		switch (uplink_dststate) {
		case UPLINK_DST_OPEN_STATE:
		{
			if (err_write != 0) {
				pcm_playhandleup = pcm_open(2, 0, PCM_OUT, &stPcmPlayConfigUp);
				if (!pcm_playhandleup || !pcm_is_ready(pcm_playhandleup)) {
					usleep(500*1000);
					//return 0;
					//force return for the open failure
				} else {
					err_write = 0;
				}
			}
			uplink_dststate = UPLINK_DST_DETECT_STATE;
		}
		break;

		case UPLINK_DST_DETECT_STATE:
		{
			err_write = pcm_write(pcm_playhandleup, zero_bufferup, sizebytes);
			if (err_write != 0) {
				sleep(2);
				detect_usb_count = 0;
				memset(zero_bufferup, 0, sizebytes);
				uplink_dststate = UPLINK_DST_CLOSE_AND_RETRY_STATE;
			} else {
				detect_usb_count++;
				if (detect_usb_count > USB_AS_SPK_STABLE_THRESHOLD) {
					uplink_dststate = UPLINK_DST_SEND_DATA_STATE;
					detect_usb_count = 0;
				} else {
					uplink_dststate = UPLINK_DST_DETECT_STATE;
				}
			}
		}
		break;

		case UPLINK_DST_SEND_DATA_STATE:
		{
			ret_b = CycleBufferDataLen(gpstUplinkCB);
			if (ret_b >= sizebytes * 2) {
				//sufficient with buffering 2 frame in cycble buffer
				//send the true data
				ret_b = CycleBufferRead(gpstUplinkCB, buffer_up_dst, sizebytes);
				if (ret_b != sizebytes) {
					//...cycle buffer abnormal
				}
				err_write = pcm_write(pcm_playhandleup, buffer_up_dst, sizebytes);
				if (err_write != 0) {
					sleep(2);
					uplink_dststate = UPLINK_DST_CLOSE_AND_RETRY_STATE;
				}  else {
					//send data success ...keep sending from cycle buffer
					uplink_dststate = UPLINK_DST_SEND_DATA_STATE;
				}
			} else {
				//buffer not enough go to mute state
				uplink_dststate = UPLINK_DST_SEND_DATA_MUTE_STATE;
			}
		}
		break;

		case UPLINK_DST_SEND_DATA_MUTE_STATE:
		{
			err_write = pcm_write(pcm_playhandleup, zero_bufferup, sizebytes);
			if (err_write != 0) {
				sleep(2);
				uplink_dststate = UPLINK_DST_CLOSE_AND_RETRY_STATE;
			}  else {
				ret_b = CycleBufferDataLen(gpstUplinkCB);
				if (ret_b < sizebytes * 2) {
					//cycle buffer underrun
					uplink_dststate = UPLINK_DST_SEND_DATA_MUTE_STATE;
					usleep(10*1000);
				} else {
					//cycle buffer enough data
					//go to send data
					uplink_dststate = UPLINK_DST_SEND_DATA_STATE;
				}
			}
		}
		break;

		case UPLINK_DST_CLOSE_AND_RETRY_STATE:
		{
			#ifdef THIS_IS_32
			#else
			free(buffer_up_dst);
			#endif
			if (err_write != 0) {
				pcm_close(pcm_playhandleup);
				pcm_playhandleup = NULL;
			}
			#ifdef THIS_IS_32
			#else
			buffer_up_dst = (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
			#endif
			usleep(500*1000);
			sleep(1);
			uplink_dststate = UPLINK_DST_OPEN_STATE;
		}
		break;
		default:
			APP_PROF_LOG_PRINT(LEVEL_ERROR,"[Error]Not a valid state in uplink dst(to usb)[%d]\n", uplink_dststate);
			sleep(2);
			break;
		}
	}

	return (void *)0;
}
#else

#endif
struct pcm_config stPcmCapConfig = {
#if CV18XX_AS_HOST
	.channels = 1,//CHANNEL_COUNT,
#else
	.channels = CHANNEL_COUNT,
#endif
	.rate = SAMPLE_RATE,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = CAP_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};


struct pcm_config stPcmPlayConfig = {
#if CV18XX_AS_HOST
	.channels = 1,//CHANNEL_COUNT,
#else
	.channels = CHANNEL_COUNT,
#endif
	.rate = SAMPLE_RATE,
	.period_size = AUDIO_PERIOD_SIZE,
	.period_count = PLAY_PERIOD_COUNT,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};

#if DOWNLINK_USE_CYCLE_BUFFER
static void *thread_downlink_src_audio(void *arg)
{
	//pcm stttus
	int err_read = -1; // -1 : not  success, 0 open & read success
	int sizebytes = 0;
	//cycle buffer status
	int ret_b;
	int ret_b_count = 0;
	//handle
	struct pcm *pcm_recordhandle = NULL;
	char zero_downbuffer[AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2];
	int downlink_src_StableCnt = 0;
#define  USB_AS_SRC_STABLE_THRESHOLD 5

	memset(zero_downbuffer, 0, (AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT));

	int downlink_srcstate = DOWNLINK_SRC_OPEN_STATE;
	app_ipcam_Gpio_Value_Set(CVI_GPIOE_02, CVI_GPIO_VALUE_H);
	while (bDownlinkEnable) {
		switch (downlink_srcstate) {
		case DOWNLINK_SRC_OPEN_STATE:
		{
			if (err_read != 0) {
				pcm_recordhandle = pcm_open(0, 0, PCM_IN, &stPcmCapConfig);
				if (!pcm_recordhandle || !pcm_is_ready(pcm_recordhandle)) {
					APP_PROF_LOG_PRINT(LEVEL_ERROR,"\n");
					usleep(500*1000);

				} else {
					err_read = 0;
				}
			}

			downlink_srcstate = DOWNLINK_SRC_DETECT_STATE;
		}
		break;

		case DOWNLINK_SRC_DETECT_STATE:
		{
			sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
			err_read = pcm_read(pcm_recordhandle, buffer_dl_src, sizebytes);
			if (err_read == 0) {
				downlink_src_StableCnt++;
				if (downlink_src_StableCnt > USB_AS_SRC_STABLE_THRESHOLD) {
					downlink_srcstate = DOWNLINK_SRC_SEND_DATA_STATE;
					downlink_src_StableCnt = 0;
				} else {
					downlink_srcstate = DOWNLINK_SRC_DETECT_STATE;
				}
			} else {
				downlink_src_StableCnt = 0;
				downlink_srcstate = DOWNLINK_SRC_CLOSE_AND_RETRY_STATE;
				 sleep(1);
				 usleep(500*1000);
			}

		}
		break;

		case DOWNLINK_SRC_SEND_DATA_STATE:
		{
			sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
			err_read = pcm_read(pcm_recordhandle, buffer_dl_src, sizebytes);
			if (err_read != 0) {
				APP_PROF_LOG_PRINT(LEVEL_ERROR,"card 2 not get data from PC\n");
				APP_PROF_LOG_PRINT(LEVEL_ERROR,"card 2[%s]\n", pcm_get_error(pcm_recordhandle));
				sleep(1);
				usleep(500*1000);
				downlink_srcstate = DOWNLINK_SRC_CLOSE_AND_RETRY_STATE;
			} else {
				downlink_srcstate = DOWNLINK_SRC_SEND_DATA_STATE;
				ret_b =  CycleBufferWrite(gpstDownlinkCB, buffer_dl_src, sizebytes);
				if (!ret_b) {
					//buffer abnormal
					ret_b_count++;
					usleep(1000*sizebytes/(64));
					if (ret_b_count >= 10) {
					ret_b = CycleBufferDataLen(gpstDownlinkCB);
					}
				} else {
					//buffer ok
					ret_b_count = 0;
				}
			}
		}
		break;

		case DOWNLINK_SRC_CLOSE_AND_RETRY_STATE:
		{
			free(buffer_dl_src);
			if (err_read != 0) {
				pcm_close(pcm_recordhandle);
				pcm_recordhandle = NULL;
			}
			buffer_dl_src = (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
			sleep(1);
			downlink_srcstate = DOWNLINK_SRC_OPEN_STATE;
		}
		break;

		default:
			APP_PROF_LOG_PRINT(LEVEL_ERROR,"[Error]downlink usb as src abnormal...[%d]\n", downlink_srcstate);
			sleep(2);
			break;

		}
	}
	app_ipcam_Gpio_Value_Set(CVI_GPIOE_02, CVI_GPIO_VALUE_L);
	return (void *)0;
}
static void *thread_downlink_dst_audio(void *arg)
{

	int err_write = -1;
	int ret_b = 0;
	char zero_downbuffer[AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT*2];
	int sizebytes = AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT;
#define  EDGE_AS_SPK_STABLE_THRESHOLD 1
	memset(zero_downbuffer, 0, (AUDIO_PERIOD_SIZE*2*CHANNEL_COUNT*2));
	//downlink dst is output microphone
	int downlink_dststate = DOWNLINK_DST_OPEN_STATE;

#if USE_CVI_API
	char dst_mono_buf[AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE] = {0};
#endif

	while (bDownlinkEnable) {
		switch (downlink_dststate) {
		case DOWNLINK_DST_OPEN_STATE:
		{
		#if USE_CVI_API
			downlink_dststate = DOWNLINK_DST_DETECT_STATE;
		#else
			if (err_write != 0) {
				pcm_playhandle = pcm_open(1, 0, PCM_OUT, &stPcmPlayConfig);
				if (!pcm_playhandle || !pcm_is_ready(pcm_playhandle)) {
					APP_PROF_LOG_PRINT(LEVEL_ERROR,"Unable to open pcm_playhandle (%s)\n",
						pcm_get_error(pcm_playhandle));
					APP_PROF_LOG_PRINT(LEVEL_ERROR,"\n");
					usleep(500*1000);
				} else {
					err_write = 0;
				}
			} else {
				err_write = 0;
				usleep(10*1000);
			}
			downlink_dststate = DOWNLINK_DST_DETECT_STATE;
		#endif
		}
		break;

		case DOWNLINK_DST_DETECT_STATE:
		{
		#if USE_CVI_API
			// !!! Send mute data will delay 5s?

			// AUDIO_FRAME_S stFrame;
			// stFrame.u64VirAddr[0] = (CVI_U8 *)zero_downbuffer;
			// stFrame.u32Len = AUDIO_PERIOD_SIZE;//samples size for each channel
			// stFrame.u64TimeStamp = 0;
			// stFrame.enSoundmode = AUDIO_SOUND_MODE_MONO;
			// stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
			// err_write = CVI_AO_SendFrame(AoDev, AoChn, (const AUDIO_FRAME_S *)&stFrame, 1000);
			downlink_dststate = DOWNLINK_DST_SEND_DATA_STATE;
		#else
			err_write = pcm_write(pcm_playhandle, zero_downbuffer, sizebytes);
			if (err_write != 0) {
				APP_PROF_LOG_PRINT(LEVEL_ERROR,"card 1[%d][%s]\n", __LINE__, pcm_get_error(pcm_playhandle));
				detect_spk_count = 0;
				sleep(1);
				downlink_dststate = DOWNLINK_DST_CLOSE_AND_RETRY_STATE;
			} else {
				detect_spk_count++;
				if (detect_spk_count > EDGE_AS_SPK_STABLE_THRESHOLD) {
					downlink_dststate = DOWNLINK_DST_SEND_DATA_STATE;
					detect_spk_count = 0;
				} else {
					downlink_dststate = DOWNLINK_DST_DETECT_STATE;
				}
			}
		#endif
		}
		break;

		case DOWNLINK_DST_SEND_DATA_STATE:
		{
		#if USE_CVI_API
			ret_b = CycleBufferDataLen(gpstDownlinkCB);
			if (ret_b >= sizebytes * 2) {
				//sufficient with buffering 2 frame in cycble buffer
				//send the true data
				ret_b = CycleBufferRead(gpstDownlinkCB, buffer_dl_dst, sizebytes);

				AUDIO_FRAME_S stFrame;
				stereo_2_mono((int16_t *)buffer_dl_dst, AUDIO_PERIOD_SIZE, (int16_t *)dst_mono_buf);

				stFrame.u64VirAddr[0] = (CVI_U8 *)dst_mono_buf;
				stFrame.u32Len = AUDIO_PERIOD_SIZE;
				stFrame.u64TimeStamp = 0;
				stFrame.enSoundmode = AUDIO_SOUND_MODE_MONO;
				stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
				err_write = CVI_AO_SendFrame(AoDev, AoChn, (const AUDIO_FRAME_S *)&stFrame, 1000);
				if (err_write != 0) {
					// APP_PROF_LOG_PRINT(LEVEL_ERROR,"card 1[%d][%s]\n", __LINE__, pcm_get_error(pcm_playhandle));
					APP_PROF_LOG_PRINT(LEVEL_ERROR,"Error pcm_write failure in downlink dst(spk)...go retry\n");
					sleep(2);
					downlink_dststate = DOWNLINK_DST_CLOSE_AND_RETRY_STATE;
				}  else {
					//send data success ...keep sending from cycle buffer
					downlink_dststate = DOWNLINK_DST_SEND_DATA_STATE;
				}
			} else {
				// !!! Send mute data will delay 5s?

				//buffer not enough go to mute state
				// downlink_dststate = DOWNLINK_DST_SEND_DATA_MUTE_STATE;
			}
		#else
			ret_b = CycleBufferDataLen(gpstDownlinkCB);
			if (ret_b >= sizebytes * 2) {
				//sufficient with buffering 2 frame in cycble buffer
				//send the true data
				ret_b = CycleBufferRead(gpstDownlinkCB, buffer_dl_dst, sizebytes);
				if (ret_b != sizebytes) {
					//...cycle buffer abnormal
				}
				err_write = pcm_write(pcm_playhandle, buffer_dl_dst, sizebytes);
				if (err_write != 0) {
					APP_PROF_LOG_PRINT(LEVEL_ERROR,"card 1[%d][%s]\n", __LINE__, pcm_get_error(pcm_playhandle));
					APP_PROF_LOG_PRINT(LEVEL_ERROR,"Error pcm_write failure in downlink dst(spk)...go retry\n");
					sleep(2);
					downlink_dststate = DOWNLINK_DST_CLOSE_AND_RETRY_STATE;
				}  else {
					//send data success ...keep sending from cycle buffer
					downlink_dststate = DOWNLINK_DST_SEND_DATA_STATE;
				}
			} else {
				//buffer not enough go to mute state
				downlink_dststate = DOWNLINK_DST_SEND_DATA_MUTE_STATE;
			}
		#endif
		}
		break;

		case DOWNLINK_DST_SEND_DATA_MUTE_STATE:
		{
		#if USE_CVI_API
			// AUDIO_FRAME_S stFrame;
			// stFrame.u64VirAddr[0] = (CVI_U8 *)zero_downbuffer;
			// stFrame.u32Len = AUDIO_PERIOD_SIZE;//samples size for each channel
			// stFrame.u64TimeStamp = 0;
			// stFrame.enSoundmode = AUDIO_SOUND_MODE_MONO;
			// stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
			// err_write = CVI_AO_SendFrame(AoDev, AoChn, (const AUDIO_FRAME_S *)&stFrame, 1000);
			// // err_write = pcm_write(pcm_playhandle, zero_downbuffer, sizebytes);
			// if (err_write != 0) {
			// 	APP_PROF_LOG_PRINT(LEVEL_ERROR,"Error pcm_write Mute failure in downlink dst(spk)...go retry\n");
			// 	sleep(2);
			// 	downlink_dststate = DOWNLINK_DST_CLOSE_AND_RETRY_STATE;
			// }  else {
			// 	ret_b = CycleBufferDataLen(gpstDownlinkCB);
			// 	if (ret_b < sizebytes * 2) {
			// 		//cycle buffer underrun
			// 		downlink_dststate = DOWNLINK_DST_SEND_DATA_MUTE_STATE;
			// 	} else {
			// 		//cycle buffer enough data
			// 		//go to send data
			// 		downlink_dststate = DOWNLINK_DST_SEND_DATA_STATE;
			// 	}
			// }
		#else
			err_write = pcm_write(pcm_playhandle, zero_downbuffer, sizebytes);
			if (err_write != 0) {
				APP_PROF_LOG_PRINT(LEVEL_ERROR,"Error pcm_write Mute failure in downlink dst(spk)...go retry\n");
				sleep(2);
				downlink_dststate = DOWNLINK_DST_CLOSE_AND_RETRY_STATE;
			}  else {
				ret_b = CycleBufferDataLen(gpstDownlinkCB);
				if (ret_b < sizebytes * 2) {
					//cycle buffer underrun
					downlink_dststate = DOWNLINK_DST_SEND_DATA_MUTE_STATE;
				} else {
					//cycle buffer enough data
					//go to send data
					downlink_dststate = DOWNLINK_DST_SEND_DATA_STATE;
				}
			}
		#endif
		}
		break;

		case DOWNLINK_DST_CLOSE_AND_RETRY_STATE:
		{
			free(buffer_dl_dst);
		#if (!USE_CVI_API)
			if (err_write != 0) {
				pcm_close(pcm_playhandle);
				pcm_playhandle = NULL;
			}
		#endif
			buffer_dl_dst = (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
			usleep(500*1000);
			downlink_dststate = DOWNLINK_DST_OPEN_STATE;
		}
		break;

		default:
		APP_PROF_LOG_PRINT(LEVEL_ERROR,"[Error]Not a valid state in downlink dst(speaker)[%d]\n", downlink_dststate);
		sleep(2);
		break;
		}
	}
	return 0;
}
#else

#endif

int cvi_audio_uac_uplink(void)
{
	bUplinkEnable = 1;
#if UPLINK_USE_CYCLE_BUFFER
	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_uplink_src_mic_thread, &attr, thread_uplink_src_audio, NULL);


	struct sched_param param2;
	pthread_attr_t attr2;

	param2.sched_priority = 80;
	pthread_attr_init(&attr2);
	pthread_attr_setschedpolicy(&attr2, SCHED_RR);
	pthread_attr_setschedparam(&attr2, &param2);
	pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_uplink_dst_usb_thread, &attr2, thread_uplink_dst_audio, NULL);
#else
	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_mic_to_pc_thread, &attr, thread_uplink_audio, NULL);
	//pthread_detach(pcm_mic_to_pc_thread);
#endif
	return 1;
}

int cvi_audio_uac_uplink_stop(void)
{
	bUplinkEnable = 0;
	return 1;
}

int cvi_audio_uac_downlink(void)
{
	bDownlinkEnable = 1;
#if DOWNLINK_USE_CYCLE_BUFFER
	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_downlink_src_usb_thread, NULL, thread_downlink_src_audio, NULL);


	struct sched_param param2;
	pthread_attr_t attr2;

	param2.sched_priority = 80;
	pthread_attr_init(&attr2);
	pthread_attr_setschedpolicy(&attr2, SCHED_RR);
	pthread_attr_setschedparam(&attr2, &param2);
	pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_downlink_dst_spk_thread, NULL, thread_downlink_dst_audio, NULL);

#else
	struct sched_param param;
	pthread_attr_t attr;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(pcm_pc_to_mic_thread, NULL, thread_downlink_audio, NULL);
	//pthread_detach(pcm_pc_to_mic_thread);
#endif
	return 1;
}

int cvi_audio_uac_downlink_stop(void)
{
	bDownlinkEnable = 0;
	return 1;
}

int  cvi_audio_init_uac(void)
{
	printf("init...easy uac audio api[%s]\n", __VERSION_TAG__);
	cvi_audio_uac_getDbgMask();
	bUplinkEnable = 0;
	bDownlinkEnable = 0;

#if DOWNLINK_USE_CYCLE_BUFFER

	CycleBufferInit(&gpstDownlinkCB, DOWNLINK_CYCLE_BUFFER_SIZE);
	pDownlinkCBuffer = (char *)malloc(DOWNLINK_CYCLE_BUFFER_SIZE);
	buffer_dl_src =  (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
	buffer_dl_dst =  (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
	pcm_downlink_src_usb_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pcm_downlink_dst_spk_thread = (pthread_t *)malloc(sizeof(pthread_t));

#else
	buffer = (char *)malloc(AUDIO_PERIOD_SIZE*2*CHANNEL_COUNT*2);
	pcm_pc_to_mic_thread = (pthread_t *)malloc(sizeof(pthread_t));
#endif


#if UPLINK_USE_CYCLE_BUFFER
	pcm_uplink_src_mic_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pcm_uplink_dst_usb_thread = (pthread_t *)malloc(sizeof(pthread_t));
	CycleBufferInit(&gpstUplinkCB, UPLINK_CYCLE_BUFFER_SIZE);
	buffer_up_src =  (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
	buffer_up_dst =  (char *)malloc(AUDIO_PERIOD_SIZE*BYTES_PER_SAMPLE*CHANNEL_COUNT);
	pflush_up_buffer = (char *)malloc(UPLINK_CYCLE_BUFFER_SIZE);
#else
	pcm_mic_to_pc_thread = (pthread_t *)malloc(sizeof(pthread_t));
	bufferup = (char *)malloc(960 * 2 * 4);
	bufferup2 = (char *)malloc(960 * 2 * 4);
#endif

	return 1;
}

int cvi_audio_deinit_uac(void)
{
	int s32Ret;

	printf("deinit audio ...easy audio api\n");
	bUplinkEnable = 0;
	bDownlinkEnable = 0;
#if UPLINK_USE_CYCLE_BUFFER
	if (*pcm_uplink_src_mic_thread != 0) {
		s32Ret = pthread_join(*pcm_uplink_src_mic_thread, NULL);
		if (s32Ret == 0)
			*pcm_uplink_src_mic_thread = 0;
	}

	if (*pcm_uplink_dst_usb_thread != 0) {
		s32Ret = pthread_join(*pcm_uplink_dst_usb_thread, NULL);
		if (s32Ret == 0)
			*pcm_uplink_dst_usb_thread = 0;
	}
	//destroy cycle  buffer
	CycleBufferDestory(gpstUplinkCB);
	gpstUplinkCB = NULL;
	if (pUplinkCBuffer != NULL)
		free(pUplinkCBuffer);
	//free buffer
	if (buffer_up_src != 0)
		free(buffer_up_src);

	if (buffer_up_dst != 0)
		free(buffer_up_dst);
#else
	if (*pcm_mic_to_pc_thread != 0) {
		s32Ret = pthread_join(*pcm_mic_to_pc_thread, NULL);
		if (s32Ret == 0)
			*pcm_mic_to_pc_thread = 0;

	}

	if (bufferup != 0)
		free(bufferup);
	if (bufferup2 != 0)
		free(bufferup2);
#endif

#if DOWNLINK_USE_CYCLE_BUFFER
	//clear downlink src / dst thread
	if (*pcm_downlink_src_usb_thread != 0) {
		s32Ret = pthread_join(*pcm_downlink_src_usb_thread, NULL);
		if (s32Ret == 0)
			*pcm_downlink_src_usb_thread = 0;
	}
	if (*pcm_downlink_dst_spk_thread != 0) {
		s32Ret = pthread_join(*pcm_downlink_dst_spk_thread, NULL);
		if (s32Ret == 0)
			*pcm_downlink_dst_spk_thread = 0;
	}
	//destroy cycle  buffer
	CycleBufferDestory(gpstDownlinkCB);
	gpstDownlinkCB = NULL;
	if (pDownlinkCBuffer != NULL)
		free(pDownlinkCBuffer);
	//free buffer
	if (buffer_dl_src != 0)
		free(buffer_dl_src);

	if (buffer_dl_dst != 0)
		free(buffer_dl_dst);
#else
	if (*pcm_pc_to_mic_thread != 0) {
		s32Ret = pthread_join(*pcm_pc_to_mic_thread, NULL);
		if (s32Ret == 0)
			*pcm_pc_to_mic_thread = 0;

	}
	if (buffer != 0)
		free(buffer);
#endif
	return 0;
}


static int  cviUacGetEnv(char *env, char *fmt, void *param)
{
#define NOT_SET -2
#define INPUT_ERR -1
	char *pEnv;
	int val = NOT_SET;

	pEnv = getenv(env);
	if (pEnv) {
		if (strcmp(fmt, "%s") == 0) {
			strcpy(param, pEnv);
		} else {
			if (sscanf(pEnv, fmt, &val) != 1)
				return INPUT_ERR;
			printf("[UAC]%s = %d\n", env, val);
		}
	} else
		printf("[Err][%s][%d]getenv failure\n", __func__, __LINE__);

	return val;
}
static void cvi_audio_uac_getDbgMask(void)
{
#define NOT_SET -2
#define INPUT_ERR -1
	int  *pUAC_level = &gUAC_level;
	int getValue;

	*pUAC_level = 0;

	getValue = cviUacGetEnv("uac_level", "%d", NULL);
	if (getValue == NOT_SET || getValue == INPUT_ERR) {
		printf("uac level not set or input err..force level=1\n");
		*pUAC_level  = CVI_UAC_MASK_DBG;
	} else
		*pUAC_level = getValue;

	printf("[UAC]uac_level = [%d]\n", (*pUAC_level));
}

// void sig_handler(int signo) {
//     if (SIGTERM == signo || SIGINT == signo) {
// 		cvi_audio_deinit_uac();
// 	#if USE_CVI_API
//         audio_ai_exit();
// 		audio_ao_exit();
// 		CVI_AUDIO_DEINIT();
// 		printf("cvi audio exit done\n");
// 	#endif
//     }
// 	exit(0);
// }

void app_uac_exit()
{
	if(!s_uac_init){
        printf("uac not init\n");
        return;
    }

	cvi_audio_deinit_uac();
	#if USE_CVI_API
        audio_ai_exit();
		audio_ao_exit();
		CVI_AUDIO_DEINIT();
		printf("cvi audio exit done\n");
	#endif
}

int app_uac_init()
{
	char uac_devname[32] = "/dev/snd/controlC0";
	if(access(uac_devname, F_OK) != 0){
		printf("file %s not found\n", uac_devname);
		return -1;
	}

	cvi_audio_init_uac();

#if USE_CVI_API
	CVI_AUDIO_INIT();
#endif

#if USE_CVI_API
	audio_ai_init();
#endif
	printf("start uplink after....\n");
	cvi_audio_uac_uplink();

#if USE_CVI_API
	audio_ao_init();
#endif
	printf("start downlink....\n");
	cvi_audio_uac_downlink();

	s_uac_init = true;
	return 0;
}

// #ifndef ENABLE_UVC_AUDIO
// int main(void)
// {
// 	signal(SIGTERM, sig_handler);
//     signal(SIGINT, sig_handler);

// 	cvi_audio_init_uac();

// #if USE_CVI_API
// 	CVI_AUDIO_INIT();
// #endif

// #if USE_CVI_API
// 	audio_ai_init();
// #endif
// 	printf("start uplink after....\n");
// 	cvi_audio_uac_uplink();

// #if USE_CVI_API
// 	audio_ao_init();
// #endif
// 	printf("start downlink....\n");
// 	cvi_audio_uac_downlink();

// 	while (1) {
// 		//this is trying to keep thread alive
// 		sleep(10);
// 	}

// }
// #endif


