#ifndef _RINGBUF_H_
#define _RINGBUF_H_

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define RBUF_ALIGN_UP(x, a) ((x + a - 1) & (~(a - 1)))
#define RINGBUF_ALIGN_SIZE 4096
#define FRAMEDATA_ALIGN_SIZE 64

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

typedef enum _RECORD_TYPE_E {
    RBUF_RECORD_TYPE_NORMAL,
    RBUF_RECORD_TYPE_BUTT
} RBUF_RECORD_TYPE_E;// be care of RECORDER_TYPE_E

typedef void * (*RBUF_MALLOC_CB)(uint32_t size , const char * name);
typedef void(*RBUF_FREE_CB)(void * vir_addr);

int32_t RBUF_Create(void **rbuf, uint32_t size, const char *name, void * mallocmemcb, void * freememcb);
void RBUF_Destroy(void *rbuf);
void *RBUF_Req_Mem(void *rbuf, uint32_t size);
int32_t RBUF_Refresh_WritePos(void *rbuf, uint32_t offs);
void *RBUF_ReadData(void *rbuf, RBUF_RECORD_TYPE_E type);
int32_t RBUF_Refresh_ReadPos(void *rbuf, uint32_t offs, RBUF_RECORD_TYPE_E type);
void RBUF_WriteRollback(void *rbuf, void *pos, uint32_t offs);
uint64_t RBUF_Get_DataCnt(void *rbuf);
uint32_t RBUF_Get_RemainSize(void *rbuf);
void RBUF_ShowMeminfo(void *rbuf);


int32_t RBUF_Init(void **rbuf, int32_t size, const char *name, int32_t outcnt, void * mallocmemcb, void * freememcb);
void RBUF_DeInit(void *rbuf);
uint32_t RBUF_DataCnt(void *rbuf, int32_t inx);
int32_t RBUF_Copy_In(void *rbuf, void *src, int32_t len, int32_t off);
int32_t RBUF_Copy_Out(void *rbuf, void *dst, int32_t len, int32_t off, int32_t inx);
uint32_t RBUF_Unused(void *rbuf);
void RBUF_Refresh_In(void *rbuf, int32_t off);
void RBUF_Refresh_Out(void *rbuf, int32_t off, int32_t inx);
void RBUF_ShowLog(void *rbuf);
int32_t RBUF_Get_Totalsize(void *rbuf);
int32_t RBUF_Reset(void *rbuf);
uint64_t RBUF_Get_InSize(void *rbuf);
int32_t RBUF_Copy_OutTmp(void *rbuf, void *dst, int32_t len, int32_t off, int32_t inx);
void RBUF_Refresh_OutTmp(void *rbuf, int32_t off, int32_t inx);

#ifdef __cplusplus
}
#endif


#endif
