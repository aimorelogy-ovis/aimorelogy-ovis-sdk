#include "stdint.h"
#include "jpuapi.h"
#include "regdefine.h"
#include "jpulog.h"
#include "jpuhelper.h"
#include "mixer.h"
#include <string.h>

#ifndef UNREFERENCED_PARAM
#define UNREFERENCED_PARAM(x) ((void)(x))
#endif

typedef struct {
#ifdef MJPEG_INTERFACE_API
	FRAME_BUF frameBuf[MAX_FRAME_JPU];
#else
	FRAME_BUF frameBuf[MAX_FRAME];
#endif
	jpu_buffer_t vb_base;
	int instIndex;
	int last_num;
	Uint64 last_addr;
} fb_context;

static fb_context s_fb[MAX_NUM_INSTANCE];

int AllocateFrameBuffer(int instIdx, int format, int width, int height,
			int frameBufNum, int pack)
{
	unsigned int divX, divY;
	int i;
	unsigned int lum_size, chr_size;
	fb_context *fb;
#ifdef MJPEG_INTERFACE_API
	unsigned long fb_virt_addr;
#endif /* MJPEG_INTERFACE_API */

	fb = &s_fb[instIdx];

	memset(fb, 0x00, sizeof(fb_context));

	divX = format == FORMAT_420 || format == FORMAT_422 ? 2 : 1;
	divY = format == FORMAT_420 || format == FORMAT_224 ? 2 : 1;

	switch (format) {
	case FORMAT_420:
		height = ((height + 1) >> 1) << 1;
		width = ((width + 1) >> 1) << 1;
		break;
	case FORMAT_224:
		height = ((height + 1) >> 1) << 1;
		break;
	case FORMAT_422:
		width = ((width + 1) >> 1) << 1;
		break;
	case FORMAT_444:
		height = ((height + 1) >> 1) << 1;
		width = ((width + 1) >> 1) << 1;
		break;
	case FORMAT_400:
		height = ((height + 1) >> 1) << 1;
		width = ((width + 1) >> 1) << 1;
		break;
	}

	lum_size = (unsigned int)width * (unsigned int)height;

	if (pack)
		chr_size = 0;
	else
		chr_size = lum_size / divX / divY;

	fb->vb_base.size = lum_size + chr_size * 2;
	fb->vb_base.size *= frameBufNum;

	CVI_JPG_DBG_MEM("src FB vb_base.size = 0x%x, frameBufNum = %d\n",
			fb->vb_base.size, frameBufNum);

	if (JDI_ALLOCATE_MEMORY(&fb->vb_base, 0, 0) < 0) {
		JLOG(ERR, "Fail to allocate frame buffer size=%d\n",
		     fb->vb_base.size);
		return 0;
	}

	fb->last_addr = fb->vb_base.phys_addr;
#ifdef MJPEG_INTERFACE_API
	fb_virt_addr = (unsigned long)fb->vb_base.virt_addr;
#endif /* MJPEG_INTERFACE_API */

	for (i = fb->last_num; i < fb->last_num + frameBufNum; i++) {
		fb->frameBuf[i].Format = format;
		fb->frameBuf[i].Index = i;

		fb->frameBuf[i].vbY.phys_addr = fb->last_addr;
		fb->frameBuf[i].vbY.size = lum_size;
#ifdef MJPEG_INTERFACE_API
		fb->frameBuf[i].vbY.virt_addr = (void *)fb_virt_addr;
		fb_virt_addr += fb->frameBuf[i].vbY.size;
		fb_virt_addr = ((fb_virt_addr + 7) & ~7);
#endif /* MJPEG_INTERFACE_API */

		fb->last_addr += fb->frameBuf[i].vbY.size;
		fb->last_addr = ((fb->last_addr + 7) & ~7);

		if (chr_size) {
			fb->frameBuf[i].vbCb.phys_addr = fb->last_addr;
			fb->frameBuf[i].vbCb.size = chr_size;
#ifdef MJPEG_INTERFACE_API
			fb->frameBuf[i].vbCb.virt_addr = (void *)fb_virt_addr;
			fb_virt_addr += fb->frameBuf[i].vbCb.size;
			fb_virt_addr = ((fb_virt_addr + 7) & ~7);
#endif /* MJPEG_INTERFACE_API */

			fb->last_addr += fb->frameBuf[i].vbCb.size;
			fb->last_addr = ((fb->last_addr + 7) & ~7);

			fb->frameBuf[i].vbCr.phys_addr = fb->last_addr;
			fb->frameBuf[i].vbCr.size = chr_size;
#ifdef MJPEG_INTERFACE_API
			fb->frameBuf[i].vbCr.virt_addr = (void *)fb_virt_addr;
			fb_virt_addr += fb->frameBuf[i].vbCr.size;
			fb_virt_addr = ((fb_virt_addr + 7) & ~7);
#endif /* MJPEG_INTERFACE_API */

			fb->last_addr += fb->frameBuf[i].vbCr.size;
			fb->last_addr = ((fb->last_addr + 7) & ~7);
		}

		fb->frameBuf[i].strideY = width;
		fb->frameBuf[i].strideC = width / divX;
	}

#ifdef CVIDEBUG_V
	for (i = fb->last_num; i < fb->last_num + frameBufNum; i++) {
#ifdef MJPEG_INTERFACE_API
		JLOG(TRACE, " >_Create DPB [ %i] -- Linear\n", i);
		JLOG(TRACE, " >_ Luminance Frame "VCODEC_UINT64_FORMAT_HEX", %p\n",
		     fb->frameBuf[i].vbY.phys_addr,
		     (void *)fb->frameBuf[i].vbY.virt_addr);
		JLOG(TRACE, " >_ Cb        Frame "VCODEC_UINT64_FORMAT_HEX", %p\n",
		     fb->frameBuf[i].vbCb.phys_addr,
		     (void *)fb->frameBuf[i].vbCb.virt_addr);
		JLOG(TRACE, " >_ Cr        Frame "VCODEC_UINT64_FORMAT_HEX", %p\n",
		     fb->frameBuf[i].vbCr.phys_addr,
		     (void *)fb->frameBuf[i].vbCr.virt_addr);
		JLOG(TRACE, " >_ fb->last_addr     "VCODEC_UINT64_FORMAT_HEX"\n", fb->last_addr);
		JLOG(TRACE, " >_ StrideY         (%d)\n",
		     fb->frameBuf[i].strideY);
		JLOG(TRACE, " >_ StrideC         (%d)\n",
		     fb->frameBuf[i].strideC);
#else
		JLOG(TRACE, " >_Create DPB [ %i] -- Linear\n", i);
		JLOG(TRACE, " >_ Luminance Frame 0x%08x\n",
		     fb->frameBuf[i].vbY.phys_addr);
		JLOG(TRACE, " >_ Cb        Frame 0x%08x\n",
		     fb->frameBuf[i].vbCb.phys_addr);
		JLOG(TRACE, " >_ Cr        Frame 0x%08x\n",
		     fb->frameBuf[i].vbCr.phys_addr);
		JLOG(TRACE, " >_ fb->last_addr     0x"VCODEC_UINT64_FORMAT_HEX"\n", fb->last_addr);
		JLOG(TRACE, " >_ StrideY         (%d)\n",
		     fb->frameBuf[i].strideY);
		JLOG(TRACE, " >_ StrideC         (%d)\n",
		     fb->frameBuf[i].strideC);
#endif
	}
#endif

	fb->last_num += frameBufNum;

	return 1;
}

int GetFrameBufBase(int instIdx)
{
	fb_context *fb;
	fb = &s_fb[instIdx];

	return fb->vb_base.phys_addr;
}

int GetFrameBufAllocSize(int instIdx)
{
	fb_context *fb;
	fb = &s_fb[instIdx];

	return (fb->last_addr - fb->vb_base.phys_addr);
}

FRAME_BUF *GetFrameBuffer(int instIdx, int index)
{
	fb_context *fb;
	fb = &s_fb[instIdx];
	return &fb->frameBuf[index];
}

FRAME_BUF *FindFrameBuffer(int instIdx, PhysicalAddress addrY)
{
	int i;
#ifdef MJPEG_INTERFACE_API
	int max_frame = MAX_FRAME_JPU;
#else
	int max_frame = MAX_FRAME;
#endif
	fb_context *fb;

	fb = &s_fb[instIdx];

	for (i = 0; i < max_frame; i++) {
		if (fb->frameBuf[i].vbY.phys_addr == addrY) {
			return &fb->frameBuf[i];
		}
	}

	return NULL;
}

void ClearFrameBuffer(int instIdx, int index)
{
	UNREFERENCED_PARAM(instIdx);
	UNREFERENCED_PARAM(index);
}

void FreeFrameBuffer(int instIdx)
{
	fb_context *fb;

	fb = &s_fb[instIdx];

	fb->last_num = 0;
	fb->last_addr = -1;
	JDI_FREE_MEMORY(&fb->vb_base);
	fb->vb_base.base = 0;
	fb->vb_base.size = 0;
}
