//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include "vpuapifunc.h"
#include "main_helper.h"
#include "malloc.h"
typedef struct {
	FeedingMethod method;
	Uint8 *remainData;
	Uint32 remainDataSize;
	void *actualFeeder;
	PhysicalAddress base;
	Uint32 size;
	Uint32 fillingMode;
	BOOL eos;
	VpuThread threadHandle;
	DecHandle decHandle;
	EndianMode endian;
	BSFeederHook observer;
	void *observerArg;
	BOOL autoUpdate; /* TRUE - Call VPU_DecUpdateBitstreamBuffer() in the
			    bitstream feeder. default */
} BitstreamFeeder;

static void BitstreamFeeder_DummyObserver(void *handle, void *es, Uint32 size,
					  void *arg)
{
	UNREFERENCED_PARAMETER(handle);
	UNREFERENCED_PARAMETER(es);
	UNREFERENCED_PARAMETER(size);
	UNREFERENCED_PARAMETER(arg);
}
extern void *BSFeederEsIn_Create(void);
extern BOOL BSFeederEsIn_Destroy(void *feeder);
extern Int32 BSFeederEsIn_Act(void *feeder, BSChunk *chunk);
extern BOOL BSFeederEsIn_Rewind(void *feeder);

/**
 * Abstract Bitstream Feeader Functions
 */
void *BitstreamFeeder_Create(const char *path, FeedingMethod method,
			     PhysicalAddress base, Uint32 size, ...)
{
	/*lint -esym(438, ap) */
	BitstreamFeeder *handle = NULL;
	void *feeder = NULL;

	UNUSED(path);

	switch (method) {
	case FEEDING_METHOD_ES_IN:
		feeder = BSFeederEsIn_Create();
		break;
	default:
		feeder = NULL;
		break;
	}

	if (feeder != NULL) {
		handle = (BitstreamFeeder *)malloc(sizeof(BitstreamFeeder));
		if (handle == NULL) {
			CVI_VC_ERR("Failed to allocate memory\n");
			return NULL;
		}
		handle->actualFeeder = feeder;
		handle->method = method;
		handle->remainData = NULL;
		handle->remainDataSize = 0;
		handle->base = base;
		handle->size = size;
		handle->fillingMode = (method == FEEDING_METHOD_FIXED_SIZE) ?
						    BSF_FILLING_RINGBUFFER :
						    BSF_FILLING_LINEBUFFER;
		handle->threadHandle = NULL;
		handle->eos = FALSE;
		handle->observer = (BSFeederHook)BitstreamFeeder_DummyObserver;
		handle->observerArg = NULL;
		handle->autoUpdate = TRUE;
	}

	return handle;
	/*lint +esym(438, ap) */
}

void BitstreamFeeder_SetFillMode(BSFeeder feeder, Uint32 mode)
{
	BitstreamFeeder *bsf = (BitstreamFeeder *)feeder;

	switch (mode) {
	case BSF_FILLING_AUTO:
		bsf->fillingMode = (bsf->method == FEEDING_METHOD_FIXED_SIZE) ?
						 BSF_FILLING_RINGBUFFER :
						 BSF_FILLING_LINEBUFFER;
		break;
	case BSF_FILLING_RINGBUFFER:
	case BSF_FILLING_LINEBUFFER:
	case BSF_FILLING_RINGBUFFER_WITH_ENDFLAG:
		bsf->fillingMode = mode;
		break;
	default:
		VLOG(INFO, "%s Not supported mode %d\n", __func__, mode);
		break;
	}
}

static Uint32 FeedBitstream(BSFeeder feeder, vpu_buffer_t *buffer)
{
	BitstreamFeeder *bsf = (BitstreamFeeder *)feeder;
	Int32 feedingSize = 0;
	BSChunk chunk = { 0 };
	PhysicalAddress rdPtr, wrPtr;
	Uint32 room;
	DecHandle decHandle;
	EndianMode endian;

	if (bsf == NULL) {
		CVI_VC_ERR("Null handle\n");
		return 0;
	}

	decHandle = bsf->decHandle;
	endian = bsf->endian;

	if (bsf->remainData == NULL) {
		if (bsf->method == FEEDING_METHOD_ES_IN) {
			chunk.size = buffer->size;
			chunk.data = buffer->virt_addr;
			chunk.eos = FALSE;
		} else {
			chunk.size = bsf->size;
			chunk.data = malloc(chunk.size);
			chunk.eos = FALSE;
		}

		if (chunk.data == NULL) {
			CVI_VC_WARN("chunk.data\n");
			return 0;
		}

		switch (bsf->method) {
		case FEEDING_METHOD_ES_IN:
			feedingSize = BSFeederEsIn_Act(bsf->actualFeeder, &chunk);
			break;
		default:
			CVI_VC_ERR("Invalid method(%d)\n", bsf->method);
			free(chunk.data);
			return 0;
		}
	} else {
		chunk.data = bsf->remainData;
		feedingSize = bsf->remainDataSize;
	}

	CVI_VC_BS("feedingSize = %d\n", feedingSize);

	bsf->observer((void *)bsf, chunk.data, feedingSize, bsf->observerArg);

	if (feedingSize < 0) {
		CVI_VC_ERR("feeding size is negative value: %d\n", feedingSize);

		if (chunk.data)
			free(chunk.data);
		return 0;
	}

	if (feedingSize > 0) {
		Uint32 coreIdx = VPU_HANDLE_CORE_INDEX(decHandle);
		Uint32 rightSize = 0, leftSize = feedingSize;

		if (bsf->method == FEEDING_METHOD_ES_IN || buffer == NULL) {
			VPU_DecGetBitstreamBuffer(decHandle, &rdPtr, &wrPtr, &room);
			CVI_VC_BS("rdPtr = 0x"VCODEC_UINT64_FORMAT_HEX", wrPtr = 0x"VCODEC_UINT64_FORMAT_HEX", room = 0x%X\n", rdPtr, wrPtr, room);
		} else {
			rdPtr = wrPtr = buffer->phys_addr;
			room = buffer->size;
			CVI_VC_BS("\n");
		}

		if ((Int32)room < feedingSize) {
			bsf->remainData = chunk.data;
			bsf->remainDataSize = feedingSize;
			CVI_VC_ERR("room < feedingSize\n");
			return 0;
		}

		if (bsf->fillingMode == BSF_FILLING_RINGBUFFER ||
		    bsf->fillingMode == BSF_FILLING_RINGBUFFER_WITH_ENDFLAG) {
			if ((wrPtr + feedingSize) >= (bsf->base + bsf->size)) {
				PhysicalAddress endAddr = bsf->base + bsf->size;

				CVI_VC_TRACE("endAddr = 0x"VCODEC_UINT64_FORMAT_HEX"\n", endAddr);

				rightSize = endAddr - wrPtr;
				leftSize = (wrPtr + feedingSize) - endAddr;
				if (rightSize > 0) {
					VpuWriteMem(coreIdx, wrPtr, (unsigned char *)chunk.data, rightSize, (int)endian);
#if (defined CVI_H26X_USE_ION_MEM && defined BITSTREAM_ION_CACHED_MEM)
					vdi_flush_ion_cache(wrPtr, (void *)(chunk.data), rightSize);
#endif
				}
				wrPtr = bsf->base;
			}
		}

		VpuWriteMem(coreIdx, wrPtr,
			    (unsigned char *)chunk.data + rightSize, leftSize,
			    (int)endian);
#if (defined CVI_H26X_USE_ION_MEM && defined BITSTREAM_ION_CACHED_MEM)
		vdi_flush_ion_cache(wrPtr, (void *)(chunk.data + rightSize), leftSize);
#endif
		CVI_VC_BS("rightSize = 0x%X, leftSize = 0x%X\n", rightSize, leftSize);
	}

	if (bsf->autoUpdate == TRUE) {
		/* If feedingSize is equal to zero then VPU will be ready to
		 * terminate current sequence. */
		VPU_DecUpdateBitstreamBuffer(decHandle, feedingSize);
		if (chunk.eos == TRUE ||
		    bsf->fillingMode == BSF_FILLING_RINGBUFFER_WITH_ENDFLAG) {
			VPU_DecUpdateBitstreamBuffer(decHandle, STREAM_END_SET_FLAG);
		}
	}

	bsf->eos = chunk.eos;

	if (bsf->method != FEEDING_METHOD_ES_IN)
		free(chunk.data);

	bsf->remainData = NULL;
	bsf->remainDataSize = 0;

	return feedingSize;
}

Uint32 BitstreamFeeder_Act(DecHandle decHandle, BSFeeder feeder,
			   EndianMode endian, vpu_buffer_t *buffer)
{
	BitstreamFeeder *bsf = (BitstreamFeeder *)feeder;

	bsf->decHandle = decHandle;
	bsf->endian = endian;
	return FeedBitstream(feeder, buffer);
}

BOOL BitstreamFeeder_Destroy(BSFeeder feeder)
{
	BitstreamFeeder *bsf = (BitstreamFeeder *)feeder;

	if (bsf == NULL) {
		CVI_VC_ERR("Null handle\n");
		return FALSE;
	}

	if (bsf->threadHandle) {
		bsf->eos = TRUE;
		VpuThread_Join(bsf->threadHandle);
		free(bsf->threadHandle);
		bsf->threadHandle = NULL;
	}

	switch (bsf->method) {
	case FEEDING_METHOD_ES_IN:
		BSFeederEsIn_Destroy(bsf->actualFeeder);
		break;
	default:
		CVI_VC_ERR("Invalid method(%d)\n", bsf->method);
		break;
	}

	if (bsf->remainData) {
		free(bsf->remainData);
	}

	free(bsf);

	return TRUE;
}
