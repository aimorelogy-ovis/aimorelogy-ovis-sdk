#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "cvi_buffer.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_video.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_3a.h"
#include "cvi_vi.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"

#include "3A_internal.h"
#include "vi_ut_comm.h"
#include "vi_ut_isp_rawreplayoffline.h"


#if defined(__GNUC__) && defined(__riscv)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation" /* Or  "-Wformat-overflow"  */
#endif

#define LOGOUT(fmt, arg...) UT_PRT("%s,%d: " fmt, __func__, __LINE__, ##arg)

#define PATH_MAX_LEN 256
#define SEC_TO_USEC 1000000

#define START_PATH "/mnt/data/raw_replay_start"
#define RESET_PATH "/mnt/data/raw_replay_reset"
#define SET_PATH "/mnt/data/raw_replay_set"
#define SAVE_PATH "/mnt/data/raw_replay_save_start"
#define CMD_RM "rm -rf "
#define CMD_TOUCH "touch "
#define OFFLINE_CTRL_CMD_PATH "/mnt/data/offline_ctrl_cmd.bin"
#define YUV_DIR "/mnt/data/yuv_dir"
#define LOAD_FRAME (20)

extern CVI_S32 vi_ut_save_frame2file(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
extern CVI_S32 isp_feature_ctrl_get_algo_ret_addr(VI_PIPE ViPipe, ISP_ALGO_RESULT_S **pp_algo_ret);
extern CVI_S32 isp_mw_post_eof;

typedef enum {
	RAW_REPLAY_INIT,
	RAW_REPLAY_RELOAD,
	RAW_REPLAY_UPDATE,
	RAW_REPLAY_SLEEP,
	RAW_REPLAY_STOP,
} RAW_REPLAY_OFFLINE_STATUS;

typedef struct {
	RAW_REPLAY_INFO *pRawHeader;
	CVI_U16 *leSe;
	CVI_U8 *le;
	CVI_U8 *se;
	CVI_S32 mSz16b;
	CVI_U32 sz16b;
	CVI_U32 sz12b;
	FILE *rawFp;

	CVI_U8 isLoadRawFromMem;

	CVI_U8 *rawLeVir;
	CVI_U8 *rawSeVir;
	FILE *le12Fd;
	FILE *se12Fd;
	CVI_S32 rawBlockFrameStartId;
	CVI_S32 rawBlockFrameEndId;
	CVI_S32 blockMaxFrame;

	VI_PIPE viPipe;
	VB_POOL poolId;
	VB_BLK blk[2];
	CVI_U32 u32BlkSize;
	CVI_U64 u64PhyAddr[2];
	CVI_VOID *pu8VirAddr[2];

	pthread_t rawReplayTid;
	CVI_BOOL bRawReplayThreadEnabled;

	CVI_BOOL isRawReplayReady;
	RAW_REPLAY_OFFLINE_STATUS rawReplayStatus;
	VI_DEV_TIMING_ATTR_S timingAttr;
	OFFLINE_3A_ALGO_RET offline3aAlgoRet;
} RAW_REPLAY_BOARD_CTX_S;

static RAW_REPLAY_BOARD_CTX_S *pRawReplayBoardCtx;

static CVI_S32 load_rawFileName_from_path(CVI_U8 *path, CVI_U8 *rawFileName, CVI_S32 maxFileNameLen);
static CVI_S32 load_raw_info_from_file(CVI_U8 *boardPath, CVI_U8 *rawFileName, RAW_REPLAY_INFO *pRawInfo);
static CVI_VOID *raw_replay_offline_thread(CVI_VOID *arg);
static CVI_S32 isp_get_chn_frame(CVI_U8 chn, CVI_U32 curFrame, CVI_U32 loopNum);
static CVI_S32 waiting_isp_mw_post_eof(CVI_U32 timeoutMillSec);
static CVI_S32 load_raw_block(CVI_S32 frameStart, CVI_S32 frameEnd);
static CVI_U32 frameCnt;
static CVI_S32 isReset;
static CVI_S32 update_3a_algo(ISP_ALGO_RESULT_S *algoRet);

OFFLINE_CTRL_CMD offlineCtrlCmd;

CVI_S32 raw_replay_offline_init(CVI_U8 *boardPath, VI_USR_PIC_INFO_S *picInfo)
{
	CVI_U8 rawFileName[PATH_MAX_LEN];
	CVI_U8 rawFilePath[PATH_MAX_LEN];

	// Calloc and Reset Raw_Replay_Ctx
	pRawReplayBoardCtx = (RAW_REPLAY_BOARD_CTX_S *) calloc(1, sizeof(RAW_REPLAY_BOARD_CTX_S));
	if (pRawReplayBoardCtx == NULL) {
		LOGOUT("pRawReplayBoardCtx calloc fail!!!\n");
		return CVI_FAILURE;
	}
	memset(pRawReplayBoardCtx, 0, sizeof(RAW_REPLAY_BOARD_CTX_S));
	pRawReplayBoardCtx->rawReplayStatus = RAW_REPLAY_INIT;
	pRawReplayBoardCtx->poolId = VB_INVALID_POOLID;

	// Calloc and Reset Raw Info
	pRawReplayBoardCtx->pRawHeader = (RAW_REPLAY_INFO *) calloc(1, sizeof(RAW_REPLAY_INFO));
	if (pRawReplayBoardCtx->pRawHeader == NULL) {
		LOGOUT("pRawReplayBoardCtx->pRawHeader calloc failed!\n");
		return CVI_FAILURE;
	}
	memset(pRawReplayBoardCtx->pRawHeader, 0, sizeof(RAW_REPLAY_INFO));

	// Load Raw Info from path
	load_rawFileName_from_path(boardPath, rawFileName, PATH_MAX_LEN);
	LOGOUT("Loading Raw FileName:%s\n", rawFileName);
	load_raw_info_from_file(boardPath, rawFileName, pRawReplayBoardCtx->pRawHeader);
	RAW_REPLAY_INFO *pRawInfo = (RAW_REPLAY_INFO *) pRawReplayBoardCtx->pRawHeader;

	LOGOUT("RawFileInfo: %dX%d, numFrame=%d, WDR=%d, bayer=%d, size=%dX%d\n", pRawInfo->width,
					pRawInfo->height, pRawInfo->numFrame, pRawInfo->enWDR,
					pRawInfo->bayerID, pRawReplayBoardCtx->mSz16b, pRawInfo->numFrame);

	// Calloc and Reset Raw Data(16bit&12bit) by Raw Info
	pRawReplayBoardCtx->leSe = (CVI_U16 *) calloc(1, pRawReplayBoardCtx->mSz16b);
	if (pRawReplayBoardCtx->leSe == NULL) {
		LOGOUT("pRawReplayBoardCtx->leSe calloc failed!\n");
		return CVI_FAILURE;
	}
	memset(pRawReplayBoardCtx->leSe, 0, sizeof(pRawReplayBoardCtx->mSz16b));

	pRawReplayBoardCtx->le = (CVI_U8 *) calloc(1, pRawReplayBoardCtx->sz12b);
	if (pRawReplayBoardCtx->le == NULL) {
		LOGOUT("pRawReplayBoardCtx->le calloc failed!\n");
		return CVI_FAILURE;
	}
	memset(pRawReplayBoardCtx->le, 0, sizeof(pRawReplayBoardCtx->sz12b));

	if (pRawInfo->enWDR) {
		pRawReplayBoardCtx->se = (CVI_U8 *) calloc(1, pRawReplayBoardCtx->sz12b);
		if (pRawReplayBoardCtx->se == NULL) {
			LOGOUT("pRawReplayBoardCtx->se calloc failed!\n");
			return CVI_FAILURE;
		}
		memset(pRawReplayBoardCtx->se, 0, sizeof(pRawReplayBoardCtx->sz12b));
	}

	// Open RawFile to Load Data
	snprintf(rawFilePath, PATH_MAX_LEN, "%s/%s.raw", boardPath, rawFileName);
	pRawReplayBoardCtx->rawFp = fopen(rawFilePath, "rb");
	if (pRawReplayBoardCtx->rawFp == NULL) {
		LOGOUT("fopen %s failed!\n", rawFilePath);
		return CVI_FAILURE;
	}

	CVI_U8 *loadRawFromMem = getenv("LOAD_RAW_FROM_MEM");

	if (loadRawFromMem != NULL) {
		pRawReplayBoardCtx->isLoadRawFromMem = atoi(loadRawFromMem);
	} else {
		pRawReplayBoardCtx->isLoadRawFromMem = 0;
	}

	if (pRawReplayBoardCtx->isLoadRawFromMem) {
		CVI_U8 leRawPath[256] = {0};
		CVI_U8 seRawPath[256] = {0};

		snprintf(leRawPath, 256, "%s/le.bin", boardPath);
		snprintf(seRawPath, 256, "%s/se.bin", boardPath);

		UT_PRT("le raw path: %s, se raw path: %s\n",
				leRawPath, seRawPath);

		CVI_BOOL checkBinOk = (access(leRawPath, F_OK) == 0) &&
			((!pRawInfo->enWDR) || (access(seRawPath, F_OK) == 0));

		if (checkBinOk) {
			pRawReplayBoardCtx->rawBlockFrameStartId = 0;
			pRawReplayBoardCtx->blockMaxFrame = (pRawInfo->enWDR) ? LOAD_FRAME / 2  : LOAD_FRAME;
			pRawReplayBoardCtx->rawBlockFrameEndId = pRawReplayBoardCtx->rawBlockFrameStartId +
				pRawReplayBoardCtx->blockMaxFrame - 1;

			if (pRawReplayBoardCtx->rawBlockFrameEndId > pRawInfo->numFrame - 1) {
				pRawReplayBoardCtx->rawBlockFrameEndId = pRawInfo->numFrame - 1;
			}

			size_t sz = pRawReplayBoardCtx->sz12b * pRawReplayBoardCtx->blockMaxFrame;

			pRawReplayBoardCtx->rawLeVir = (CVI_U8 *)malloc(sz);

			if (pRawReplayBoardCtx->rawLeVir == NULL) {
				UT_PRT("fail to malloc the raw se vir, size: %zu!\n", sz);
				goto LOAD_FROM_MEM_FAIL;
			} else {
				UT_PRT("alloc rawLeVir: %p, size: %zu\n", pRawReplayBoardCtx->rawLeVir, sz);
			}

			if (pRawInfo->enWDR) {
				pRawReplayBoardCtx->rawSeVir = (CVI_U8 *)malloc(sz);

				if (pRawReplayBoardCtx->rawSeVir == NULL) {
					UT_PRT("fail to malloc the raw se vir, size: %zu!\n", sz);
					goto LOAD_FROM_MEM_FAIL;
				} else {
					UT_PRT("alloc rawSeVir: %p, size: %zu\n", pRawReplayBoardCtx->rawSeVir, sz);
				}
			}

			pRawReplayBoardCtx->le12Fd = fopen(leRawPath, "rb");
			if (pRawReplayBoardCtx->le12Fd == NULL) {
				UT_PRT("fail to open the %s\n", leRawPath);
				goto LOAD_FROM_MEM_FAIL;
			}

			if (pRawInfo->enWDR) {
				pRawReplayBoardCtx->se12Fd = fopen(seRawPath, "rb");
				if (pRawReplayBoardCtx->se12Fd == NULL) {
					UT_PRT("fail to open the %s\n", seRawPath);
					goto LOAD_FROM_MEM_FAIL;
				}
			}

			load_raw_block(pRawReplayBoardCtx->rawBlockFrameStartId,
					pRawReplayBoardCtx->rawBlockFrameEndId);

			// clean something that belongs to the load from file
			if (pRawReplayBoardCtx->rawFp != NULL) {
				fclose(pRawReplayBoardCtx->rawFp);
				pRawReplayBoardCtx->rawFp = NULL;
			}

			if (pRawReplayBoardCtx->le) {
				free(pRawReplayBoardCtx->le);
				pRawReplayBoardCtx->le = NULL;
			}

			if (pRawReplayBoardCtx->se) {
				free(pRawReplayBoardCtx->se);
				pRawReplayBoardCtx->se = NULL;
			}

			UT_PRT("load raw from memory success!\n");
		} else {
		LOAD_FROM_MEM_FAIL:
			UT_PRT("load raw from memory fail!\n");
			pRawReplayBoardCtx->isLoadRawFromMem = 0;
			if (pRawReplayBoardCtx->rawLeVir) {
				free(pRawReplayBoardCtx->rawLeVir);
				pRawReplayBoardCtx->rawLeVir = NULL;
			}
			if (pRawReplayBoardCtx->rawSeVir) {
				free(pRawReplayBoardCtx->rawSeVir);
				pRawReplayBoardCtx->rawSeVir = NULL;
			}
		}

	}
	UT_PRT("load raw from mem: %d\n", pRawReplayBoardCtx->isLoadRawFromMem);

	// pRawReplayBoardCtx Init
	pRawReplayBoardCtx->bRawReplayThreadEnabled = CVI_FALSE;
	pRawReplayBoardCtx->isRawReplayReady = CVI_FALSE;
	pRawReplayBoardCtx->timingAttr.bEnable = CVI_FALSE;
	pRawReplayBoardCtx->timingAttr.s32FrmRate = 25;

	// init vb pool
	pRawReplayBoardCtx->poolId = picInfo->poolId;

	pRawReplayBoardCtx->blk[0] = picInfo->usrBlk[0];
	pRawReplayBoardCtx->blk[1] = picInfo->usrBlk[1];

	pRawReplayBoardCtx->u64PhyAddr[0] = picInfo->usrPhyAddr[0];
	pRawReplayBoardCtx->u64PhyAddr[1] = picInfo->usrPhyAddr[1];

	pRawReplayBoardCtx->pu8VirAddr[0] = picInfo->usrVirAddr[0];
	pRawReplayBoardCtx->pu8VirAddr[1] = picInfo->usrVirAddr[1];

	offlineCtrlCmd.rawReplayMaxFrameId = pRawInfo->numFrame - 1;
	offlineCtrlCmd.rawReplayEndIdx = pRawInfo->numFrame - 1;
	offlineCtrlCmd.rawReplayReset = 1;

	return CVI_SUCCESS;
}

CVI_VOID raw_replay_offline_uninit(CVI_VOID)
{
	if (pRawReplayBoardCtx->pRawHeader != NULL) {
		free(pRawReplayBoardCtx->pRawHeader);
	}
	if (pRawReplayBoardCtx->leSe != NULL) {
		free(pRawReplayBoardCtx->leSe);
	}
	if (pRawReplayBoardCtx->le != NULL) {
		free(pRawReplayBoardCtx->le);
	}
	if (pRawReplayBoardCtx->se != NULL) {
		free(pRawReplayBoardCtx->se);
	}
	if (pRawReplayBoardCtx->rawLeVir != NULL) {
		free(pRawReplayBoardCtx->rawSeVir);
	}
	if (pRawReplayBoardCtx->rawSeVir != NULL) {
		free(pRawReplayBoardCtx->rawSeVir);
	}
	if (pRawReplayBoardCtx->rawFp != NULL) {
		fclose(pRawReplayBoardCtx->rawFp);
	}
	if (pRawReplayBoardCtx->le12Fd) {
		fclose(pRawReplayBoardCtx->le12Fd);
	}
	if (pRawReplayBoardCtx->se12Fd) {
		fclose(pRawReplayBoardCtx->se12Fd);
	}

	free(pRawReplayBoardCtx);
}

CVI_S32 start_raw_replay_offline(VI_PIPE viPipe)
{
	if (pRawReplayBoardCtx == NULL) {
		LOGOUT("pRawReplayBoardCtx == NULL\n");
		return CVI_FAILURE;
	}

	// TODO
	//CVI_ISP_AESetRawReplayMode(0, CVI_TRUE);

	if (!pRawReplayBoardCtx->bRawReplayThreadEnabled) {
		pRawReplayBoardCtx->viPipe = viPipe;
		pRawReplayBoardCtx->bRawReplayThreadEnabled = CVI_TRUE;

		if (pthread_create(&pRawReplayBoardCtx->rawReplayTid, NULL, raw_replay_offline_thread,
									pRawReplayBoardCtx->pRawHeader) != 0) {
			LOGOUT("pthread_create failed!\n");
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 stop_raw_replay_offline(CVI_VOID)
{
	//OTODO
	//CVI_ISP_AESetRawReplayMode(0, CVI_FALSE);

	if (pRawReplayBoardCtx->bRawReplayThreadEnabled) {

		pRawReplayBoardCtx->bRawReplayThreadEnabled = CVI_FALSE;

		pthread_join(pRawReplayBoardCtx->rawReplayTid, NULL);

		pRawReplayBoardCtx->rawReplayStatus = RAW_REPLAY_STOP;
	}

	system(CMD_RM RESET_PATH);
	system(CMD_RM SET_PATH);
	system(CMD_RM START_PATH);
	system(CMD_RM SAVE_PATH);
	system(CMD_RM YUV_DIR);

	return CVI_SUCCESS;
}

// load rawFileName from filePath
static CVI_S32 load_rawFileName_from_path(CVI_U8 *path, CVI_U8 *rawFileName, CVI_S32 maxFileNameLen)
{
	DIR *dirPtr = NULL;
	struct dirent *ptrDir = NULL;
	CVI_S32 fileNameLen = 0;

	dirPtr = opendir(path);
	if (dirPtr == NULL) {
		perror("Failed to open directory");
		return CVI_FAILURE;
	}
	while ((ptrDir = readdir(dirPtr)) != NULL) {
		if (strcmp(ptrDir->d_name, "..") == 0) {
			continue;
		}
		// Load file name from .raw
		if (ptrDir->d_type == DT_REG) {
			if (strstr(ptrDir->d_name, ".raw") != 0) {
				fileNameLen = strlen(ptrDir->d_name);
				if (fileNameLen > maxFileNameLen) {
					UT_PRT("file name is too long!\n");
					return CVI_FAILURE;
				}
				snprintf(rawFileName, fileNameLen - 3, "%s", ptrDir->d_name);
				break;
			}
		}
	}
	closedir(dirPtr);
	return CVI_SUCCESS;
}

static CVI_S32 Bayer_16bit_2_12bit(CVI_U16 *buffer16bit, CVI_U8 *le, CVI_U8 *se,
						CVI_U16 width, CVI_U16 height)
{
	CVI_U32 rowIdx, colIdx;
	CVI_U16 pixel1, pixel2;
	CVI_U8 *ptrLe, *ptrSe;
	CVI_U16 frameWidth = width;

	if (se) {
		frameWidth = 2 * width;
	} else {
	}

	if (buffer16bit == NULL || le == NULL) {
		LOGOUT("pointer is NULL\n");
		return CVI_FAILURE;
	}

	ptrLe = le;
	ptrSe = se;

	for (rowIdx = 0; rowIdx < height; rowIdx++) {
		for (colIdx = 0; colIdx < width; colIdx += 2) {
			pixel1 = *buffer16bit;
			pixel2 = *(buffer16bit + 1);

			*ptrLe = (pixel1 >> 4) & 0xFF;
			*(ptrLe + 1) = (pixel2 >> 4) & 0xFF;
			*(ptrLe + 2) = (pixel1 & 0x0F) | ((pixel2 & 0x0F) << 4);
			buffer16bit += 2;
			ptrLe += 3;
		}

		for (colIdx = width; colIdx < frameWidth; colIdx += 2) {
			pixel1 = *buffer16bit;
			pixel2 = *(buffer16bit + 1);

			*ptrSe = (pixel1 >> 4) & 0xFF;
			*(ptrSe + 1) = (pixel2 >> 4) & 0xFF;
			*(ptrSe + 2) = (pixel1 & 0x0F) | ((pixel2 & 0x0F) << 4);
			buffer16bit += 2;
			ptrSe += 3;
		}
	}

	return CVI_SUCCESS;
}

// 12 bit
static CVI_S32 load_one_frame_data_from_mem(RAW_REPLAY_INFO *pRawInfo, CVI_S32 curFrame)
{
	static CVI_S32 preFrame = -1;

	CVI_S32 direction = curFrame - preFrame;

	if (preFrame == curFrame) {
		return CVI_SUCCESS;
	}
	preFrame = curFrame;

	if (curFrame >= pRawReplayBoardCtx->rawBlockFrameStartId &&
		curFrame <= pRawReplayBoardCtx->rawBlockFrameEndId) {
		;
	} else {
		CVI_S32 endFrame = 0;

		if (direction > 0) {
			endFrame = curFrame + pRawReplayBoardCtx->blockMaxFrame;
		} else {
			endFrame = curFrame - pRawReplayBoardCtx->blockMaxFrame;
		}

		if (endFrame < 0) {
			endFrame = 0;
		}

		if (endFrame > pRawInfo->numFrame - 1) {
			endFrame = pRawInfo->numFrame - 1;
		}

		if (endFrame > curFrame) {
			pRawReplayBoardCtx->rawBlockFrameStartId = curFrame;
			pRawReplayBoardCtx->rawBlockFrameEndId = endFrame;
		} else {
			pRawReplayBoardCtx->rawBlockFrameStartId = endFrame;
			pRawReplayBoardCtx->rawBlockFrameEndId = curFrame;
		}

		load_raw_block(pRawReplayBoardCtx->rawBlockFrameStartId,
				pRawReplayBoardCtx->rawBlockFrameEndId);
	}

	long frameOffset = curFrame - pRawReplayBoardCtx->rawBlockFrameStartId;

	long offsetBytes = frameOffset * pRawReplayBoardCtx->sz12b;

	CVI_U8 *ptrLe = NULL;
	CVI_U8 *ptrSe = NULL;

	ptrLe = pRawReplayBoardCtx->rawLeVir + offsetBytes;

	if (pRawReplayBoardCtx->rawSeVir) {
		ptrSe = pRawReplayBoardCtx->rawSeVir + offsetBytes;
	}

	if (curFrame < pRawInfo->numFrame) {
		memcpy(pRawReplayBoardCtx->pu8VirAddr[0], ptrLe, pRawReplayBoardCtx->sz12b);
		if (pRawInfo->enWDR) {
			memcpy(pRawReplayBoardCtx->pu8VirAddr[1], ptrSe, pRawReplayBoardCtx->sz12b);
		}
	}

	return CVI_SUCCESS;

}

static CVI_S32 load_one_frame_data_from_file(FILE *fp, RAW_REPLAY_INFO *pRawInfo, CVI_S32 curFrame)
{
	static CVI_S32 preFrame = -1;

	if (preFrame == curFrame) {
		return CVI_SUCCESS;
	}
	preFrame = curFrame;

	long offsetBytes = (long)curFrame * pRawReplayBoardCtx->mSz16b;
	CVI_U64 frameRead;

	fseek(fp, offsetBytes, SEEK_SET);

	// Read and Transform 16bit Data to 12bit
	frameRead = fread(pRawReplayBoardCtx->leSe, 1, pRawReplayBoardCtx->mSz16b, fp);
	if (frameRead < (CVI_U64) pRawReplayBoardCtx->mSz16b) {
		// return failure if rearch the end of file
		if (feof(fp)) {
			LOGOUT("End of file reached prematurely after %d frames[%lu/%u].\n",
						curFrame+1, frameRead, pRawReplayBoardCtx->mSz16b);
		} else {
			LOGOUT("File read error after %d frames[%lu/%u].\n", curFrame+1,
						frameRead, pRawReplayBoardCtx->mSz16b);
		}
		return CVI_FAILURE;
	}

	Bayer_16bit_2_12bit(pRawReplayBoardCtx->leSe, pRawReplayBoardCtx->le,
							pRawReplayBoardCtx->se, pRawInfo->width, pRawInfo->height);
	return CVI_SUCCESS;
}

static CVI_U8 *get_str_value(const CVI_U8 *src, const CVI_U8 *str)
{
	CVI_S32 idx = 0;
	CVI_U8 *temp = NULL;
	static CVI_U8 value[PATH_MAX_LEN] = {0};

	temp = strstr(src, str);
	if (temp == NULL) {
		goto FAIL;
	}
	temp = strchr(temp, '=');
	if (temp == NULL) {
		goto FAIL;
	}
	temp = strchr(temp, ' ');
	if (temp == NULL) {
		goto FAIL;
	}
	temp++;

	memset(value, '\0', PATH_MAX_LEN);
	for (idx = 0; idx < PATH_MAX_LEN; idx++) {
		value[idx] = *temp;
		temp++;
		if (*temp == '\r' || *temp == '\n') {
			break;
		}
	}
	return value;

FAIL:
	LOGOUT("get %s value fail!\n", str);
	return NULL;
}

// load raw info from TXT file
static CVI_S32 load_raw_info_from_file(CVI_U8 *boardPath, CVI_U8 *rawFileName, RAW_REPLAY_INFO *pRawInfo)
{
	FILE *fp = NULL;
	CVI_U8 *src = NULL;
	CVI_U8 *value = NULL;
	struct stat statBuf;
	CVI_U8 rawInfoFile[PATH_MAX_LEN] = {0};
	CVI_U8 bayerFormat[20], wdrFormat[20];
	CVI_U8 *bayerAndWdr = NULL;
	OFFLINE_3A_ALGO_RET *offline3aAlgo = &pRawReplayBoardCtx->offline3aAlgoRet;

	LOGOUT("raw info:\n");
	// Load Raw Info from RawFile Name
	// e.g:2560X1440_GRBG_Linear_-color=2_-bits=12_-frame=15_-hdr=1_ISO=100_20241029101706
	if (sscanf(rawFileName, "%dX%d", &pRawInfo->width, &pRawInfo->height) != 2) {
		LOGOUT("Failed to load Raw W*H.\n");
		goto FAIL;
	}
	bayerAndWdr = strchr(rawFileName, '_') + 1;
	if (sscanf(bayerAndWdr, "%16[^_]_%16[^_-]", bayerFormat, wdrFormat) != 2) {
		LOGOUT("Failed to load Raw Format.\n");
		goto FAIL;
	}
	if (strcmp(bayerFormat, "BGGR") == 0) {
		pRawInfo->bayerID = 0;
	} else if (strcmp(bayerFormat, "GBRG") == 0) {
		pRawInfo->bayerID = 1;
	} else if (strcmp(bayerFormat, "GRBG") == 0) {
		pRawInfo->bayerID = 2;
	} else if (strcmp(bayerFormat, "RGGB") == 0) {
		pRawInfo->bayerID = 3;
	} else {
		LOGOUT("bayerFormat(%s) is wrong! Please recheck.\n", bayerFormat);
		goto FAIL;
	}
	if (strcmp(wdrFormat, "Linear") == 0) {
		pRawInfo->enWDR = 0;
	} else if (strcmp(wdrFormat, "WDR") == 0) {
		pRawInfo->enWDR = 1;
	} else {
		LOGOUT("wdrFormat(%s) is wrong! Please recheck.\n", wdrFormat);
		goto FAIL;
	}
	sscanf(strstr(rawFileName, "-frame="), "-frame=%d", &pRawInfo->numFrame);

	// deal the wdr
	if (pRawInfo->enWDR) {
		pRawInfo->width /= 2;
	}

	pRawInfo->curFrame = 0;
	pRawInfo->pixFormat = 0;		// PixelFormat: 0 raw, !0 yuv(22 yuyv422,...)
	pRawReplayBoardCtx->sz16b = (CVI_U32) pRawInfo->width * pRawInfo->height * 2;		// 16bit RawSize
	pRawReplayBoardCtx->sz12b = (CVI_U32) pRawInfo->width * pRawInfo->height * 1.5;	// 12bit RawSize

	if (pRawInfo->enWDR) {
		pRawReplayBoardCtx->mSz16b = 2 * pRawReplayBoardCtx->sz16b;
	} else {
		pRawReplayBoardCtx->mSz16b = pRawReplayBoardCtx->sz16b;
	}

	// Load Raw Info from TXT File
	snprintf(rawInfoFile, PATH_MAX_LEN, "%s/%s.txt", boardPath, rawFileName);
	fp = fopen(rawInfoFile, "rb");
	if (fp == NULL) {
		LOGOUT("fopen(%s) failed!\n", rawInfoFile);
		goto FAIL;
	}

	stat(rawInfoFile, &statBuf);
	src = (CVI_U8 *) calloc(statBuf.st_size + 1, 1);
	if (src == NULL) {
		LOGOUT("src == NULL\n");
		goto FAIL;
	}
	fread(src, statBuf.st_size, 1, fp);
	fclose(fp);
	fp = NULL;

	value = get_str_value(src, "ISO");
	if (value != NULL) {
		pRawInfo->ISO = atoi(value);
		offline3aAlgo->iso = atoi(value);
		LOGOUT("ISO = %d\n", pRawInfo->ISO);
	}

	value = get_str_value(src, "Light Value");
	if (value != NULL) {
		pRawInfo->lightValue = atof(value);
		offline3aAlgo->lv = atof(value);
		LOGOUT("Light value = %f\n", pRawInfo->lightValue);
	}

	value = get_str_value(src, "Color Temp.");
	if (value != NULL) {
		pRawInfo->colorTemp = atoi(value);
		offline3aAlgo->colorTmp = atoi(value);
		LOGOUT("Color Temp. = %d\n", pRawInfo->colorTemp);
	}

	value = get_str_value(src, "ISP DGain");
	if (value != NULL) {
		pRawInfo->ispDGain = atoi(value);
		offline3aAlgo->ispDgain = atoi(value);
		LOGOUT("ISP DGain = %d\n", pRawInfo->ispDGain);
	}

	value = get_str_value(src, "Exposure Time");
	if (value != NULL) {
		pRawInfo->longExposure = atoi(value);
		offline3aAlgo->leExpTime = atoi(value);
		LOGOUT("Exposure Time = %d\n", pRawInfo->longExposure);
	}

	value = get_str_value(src, "Short Exposure");
	if (value != NULL) {
		pRawInfo->shortExposure = atoi(value);
		offline3aAlgo->seExpTime = atoi(value);
		LOGOUT("Short Exposure = %d\n", pRawInfo->shortExposure);
	}

	value = get_str_value(src, "Exposure Ratio");
	if (value != NULL) {
		pRawInfo->exposureRatio = atoi(value);
		offline3aAlgo->expRatio = atoi(value);
		LOGOUT("Exposure Ratio = %d\n", pRawInfo->exposureRatio);
	}

	value = get_str_value(src, "Exposure AGain");
	if (value != NULL) {
		pRawInfo->exposureAGain = atoi(value);
		offline3aAlgo->expAgain = atoi(value);
		LOGOUT("Exposure AGain = %d\n", pRawInfo->exposureAGain);
	}

	value = get_str_value(src, "Exposure DGain");
	if (value != NULL) {
		pRawInfo->exposureDGain = atoi(value);
		offline3aAlgo->expDgain = atoi(value);
		LOGOUT("Exposure DGain = %d\n", pRawInfo->exposureDGain);
	}

	value = get_str_value(src, "reg_wbg_rgain");
	if (value != NULL) {
		pRawInfo->WB_RGain = atoi(value);
		offline3aAlgo->wbgRgain = atoi(value);
		LOGOUT("reg_wbg_rgain = %d\n", pRawInfo->WB_RGain);
	}

	value = get_str_value(src, "reg_wbg_bgain");
	if (value != NULL) {
		pRawInfo->WB_BGain = atoi(value);
		offline3aAlgo->wbgBgain = atoi(value);
		LOGOUT("reg_wbg_bgain = %d\n", pRawInfo->WB_BGain);
	}

	value = get_str_value(src, "reg_wbg_grgain");
	if (value != NULL) {
		pRawInfo->WB_GGain = atoi(value);
		offline3aAlgo->wbgGgain = atoi(value);
		LOGOUT("reg_wbg_ggain = %d\n", pRawInfo->WB_GGain);
	}

	value = get_str_value(src, "reg_ccm_00");
	if (value != NULL) {
		pRawInfo->CCM[0] = atoi(value);
		offline3aAlgo->ccm[0] = atoi(value);
	}
	value = get_str_value(src, "reg_ccm_01");
	if (value != NULL) {
		pRawInfo->CCM[1] = atoi(value);
		offline3aAlgo->ccm[1] = atoi(value);
	}
	value = get_str_value(src, "reg_ccm_02");
	if (value != NULL) {
		pRawInfo->CCM[2] = atoi(value);
		offline3aAlgo->ccm[2] = atoi(value);
	}
	value = get_str_value(src, "reg_ccm_10");
	if (value != NULL) {
		pRawInfo->CCM[3] = atoi(value);
		offline3aAlgo->ccm[3] = atoi(value);
	}
	value = get_str_value(src, "reg_ccm_11");
	if (value != NULL) {
		pRawInfo->CCM[4] = atoi(value);
		offline3aAlgo->ccm[4] = atoi(value);
	}
	value = get_str_value(src, "reg_ccm_12");
	if (value != NULL) {
		pRawInfo->CCM[5] = atoi(value);
		offline3aAlgo->ccm[5] = atoi(value);
	}
	value = get_str_value(src, "reg_ccm_20");
	if (value != NULL) {
		pRawInfo->CCM[6] = atoi(value);
		offline3aAlgo->ccm[6] = atoi(value);
	}
	value = get_str_value(src, "reg_ccm_21");
	if (value != NULL) {
		pRawInfo->CCM[7] = atoi(value);
		offline3aAlgo->ccm[7] = atoi(value);
	}
	value = get_str_value(src, "reg_ccm_22");
	if (value != NULL) {
		pRawInfo->CCM[8] = atoi(value);
		offline3aAlgo->ccm[8] = atoi(value);
		LOGOUT("reg_ccm = \t%d, %d, %d\n\t\t%d, %d, %d\n\t\t%d, %d, %d\n",
						pRawInfo->CCM[0], pRawInfo->CCM[1], pRawInfo->CCM[2],
						pRawInfo->CCM[3], pRawInfo->CCM[4], pRawInfo->CCM[5],
						pRawInfo->CCM[6], pRawInfo->CCM[7], pRawInfo->CCM[8]);
	}

	value = get_str_value(src, "reg_blc_offset_r");
	if (value != NULL) {
		pRawInfo->BLC_Offset[0] = atoi(value);
		offline3aAlgo->blcOffsetR = atoi(value);
	}
	value = get_str_value(src, "reg_blc_offset_gr");
	if (value != NULL) {
		pRawInfo->BLC_Offset[1] = atoi(value);
		offline3aAlgo->blcOffsetGr = atoi(value);
	}
	value = get_str_value(src, "reg_blc_offset_gb");
	if (value != NULL) {
		pRawInfo->BLC_Offset[2] = atoi(value);
		offline3aAlgo->blcOffsetGb = atoi(value);
	}
	value = get_str_value(src, "reg_blc_offset_b");
	if (value != NULL) {
		pRawInfo->BLC_Offset[3] = atoi(value);
		offline3aAlgo->blcOffsetB = atoi(value);
		LOGOUT("reg_blc_offset = %d, %d, %d, %d\n",
						pRawInfo->BLC_Offset[0], pRawInfo->BLC_Offset[1],
						pRawInfo->BLC_Offset[2], pRawInfo->BLC_Offset[3]);
	}

	value = get_str_value(src, "reg_blc_gain_r");
	if (value != NULL) {
		pRawInfo->BLC_Gain[0] = atoi(value);
		offline3aAlgo->blcGainR = atoi(value);
	}
	value = get_str_value(src, "reg_blc_gain_gr");
	if (value != NULL) {
		pRawInfo->BLC_Gain[1] = atoi(value);
		offline3aAlgo->blcGainGr = atoi(value);
	}
	value = get_str_value(src, "reg_blc_gain_gb");
	if (value != NULL) {
		pRawInfo->BLC_Gain[2] = atoi(value);
		offline3aAlgo->blcGainGb = atoi(value);
	}
	value = get_str_value(src, "reg_blc_gain_b");
	if (value != NULL) {
		pRawInfo->BLC_Gain[3] = atoi(value);
		offline3aAlgo->blcGainB = atoi(value);
		LOGOUT("reg_blc_gain = %d, %d, %d, %d\n",
						pRawInfo->BLC_Gain[0], pRawInfo->BLC_Gain[1],
						pRawInfo->BLC_Gain[2], pRawInfo->BLC_Gain[3]);
	}

	free(src);
	src = NULL;

	return CVI_SUCCESS;

FAIL:
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
	if (src != NULL) {
		free(src);
		src = NULL;
	}

	return CVI_FAILURE;
}

// put one frame raw data into vbpool
static CVI_S32 set_1_frame_raw_data_into_vbpool(RAW_REPLAY_INFO *pRawInfo, CVI_S32 curFrame)
{
	if (curFrame < pRawInfo->numFrame) {
		memcpy(pRawReplayBoardCtx->pu8VirAddr[0], pRawReplayBoardCtx->le, pRawReplayBoardCtx->sz12b);

		if (pRawReplayBoardCtx->se) {
			memcpy(pRawReplayBoardCtx->pu8VirAddr[1], pRawReplayBoardCtx->se, pRawReplayBoardCtx->sz12b);
		}
	} else {
		LOGOUT("curFrame[%u] >= TotalFrame[%u]!\n", curFrame, pRawInfo->numFrame);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

static CVI_VOID get_current_awb_info(ISP_MWB_ATTR_S *pstMwbAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ISP_WB_INFO_S stWBInfo;

	memset(&stWBInfo, 0, sizeof(ISP_WB_INFO_S));
	s32Ret = CVI_ISP_QueryWBInfo(pRawReplayBoardCtx->viPipe, &stWBInfo);
	if (s32Ret != CVI_SUCCESS) {
		LOGOUT("Error: CVI_ISP_QueryWBInfo Failed!\n");
	}

	pstMwbAttr->u16Rgain = stWBInfo.u16Rgain;
	pstMwbAttr->u16Grgain = stWBInfo.u16Grgain;
	pstMwbAttr->u16Gbgain = stWBInfo.u16Gbgain;
	pstMwbAttr->u16Bgain = stWBInfo.u16Bgain;
}

static CVI_VOID update_awb_config(const ISP_MWB_ATTR_S *pstMwbAttr, ISP_OP_TYPE_E eType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ISP_WB_ATTR_S stWbAttr;

	if (DISABLE_AWB_UPDATE_CTRL != 0) {
		return;
	}

	memset(&stWbAttr, 0, sizeof(ISP_WB_ATTR_S));
	s32Ret = CVI_ISP_GetWBAttr(pRawReplayBoardCtx->viPipe, &stWbAttr);
	if (s32Ret != CVI_SUCCESS) {
		LOGOUT("Error: CVI_ISP_GetWBAttr Failed!\n");
	}

	if (eType == OP_TYPE_MANUAL) {
		stWbAttr.enOpType = OP_TYPE_MANUAL;
		stWbAttr.stManual = *pstMwbAttr;
	} else {
		stWbAttr.enOpType = OP_TYPE_AUTO;
	}

	s32Ret = CVI_ISP_SetWBAttr(pRawReplayBoardCtx->viPipe, &stWbAttr);
	if (s32Ret != CVI_SUCCESS) {
		LOGOUT("Error: CVI_ISP_SetWBAttr Failed!\n");
	}
}

static CVI_VOID get_current_ae_info(ISP_EXP_INFO_S *pstExpInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UT_PRT("get current ae info\n");

	s32Ret = CVI_ISP_QueryExposureInfo(pRawReplayBoardCtx->viPipe, pstExpInfo);
	if (s32Ret != CVI_SUCCESS) {
		LOGOUT("Error: CVI_ISP_QueryExposureInfo Failed!\n");
	}
}

static CVI_VOID update_ae_config(const ISP_EXP_INFO_S *pstExpInfo, ISP_OP_TYPE_E eType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	ISP_EXPOSURE_ATTR_S stAEAttr;
	ISP_WDR_EXPOSURE_ATTR_S stWdrExpAttr;

	memset(&stAEAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));
	memset(&stWdrExpAttr, 0, sizeof(ISP_WDR_EXPOSURE_ATTR_S));
	s32Ret = CVI_ISP_GetExposureAttr(pRawReplayBoardCtx->viPipe, &stAEAttr);
	if (s32Ret != CVI_SUCCESS) {
		LOGOUT("Error: CVI_ISP_GetExposureAttr Failed!\n");
	}
	s32Ret = CVI_ISP_GetWDRExposureAttr(pRawReplayBoardCtx->viPipe, &stWdrExpAttr);
	if (s32Ret != CVI_SUCCESS) {
		LOGOUT("Error: CVI_ISP_GetWDRExposureAttr Failed!\n");
	}

	if (eType == OP_TYPE_MANUAL) {
		stAEAttr.enOpType = OP_TYPE_MANUAL;

		stAEAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
		stAEAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
		stAEAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
		stAEAttr.stManual.enISPDGainOpType = OP_TYPE_MANUAL;
		stAEAttr.stManual.enGainType = AE_TYPE_GAIN;

		stAEAttr.stManual.u32ExpTime = pstExpInfo->u32ExpTime;
		stAEAttr.stManual.u32AGain = pstExpInfo->u32AGain;
		stAEAttr.stManual.u32DGain = pstExpInfo->u32DGain;
		stAEAttr.stManual.u32ISPDGain = pstExpInfo->u32ISPDGain;

		if (pRawReplayBoardCtx->pRawHeader->numFrame == 1) {
			stWdrExpAttr.enExpRatioType = OP_TYPE_MANUAL;

			for (CVI_U32 i = 0; i < WDR_EXP_RATIO_NUM; i++) {
				stWdrExpAttr.au32ExpRatio[i] = pstExpInfo->u32WDRExpRatio;
			}
		}

		memcpy((CVI_U8 *)&stAEAttr.stAuto.au32Reserve[0], (CVI_U8 *)&pstExpInfo->fLightValue,
								sizeof(CVI_FLOAT));
	} else {
		stAEAttr.enOpType = OP_TYPE_AUTO;

		stAEAttr.stManual.enExpTimeOpType = OP_TYPE_AUTO;
		stAEAttr.stManual.enGainType = AE_TYPE_GAIN;
		stAEAttr.stManual.enISONumOpType = OP_TYPE_AUTO;

		stWdrExpAttr.enExpRatioType = OP_TYPE_AUTO;
	}

	s32Ret = CVI_ISP_SetExposureAttr(pRawReplayBoardCtx->viPipe, &stAEAttr);
	if (s32Ret != CVI_SUCCESS) {
		LOGOUT("Error: CVI_ISP_SetExposureAttr Failed!\n");
	}
	s32Ret = CVI_ISP_SetWDRExposureAttr(pRawReplayBoardCtx->viPipe, &stWdrExpAttr);
	if (s32Ret != CVI_SUCCESS) {
		LOGOUT("Error: CVI_ISP_SetWDRExposureAttr Failed!\n");
	}

	if (eType == OP_TYPE_MANUAL)
		CVI_ISP_AESetRawReplayExposure(pRawReplayBoardCtx->viPipe, pstExpInfo);
}

static CVI_VOID apply_raw_info(ISP_MWB_ATTR_S *pstMwbAttr, ISP_EXP_INFO_S *pstExpInfo, RAW_REPLAY_INFO *pRawInfo)
{
	pstMwbAttr->u16Rgain = pRawInfo->WB_RGain;
	pstMwbAttr->u16Grgain = pstMwbAttr->u16Gbgain = pRawInfo->WB_GGain;
	pstMwbAttr->u16Bgain = pRawInfo->WB_BGain;

	pstExpInfo->u32ShortExpTime = pRawInfo->shortExposure;
	pstExpInfo->u32ExpTime = pRawInfo->longExposure;
	pstExpInfo->u32AGain = pRawInfo->exposureAGain;
	pstExpInfo->u32DGain = pRawInfo->exposureDGain;
	pstExpInfo->u32ISPDGain = pRawInfo->ispDGain;

	pstExpInfo->u32ISO = pRawInfo->ISO;
	pstExpInfo->fLightValue = pRawInfo->lightValue;

	pstExpInfo->u32WDRExpRatio = pRawInfo->exposureRatio;
}

CVI_BOOL is_raw_replay_offline_ready(CVI_VOID)
{
	if (pRawReplayBoardCtx != NULL) {
		return pRawReplayBoardCtx->isRawReplayReady;
	} else {
		return CVI_FALSE;
	}
}

// Update Frame by RawInfo (Size\Addr\Format\...)
static CVI_VOID update_video_frame(VIDEO_FRAME_INFO_S *stVideoFrame, RAW_REPLAY_INFO *pRawInfo)
{
	//LOGOUT("wdrmode: %d, width: %d, height: %d\n", mode, pRawInfo->width, pRawInfo->height);

	stVideoFrame->stVFrame.u32Width = pRawInfo->width;
	stVideoFrame->stVFrame.u32Height = pRawInfo->height;

	stVideoFrame->stVFrame.s16OffsetLeft = stVideoFrame->stVFrame.s16OffsetTop =
		stVideoFrame->stVFrame.s16OffsetRight = stVideoFrame->stVFrame.s16OffsetBottom = 0;

	stVideoFrame->stVFrame.enBayerFormat = (BAYER_FORMAT_E) pRawInfo->bayerID;
	stVideoFrame->stVFrame.enPixelFormat = (PIXEL_FORMAT_E) 0;
	stVideoFrame->stVFrame.u64PhyAddr[0] = pRawReplayBoardCtx->u64PhyAddr[0];
	stVideoFrame->stVFrame.u64PhyAddr[1] = pRawReplayBoardCtx->u64PhyAddr[1];

	if (pRawInfo->enWDR) {
		stVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_HDR10;
	} else {
		stVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;
	}
}

static CVI_VOID send_replay_frame_2_vi(CVI_U32 curFrame, ISP_MWB_ATTR_S *pstMwbAttr,
						ISP_EXP_INFO_S *pstExpInfo, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE pipeId[] = {0};
	const VIDEO_FRAME_INFO_S *pstVideoFrameInfo[1];
	static CVI_U32 loopNum;

	pstVideoFrameInfo[0] = pstVideoFrame;

	if (!pRawReplayBoardCtx->pRawHeader->pixFormat) {
		apply_raw_info(pstMwbAttr, pstExpInfo, pRawReplayBoardCtx->pRawHeader);
		//update_awb_config(pstMwbAttr, OP_TYPE_MANUAL);
		//update_ae_config(pstExpInfo, OP_TYPE_MANUAL);
		// TODO
		UNUSED(update_awb_config);
		UNUSED(update_ae_config);
	}

	pstVideoFrame->stVFrame.u64PhyAddr[0] = pRawReplayBoardCtx->u64PhyAddr[0];
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pRawReplayBoardCtx->u64PhyAddr[1];

	s32Ret = CVI_VI_SendPipeRaw(1, pipeId, pstVideoFrameInfo, 80);
	if (s32Ret != CVI_SUCCESS) {
		LOGOUT("Error: CVI_VI_SendPipeRaw Failed!\n");
	}
	//
	UT_PRT("raw replay: [%u] [%u] waiting ISP_VD_BE_END...\n", frameCnt++, curFrame);

	while (pRawReplayBoardCtx->bRawReplayThreadEnabled) {
#ifdef CONFIG_DUAL_OS
		s32Ret = CVI_ISP_GetVDTimeOut(0, ISP_VD_BE_END, 1 * 1000);
		UNUSED(waiting_isp_mw_post_eof);
#else
		s32Ret = waiting_isp_mw_post_eof(2 * 1000);
#endif
		if (s32Ret == 0) {
			break;
		}
	}

	if (curFrame == 0) {
		loopNum++;
	}

	if (isReset != 1 && offlineCtrlCmd.yuvSaveIdx[curFrame] == 1) {
		s32Ret = isp_get_chn_frame(0, curFrame, loopNum);
	}

}

static CVI_VOID *raw_replay_offline_thread(CVI_VOID *arg)
{
	CVI_S32 retVal = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S videoFrame;
	ISP_MWB_ATTR_S mwbAttr;
	ISP_EXP_INFO_S expInfo;
	VI_DEV_TIMING_ATTR_S timingAttr;
	RAW_REPLAY_INFO *pRawInfo = (RAW_REPLAY_INFO *)arg;
	struct timeval timeSend;
	CVI_U32 frameTime, frameRate;
	CVI_S32 curFrame = 0, step = 1;
	struct timeval timeStart;
	struct timeval timeEnd;

	// Get AE/AWB Info
	memset(&mwbAttr, 0, sizeof(ISP_MWB_ATTR_S));
	memset(&expInfo, 0, sizeof(ISP_EXP_INFO_S));
	get_current_awb_info(&mwbAttr);
	get_current_ae_info(&expInfo);
	LOGOUT("wbRGain:%d,wbGGain:%d,wbBGain:%d,exptime:%d,iso:%d,expratio:%d,LV:%f\n",
				pRawInfo->WB_RGain, pRawInfo->WB_GGain, pRawInfo->WB_BGain, pRawInfo->longExposure,
				pRawInfo->ISO, pRawInfo->exposureRatio, pRawInfo->lightValue);

	retVal = CVI_VI_GetDevTimingAttr(0, &timingAttr);
	if (retVal == CVI_SUCCESS) {
		pRawReplayBoardCtx->timingAttr.bEnable = timingAttr.bEnable;
		pRawReplayBoardCtx->timingAttr.s32FrmRate = timingAttr.s32FrmRate;
	}
	LOGOUT("\n\nstart raw replay, mode %d (0 manu, 1 auto), framerate %d...\n\n",
		pRawReplayBoardCtx->timingAttr.bEnable, pRawReplayBoardCtx->timingAttr.s32FrmRate);

#ifndef CONFIG_DUAL_OS
	ISP_ALGO_RESULT_S *pAlgoRet = NULL;

	isp_feature_ctrl_get_algo_ret_addr(0, &pAlgoRet);

	if (pAlgoRet == NULL) {
		UT_PRT("fail to get the isp's algo result's address! Stop Raw replay\n");
	}

	UT_PRT("update 3a algo ret\n");
	update_3a_algo(pAlgoRet);
#else
	CVI_ISP_Set3aRet(0, &(pRawReplayBoardCtx->offline3aAlgoRet));
	UNUSED(update_3a_algo);
#endif

	while (pRawReplayBoardCtx->bRawReplayThreadEnabled) {
		gettimeofday(&timeStart, NULL);
		if (!offlineCtrlCmd.rawReplayStart && access(START_PATH, F_OK) != 0) {
			UT_PRT("waiting for the raw replay start!\n");
			usleep(3 * 1000 * 1000);
			continue;
		}

		if (offlineCtrlCmd.rawReplayStop) {
			offlineCtrlCmd.rawReplayStop = 0;
			offlineCtrlCmd.rawReplayStart = 0;
			if (access(START_PATH, F_OK) == 0) {
				system(CMD_RM START_PATH);
			}
			UT_PRT("stop the raw replay run\n");
			continue;
		}

		if (offlineCtrlCmd.rawReplayReset) {
			offlineCtrlCmd.rawReplayReset = 0;
			curFrame = offlineCtrlCmd.rawReplayStartIdx;
			isReset = 1;
			UT_PRT("reset the raw replay run, cur frame: %d\n", curFrame);
		}

		if (isReset && curFrame > 3 + offlineCtrlCmd.rawReplayStartIdx) {
			isReset = 0;
			curFrame = offlineCtrlCmd.rawReplayStartIdx;
		}

		frameRate = pRawReplayBoardCtx->timingAttr.s32FrmRate;

		//Reload one Frame Data from Raw File
		pRawReplayBoardCtx->rawReplayStatus = RAW_REPLAY_RELOAD;
		if (pRawReplayBoardCtx->isLoadRawFromMem) {
			load_one_frame_data_from_mem(pRawReplayBoardCtx->pRawHeader, curFrame);
		} else {
			load_one_frame_data_from_file(pRawReplayBoardCtx->rawFp,
							pRawReplayBoardCtx->pRawHeader, curFrame);
			set_1_frame_raw_data_into_vbpool(pRawReplayBoardCtx->pRawHeader, curFrame);
		}

		// Update one Frame Data to VB
		pRawReplayBoardCtx->rawReplayStatus = RAW_REPLAY_UPDATE;
		update_video_frame(&videoFrame, pRawInfo);
		// check this !!!
		if (offlineCtrlCmd.rawReplayRunMode == 0) {
			if (offlineCtrlCmd.rawReplayReset) {
				videoFrame.stVFrame.u32TimeRef = (curFrame - offlineCtrlCmd.rawReplayStartIdx) + 2;
			} else {
				videoFrame.stVFrame.u32TimeRef = (curFrame - offlineCtrlCmd.rawReplayStartIdx) + 1;
			}
		} else {
			videoFrame.stVFrame.u32TimeRef = (curFrame - offlineCtrlCmd.rawReplayStartIdx) + 1;
		}
		pRawReplayBoardCtx->isRawReplayReady = CVI_TRUE;
		send_replay_frame_2_vi(curFrame, &mwbAttr, &expInfo, &videoFrame);
		pRawReplayBoardCtx->isRawReplayReady = CVI_FALSE;

		// Refresh one Frame per frameRate
		pRawReplayBoardCtx->rawReplayStatus = RAW_REPLAY_SLEEP;
		gettimeofday(&timeSend, NULL);

		UNUSED(frameRate);
		UNUSED(frameTime);

		// is stop wait
		if (offlineCtrlCmd.stopWaitIdx[curFrame]) {
			if (offlineCtrlCmd.rawReplayWaitContinue) {
				offlineCtrlCmd.rawReplayWaitContinue = 0;
			} else {
				continue;
			}
		}

		// print the duration information
		gettimeofday(&timeEnd, NULL);

		UT_PRT(">> cur frame[%d], duration: %.2fs\n", curFrame, (timeEnd.tv_sec - timeStart.tv_sec)
						+ (timeEnd.tv_usec - timeStart.tv_usec) / (1.0 * 1000000));

		curFrame += step;

		if (curFrame > offlineCtrlCmd.rawReplayEndIdx) {
			if (offlineCtrlCmd.rawReplayRunMode == 0) {
				curFrame = offlineCtrlCmd.rawReplayEndIdx - 1;
				step = -1;
			} else {
				curFrame = offlineCtrlCmd.rawReplayStartIdx;
				step = 1;
			}
		} else if (curFrame < offlineCtrlCmd.rawReplayStartIdx) {
			if (offlineCtrlCmd.rawReplayRunMode == 0) {
				curFrame = offlineCtrlCmd.rawReplayStartIdx + 1;
				step = 1;
			} else {
				curFrame = offlineCtrlCmd.rawReplayStartIdx;
				step = 1;
			}
		}
	}
	LOGOUT("/*** raw replay therad end ***/\n");
	return NULL;
}

static CVI_S32 isp_get_chn_frame(CVI_U8 chn, CVI_U32 curFrame, CVI_U32 loopNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));

	s32Ret = CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000);
	if (s32Ret == CVI_SUCCESS) {
		CVI_CHAR imgName[128] = {0, };
		struct timeval tv;

		gettimeofday(&tv, NULL);

		// read the save dir
		CVI_U8 yuvSaveDir[64] = {0};

		if (access(YUV_DIR, F_OK) == 0) {
			FILE *fd = fopen(YUV_DIR, "r");
			if (fd != NULL) {
				fscanf(fd, "%s", yuvSaveDir);
				fclose(fd);
			}
		} else {
			snprintf(yuvSaveDir, 64, "%s", "yuv");
		}

		if (access(yuvSaveDir, F_OK) != 0) {
			CVI_U8 cmd[128] = {0};

			snprintf(cmd, 128, "mkdir %s", yuvSaveDir);
			system(cmd);
		}

		snprintf(imgName, sizeof(imgName), "%s/frame_%u_%u_%ld_%ld.yuv",
				yuvSaveDir,
				curFrame, loopNum,
				tv.tv_sec, tv.tv_usec);

		vi_ut_save_frame2file(imgName, &stVideoFrame);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			UT_PRT("CVI_VI_ReleaseChnFrame NG\n");

		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 update_3a_algo(ISP_ALGO_RESULT_S *algoRet)
{
	CVI_S32 ret = 0;

	algoRet->u32ColorTemp = pRawReplayBoardCtx->offline3aAlgoRet.colorTmp;

	algoRet->u32PreNpIso =
	algoRet->u32PostNpIso =
	algoRet->u32PostIso =
	algoRet->u32PreBlcIso =
	algoRet->u32PostBlcIso = pRawReplayBoardCtx->offline3aAlgoRet.iso;

	algoRet->currentLV = pRawReplayBoardCtx->offline3aAlgoRet.lv;

	algoRet->u32IspPostDgain =
	algoRet->u32IspPostDgainSE =
	algoRet->u32IspPreDgain =
	algoRet->u32IspPreDgainSE = pRawReplayBoardCtx->offline3aAlgoRet.ispDgain;

	algoRet->au32ExpRatio[0] = pRawReplayBoardCtx->offline3aAlgoRet.expRatio;

	algoRet->au32WhiteBalanceGainPre[ISP_BAYER_CHN_R] =
	algoRet->au32WhiteBalanceGain[ISP_BAYER_CHN_R] = pRawReplayBoardCtx->offline3aAlgoRet.wbgRgain;
	algoRet->au32WhiteBalanceGainPre[ISP_BAYER_CHN_GR] =
	algoRet->au32WhiteBalanceGain[ISP_BAYER_CHN_GR] = pRawReplayBoardCtx->offline3aAlgoRet.wbgGgain;
	algoRet->au32WhiteBalanceGainPre[ISP_BAYER_CHN_GB] =
	algoRet->au32WhiteBalanceGain[ISP_BAYER_CHN_GB] =  pRawReplayBoardCtx->offline3aAlgoRet.wbgGgain;
	algoRet->au32WhiteBalanceGainPre[ISP_BAYER_CHN_B] =
	algoRet->au32WhiteBalanceGain[ISP_BAYER_CHN_B] =  pRawReplayBoardCtx->offline3aAlgoRet.wbgBgain;

	return ret;
}

static CVI_S32 waiting_isp_mw_post_eof(CVI_U32 timeoutMillSec)
{
	struct timeval timeStart;
	struct timeval timeEnd;

	gettimeofday(&timeStart, NULL);

	while (true) {
		if (isp_mw_post_eof) {
			isp_mw_post_eof = 0;
			return 0;
		}

		gettimeofday(&timeEnd, NULL);
		if ((timeEnd.tv_sec - timeStart.tv_sec) * 1000 + (timeEnd.tv_usec - timeStart.tv_usec) / 1000
				> timeoutMillSec) {
			return -1;
		} else {
			usleep(100 * 1000); // 100 ms
		}
	}

	return 0;
}

static CVI_S32 load_raw_block(CVI_S32 frameStart, CVI_S32 frameEnd)
{
	if (frameStart > frameEnd) {
		CVI_S32 tmp = frameStart;

		frameStart = frameEnd;
		frameEnd = tmp;
	}

	size_t offsetBytes = frameStart * pRawReplayBoardCtx->sz12b;
	size_t readSize = (frameEnd - frameStart + 1) * pRawReplayBoardCtx->sz12b;

	fseek(pRawReplayBoardCtx->le12Fd, offsetBytes, SEEK_SET);
	fread(pRawReplayBoardCtx->rawLeVir, 1, readSize, pRawReplayBoardCtx->le12Fd);

	if (pRawReplayBoardCtx->rawSeVir) {
		fseek(pRawReplayBoardCtx->se12Fd, offsetBytes, SEEK_SET);
		fread(pRawReplayBoardCtx->rawSeVir, 1, readSize, pRawReplayBoardCtx->se12Fd);
	}

	return 0;
}

#if defined(__GNUC__) && defined(__riscv)
#pragma GCC diagnostic pop
#endif
