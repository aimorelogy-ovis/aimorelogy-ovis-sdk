
#ifndef _JPU_HELPER_H_
#define _JPU_HELPER_H_

#include "jpurun.h"
#include "jpuapi.h"

typedef struct {
	char SrcFileName[256];
	int NumFrame;
	int PicX;
	int PicY;
	int FrameRate;

	// MPEG4 ONLY
	int VerId;
	int DataPartEn;
	int RevVlcEn;
	int ShortVideoHeader;
	int AnnexI;
	int AnnexJ;
	int AnnexK;
	int AnnexT;
	int IntraDcVlcThr;
	int VopQuant;

	// H.264 ONLY
	int ConstIntraPredFlag;
	int DisableDeblk;
	int DeblkOffsetA;
	int DeblkOffsetB;
	int ChromaQpOffset;
	int PicQpY;

	// MJPEG ONLY
	char HuffTabName[256];
	char QMatTabName[256];
	int VersionID;
	int FrmFormat;
	int SrcFormat;
	int RstIntval;
	int ThumbEnable;
	int ThumbSizeX;
	int ThumbSizeY;

	// COMMON
	int GopPicNum;
	int SliceMode;
	int SliceSizeMode;
	int SliceSizeNum;

	int IntraRefreshNum;

	int ConstantIntraQPEnable;
	int RCIntraQP;
	int MaxQpSetEnable;
	int MaxQp;
	int GammaSetEnable;
	int Gamma;
	int HecEnable;

	// RC
	int RcEnable;
	int RcBitRate;
	int RcInitDelay;
	int RcBufSize;

	// NEW RC Scheme
	int RcIntervalMode;
	int RcMBInterval;
	int IntraCostWeight;
	int SearchRange;
	int MeUseZeroPmv;
	int MeBlkModeEnable;

} ENC_CFG;

typedef struct {
	FrameFormat sourceFormat;
	int restartInterval;
	BYTE huffVal[4][162];
	BYTE huffBits[4][256];
	BYTE qMatTab[4][64];
} EncMjpgParam;

#if defined(__cplusplus)
extern "C" {
#endif

int jpgGetHuffTable(EncMjpgParam *param);
int jpgGetQMatrix(EncMjpgParam *param);

int getJpgEncOpenParamDefault(JpgEncOpenParam *pEncOP, EncConfigParam *pEncConfig);

JpgRet WriteJpgBsBufHelper(JpgDecHandle handle, BufInfo *pBufInfo,
			   PhysicalAddress paBsBufStart,
			   PhysicalAddress paBsBufEnd, int defaultsize,
			   int checkeos, int *pstreameos, int endian);

unsigned int GetFrameBufSize(int framebufFormat, int picWidth, int picHeight);
void GetMcuUnitSize(int format, int *mcuWidth, int *mcuHeight);
Uint64 jpgGetCurrentTime(void);
#if defined(__cplusplus)
}
#endif

#endif //#ifndef _JPU_HELPER_H_
