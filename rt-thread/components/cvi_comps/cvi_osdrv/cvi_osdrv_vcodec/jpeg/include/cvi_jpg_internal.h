#ifndef CVI_JPG_INTERNAL_H
#define CVI_JPG_INTERNAL_H

/* default jpu device struct */
#define WIDTH_BUFFER_SIZE 4096
#define HEIGHT_BUFFER_SIZE 2220

/* DEC */
int cviJpgDecOpen(CVIJpgHandle *pHandle, CVIDecConfigParam *pConfig);
int cviJpgDecClose(CVIJpgHandle handle);
int cviJpgDecSendFrameData(CVIJpgHandle jpgHandle, void *data, int length);
int cviJpgDecGetFrameData(CVIJpgHandle jpgHandle, void *data);
int cviJpgDecFlush(CVIJpgHandle jpgHandle);

/* ENC */
int cviJpgEncOpen(CVIJpgHandle *pHandle, CVIEncConfigParam *pConfig);
int cviJpgEncClose(CVIJpgHandle handle);
int cviJpgEncSendFrameData(CVIJpgHandle jpgHandle, void *data, int srcType);
int cviJpgEncGetFrameData(CVIJpgHandle jpgHandle, void *data);
int cviJpgEncFlush(CVIJpgHandle jpgHandle);
int cviJpgEncGetInputDataBuf(CVIJpgHandle jpgHandle, void *data);
int cviJpgEncResetQualityTable(CVIJpgHandle jpgHandle);
int cviJpgEncEncodeUserData(CVIJpgHandle jpgHandle, void *data);
int cviJpgEncStart(CVIJpgHandle jpgHandle, void *data);
int cviJpeEncSetRcParam(CVIJpgHandle pHandle, struct CVIJpegEncRcParam *rc);
int cvi_jpeg_scale_quality(int scale_factor);

extern void jpu_set_channel_num(int chnIdx);
#endif /* CVI_JPG_INTERNAL_H */
