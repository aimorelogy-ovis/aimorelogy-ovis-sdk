#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#include "loadbmp.h"
#include "cvi_comm_gfbg.h"
#include "cvi_comm_video.h"
#include "cvi_sys.h"
#include "cvi_tde.h"
#include "vo_comm.h"

#define FILE_LENGTH_MAX 12
#define CMAP_LENGTH_MAX 256
#define CMAP_LENGTH_MIN 16
#define WIDTH_720P 720
#define HEIGHT_1280P 1280

#define SAMPLE_IMAGE_WIDTH     304
#define SAMPLE_IMAGE_HEIGHT    186
#define SAMPLE_IMAGE_NUM       4

#define SAMPLE_IMAGE2_PATH	"./res/tiger256lut.bmp"
#define SAMPLE_CURSOR_PATH	"./res/dog.bmp"
#define TDE_DEFAULT_FILE_IN     "./res/1280_720_bgra.bin"

#define GFBG_RED_1555          0xFC00
#define GFBG_RED_8888          0xFFFF0000

#define GFBG_RED_256LUT        0x0
#define GFBG_GREEN_256LUT      0x1
#define GFBG_BLUE_256LUT       0x2
#define GFBG_WHITE_256LUT      0x3

#define GFBG_RED_16LUT        0x00
#define GFBG_GREEN_16LUT      0x11
#define GFBG_BLUE_16LUT       0x22
#define GFBG_WHITE_16LUT      0x33

#define GRAPHICS_LAYER_G0      VO_LAYER_G0

static int g_sample_gfbg_exit;
pthread_t g_gfbg_thread;

OSD_COLOR_FMT_E g_osd_color_fmt = OSD_COLOR_FMT_RGB1555;

static struct fb_bitfield g_r16 = {10, 5, 0};
static struct fb_bitfield g_g16 = {5, 5, 0};
static struct fb_bitfield g_b16 = {0, 5, 0};
static struct fb_bitfield g_a16 = {15, 1, 0};

static struct fb_bitfield g_r32 = {16, 8, 0};
static struct fb_bitfield g_g32 = {8,  8, 0};
static struct fb_bitfield g_b32 = {0,  8, 0};
static struct fb_bitfield g_a32 = {24, 8, 0};

static struct fb_bitfield g_r8 = {0, 8, 0};
static struct fb_bitfield g_g8 = {0, 8, 0};
static struct fb_bitfield g_b8 = {0, 8, 0};
static struct fb_bitfield g_a8 = {0, 0, 0};

static struct fb_bitfield g_r4 = {0, 4, 0};
static struct fb_bitfield g_g4 = {0, 4, 0};
static struct fb_bitfield g_b4 = {0, 4, 0};
static struct fb_bitfield g_a4 = {0, 0, 0};

CVI_U32 g_colors_len;
CVI_U16 g_cmap_alpha[CMAP_LENGTH_MAX] = {0xff,	0xff,	0xff,	0xff};
CVI_U16 g_cmap_red[CMAP_LENGTH_MAX] =	{0xff,	0,	0,	0xff};
CVI_U16 g_cmap_green[CMAP_LENGTH_MAX] = {0,	0xff,	0,	0xff};
CVI_U16 g_cmap_blue[CMAP_LENGTH_MAX] =	{0,	0,	0xff,	0xff};

CVI_U64 g_phyaddr;
CVI_U64 g_canvas_addr;

static VO_UT_FILE vo_file = {
	.stSize = { .u32Width = 720, .u32Height = 1280 },
	.filename = "res/720x1280.nv21",
	.enPixelFormat = PIXEL_FORMAT_NV21
};

typedef struct {
	CVI_S32 fd; /* fb's file describe */
	CVI_S32 layer; /* which graphic layer */
	CVI_S32 ctrlkey; /* {0,1,2,3}={1buffer, 2buffer, 0buffer pan display, 0buffer refresh} */
	CVI_BOOL compress; /* image compressed or not */
	cvi_fb_color_format color_format; /* color format. */
	CVI_BOOL use_tde; /* image compressed or not */
} pthread_gfbg_sample_info;

CVI_VOID sample_sys_signal(void (*func)(int))
{
	struct sigaction sa = { 0 };

	sa.sa_handler = func;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, CVI_NULL);
	sigaction(SIGTERM, &sa, CVI_NULL);
}

static CVI_VOID sample_gfbg_handle_sig(CVI_S32 signo)
{
	static int sig_handled;

	if (!sig_handled && (signo == SIGINT || signo == SIGTERM)) {
		sig_handled = 1;
		g_sample_gfbg_exit = 1;
	}
}

static CVI_VOID sample_gfbg_usage2(CVI_VOID)
{
	SAMPLE_PRT("\n\n/****************index******************/\n");
	SAMPLE_PRT("please choose the case which you want to run:\n");
	SAMPLE_PRT("\t0:  ARGB8888 standard mode with colorkey set\n");
	SAMPLE_PRT("\t1:  ARGB8888 standard mode with move cursor + scale(w,h) x 2 by graphic layer\n");
	SAMPLE_PRT("\t2:  ARGB1555 BUF_NONE mode\n");
	SAMPLE_PRT("\t3:  CLUT256 standard mode\n");
	SAMPLE_PRT("\t4:  ARGB888 standard mode with tde\n");
	SAMPLE_PRT("\t5:  ARGB888 standard mode with oenc\n");
}

static CVI_VOID sample_gfbg_usage1(CVI_CHAR *s_prg_nm)
{
	SAMPLE_PRT("usage : %s <index>\n", s_prg_nm);
	sample_gfbg_usage2();
}

static CVI_VOID sample_gfbg_to_exit(CVI_VOID)
{
	if (g_gfbg_thread != 0) {
		pthread_join(g_gfbg_thread, 0);
		g_gfbg_thread = 0;
	}
}

static CVI_S32 sample_get_file_name(pthread_gfbg_sample_info *info, CVI_CHAR *file)
{
	switch (info->layer) {
	case GRAPHICS_LAYER_G0:
		if (strncpy(file, "/dev/fb0", strlen("/dev/fb0") + 1) != file) {
			SAMPLE_PRT("%s:%d:strncpy_s failed.\n", __func__, __LINE__);
			return CVI_FAILURE;
		}
		break;
	default:
		if (strncpy(file, "/dev/fb0", strlen("/dev/fb0") + 1) != file) {
			SAMPLE_PRT("%s:%d:strncpy_s failed.\n", __func__, __LINE__);
			return CVI_FAILURE;
		}
		break;
	}
	return CVI_SUCCESS;
}

static CVI_S32 sample_init_frame_buffer(pthread_gfbg_sample_info *info, const char *input_file)
{
	CVI_BOOL show;
	cvi_fb_point point = {0, 0};

	/* open framebuffer device overlay 0 */
	info->fd = open(input_file, O_RDWR, 0);
	if (info->fd < 0) {
		SAMPLE_PRT("open %s failed!\n", input_file);
		return CVI_FAILURE;
	}

	show = CVI_FALSE;
	if (ioctl(info->fd, FBIOPUT_SHOW_GFBG, &show) < 0) {
		SAMPLE_PRT("FBIOPUT_SHOW_GFBG failed!\n");
		close(info->fd);
		info->fd = -1;
		return CVI_FAILURE;
	}

	// wait a vblank
	if (ioctl(info->fd, FBIOGET_VER_BLANK_GFBG, NULL) < 0) {
		SAMPLE_PRT("wait a vblank failed!\n");
		return CVI_FAILURE;
	}

	/* set the screen original position */
	switch (info->ctrlkey) {
	case 3:
		if (info->use_tde) {
			point.x_pos = 0; /* 0 x pos */
			point.y_pos = 0; /* 0 y pos */
		} else {
			point.x_pos = 150; /* 150 x pos */
			point.y_pos = 150; /* 150 y pos */
		}
		break;
	default:
		point.x_pos = 0;
		point.y_pos = 0;
		break;
	}

	if (ioctl(info->fd, FBIOPUT_SCREEN_ORIGIN_GFBG, &point) < 0) {
		SAMPLE_PRT("set screen original show position failed!\n");
		close(info->fd);
		info->fd = -1;
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sample_init_var(pthread_gfbg_sample_info *info)
{
	struct fb_var_screeninfo var;

	if (ioctl(info->fd, FBIOGET_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("get variable screen info failed!\n");
		return CVI_FAILURE;
	}

	switch (info->color_format) {
	case CVI_FB_FORMAT_ARGB8888:
		var.transp = g_a32;
		var.red = g_r32;
		var.green = g_g32;
		var.blue = g_b32;
		var.bits_per_pixel = 32; /* 32 for 4 byte */
		g_osd_color_fmt = OSD_COLOR_FMT_RGB8888;
		break;
	default:
		var.transp = g_a16;
		var.red = g_r16;
		var.green = g_g16;
		var.blue = g_b16;
		var.bits_per_pixel = 16; /* 16 for 2 byte */
		break;
	}

	switch (info->ctrlkey) {
	case 3:
		if (info->use_tde) {
			var.xres_virtual = WIDTH_720P;
			var.yres_virtual = HEIGHT_1280P * 2; /* 2 for 2buf */
			var.xres = WIDTH_720P;
			var.yres = HEIGHT_1280P;
		} else {
			var.xres_virtual = 72;
			var.yres_virtual = 60;
			var.xres = 72;
			var.yres = 60;
		}
		break;
	default:
		var.xres_virtual = WIDTH_720P;
		var.yres_virtual = HEIGHT_1280P * 2; /* 2 for 2buf */
		var.xres = WIDTH_720P;
		var.yres = HEIGHT_1280P;
		break;
	}
	var.activate       = FB_ACTIVATE_NOW;

	if (ioctl(info->fd, FBIOPUT_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("put variable screen info failed!\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

static CVI_VOID sample_draw_rect_by_cpu(CVI_VOID *ptemp, cvi_fb_color_format clr_fmt, struct fb_var_screeninfo *var)
{
	CVI_S32 x, y;

	for (y = 100; y < 300; y++) { /* 100 300 for y */
		for (x = 0; x < 300; x++) { /* 300 for x */
			if (clr_fmt == CVI_FB_FORMAT_ARGB8888) {
				*((CVI_U32 *)ptemp + y * var->xres + x) = GFBG_RED_8888;
			} else {
				*((CVI_U16 *)ptemp + y * var->xres + x) = GFBG_RED_1555;
			}
		}
	}
}

// #define FLIP_SURFACE /* to use FBIOFLIP_SURFACE replace FBIOPUT_COLORKEY_GFBG + FBIOPAN_DISPLAY */
static CVI_S32 sample_put_colorkey(pthread_gfbg_sample_info *info, cvi_fb_color_format clr_fmt, CVI_U64 phyaddr)
{
	CVI_S32 ret;
#ifndef FLIP_SURFACE
	cvi_fb_colorkey color_key;
	(void)phyaddr;
#else
	cvi_fb_surfaceex surfaceex;
#endif
	SAMPLE_PRT("expected: the red box will erased by colorkey!\n");
#ifndef FLIP_SURFACE
	color_key.enable = CVI_TRUE;
	color_key.value = (clr_fmt == CVI_FB_FORMAT_ARGB8888) ? GFBG_RED_8888 : GFBG_RED_1555;
	ret = ioctl(info->fd, FBIOPUT_COLORKEY_GFBG, &color_key);
	if (ret < 0) {
		SAMPLE_PRT("FBIOPUT_COLORKEY_GFBG failed!\n");
		return CVI_FAILURE;
	}
#else
	surfaceex.phys_addr = phyaddr;
	surfaceex.colorkey.enable = CVI_TRUE;
	surfaceex.colorkey.value = (clr_fmt == CVI_FB_FORMAT_ARGB8888) ? GFBG_RED_8888 : GFBG_RED_1555;
	ret = ioctl(info->fd, FBIOFLIP_SURFACE, &surfaceex);
	if (ret < 0) {
		SAMPLE_PRT("FBIOFLIP_SURFACE failed!\n");
		return CVI_FAILURE;
	}
#endif
	sleep(2); /* 2 second */
	SAMPLE_PRT("expected: the red box will appear again!\n");
#ifndef FLIP_SURFACE
	color_key.enable = CVI_FALSE;
	ret = ioctl(info->fd, FBIOPUT_COLORKEY_GFBG, &color_key);
	if (ret < 0) {
		SAMPLE_PRT("FBIOPUT_COLORKEY_GFBG failed!\n");
		return CVI_FAILURE;
	}
#else
	surfaceex.colorkey.enable = CVI_FALSE;
	ret = ioctl(info->fd, FBIOFLIP_SURFACE, &surfaceex);
	if (ret < 0) {
		SAMPLE_PRT("FBIOPUT_COLORKEY_GFBG failed!\n");
		return CVI_FAILURE;
	}
#endif
	return CVI_SUCCESS;
}

static CVI_S32 sample_time_to_play(pthread_gfbg_sample_info *info, CVI_U8 *show_screen, CVI_U32 fix_screen_stride)
{
	CVI_BOOL show;
	CVI_VOID *ptemp = CVI_NULL;
	CVI_S32 i;
	struct fb_var_screeninfo var;
	cvi_fb_color_format clr_fmt;
	CVI_U64 phyaddr = 0;

#ifdef FLIP_SURFACE
	struct fb_fix_screeninfo fix;
	CVI_U64 base_phyaddr;
#endif

	if (ioctl(info->fd, FBIOGET_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("get variable screen info failed!\n");
		return CVI_FAILURE;
	}

	show = CVI_TRUE;
	if (ioctl(info->fd, FBIOPUT_SHOW_GFBG, &show) < 0) {
		SAMPLE_PRT("FBIOPUT_SHOW_GFBG failed!\n");
		return CVI_FAILURE;
	}

	if (ioctl(info->fd, FBIOPUT_COMPRESSION_GFBG, &info->compress) < 0) {
		SAMPLE_PRT("FBIOPUT_COMPRESSION_GFBG failed!\n");
		return CVI_FAILURE;
	}

	// move cursor case no need
	if (info->ctrlkey == 3)
		return CVI_SUCCESS;

	switch (info->color_format) {
	case CVI_FB_FORMAT_ARGB8888:
		clr_fmt = CVI_FB_FORMAT_ARGB8888;
		break;
	default:
		clr_fmt = CVI_FB_FORMAT_ARGB1555;
		break;
	}

#ifdef FLIP_SURFACE
	if (ioctl(info->fd, FBIOGET_FSCREENINFO, &fix) < 0) {
		return CVI_FAILURE;
	}
	base_phyaddr = fix.smem_start;
	SAMPLE_PRT("base_phyaddr=0x%lx\n", base_phyaddr);
#endif

	for (i = 0; i < 4; i++) {
#ifndef FLIP_SURFACE
		if (i % 2) { /* 2 for 0 or 1 */
			var.yoffset = var.yres;
		} else {
			var.yoffset = 0;
		}

		if (ioctl(info->fd, FBIOPAN_DISPLAY, &var) < 0) {
			SAMPLE_PRT("FBIOPAN_DISPLAY failed!\n");
			return CVI_FAILURE;
		}
#else
		phyaddr = base_phyaddr + var.yres * fix_screen_stride * (i % 2);
#endif
		ptemp = (show_screen + var.yres * fix_screen_stride * ((i + 1) % 2)); /* 2 for 0 or 1 */
		sample_draw_rect_by_cpu(ptemp, clr_fmt, &var);

		/* colorkey only argb8888 support */
		if (sample_put_colorkey(info, clr_fmt, phyaddr) != CVI_SUCCESS) {
			return CVI_FAILURE;
		}

		sleep(2); /* 2 second */
	}
	return CVI_SUCCESS;
}

static CVI_S32 sample_gfbg_load_bmp(const char *filename, CVI_U8 *addr)
{
	OSD_SURFACE_S Surface;
	OSD_BITMAPFILEHEADER bmp_file_header;
	OSD_BITMAPINFO bmp_info;

	if (GetBmpInfo(filename, &bmp_file_header, &bmp_info) < 0) {
		SAMPLE_PRT("get_bmp_info err!\n");
		return CVI_FAILURE;
	}

	Surface.enColorFmt = g_osd_color_fmt;
	CreateSurfaceByBitMap(filename, &Surface, addr);
	return CVI_SUCCESS;
}

static CVI_S32 sample_move_cursor(pthread_gfbg_sample_info *info, struct fb_var_screeninfo *var, CVI_U8 *show_screen)
{
	cvi_fb_size screen_size;
	cvi_fb_point point = {0, 0};

	point.x_pos = (info->ctrlkey == 3) ? 150 : 0; /* 3 150:for case;alg data */
	point.y_pos = (info->ctrlkey == 3) ? 150 : 0; /* 3 150:for case;alg data */

	if (sample_gfbg_load_bmp(SAMPLE_CURSOR_PATH, show_screen) != CVI_SUCCESS) {
		return CVI_FAILURE;
	}
	if (ioctl(info->fd, FBIOPAN_DISPLAY, var) < 0) {
		SAMPLE_PRT("FBIOPAN_DISPLAY failed!\n");
		return CVI_FAILURE;
	}

	// scale x 2 by Graphic layer
	if (ioctl(info->fd, FBIOGET_SCREEN_SIZE, &screen_size) < 0) {
		SAMPLE_PRT("FBIOGET_SCREEN_SIZE failed!\n");
		return CVI_FAILURE;
	}
	SAMPLE_PRT("FBIOPUT_SCREEN_SIZE before(%d,%d)\n", screen_size.width, screen_size.height);

	screen_size.width *= 2;
	screen_size.height *= 2;
	if (ioctl(info->fd, FBIOPUT_SCREEN_SIZE, &screen_size) < 0) {
		SAMPLE_PRT("FBIOPUT_SCREEN_SIZE failed!\n");
		return CVI_FAILURE;
	}

	if (ioctl(info->fd, FBIOGET_SCREEN_SIZE, &screen_size) < 0) {
		SAMPLE_PRT("FBIOGET_SCREEN_SIZE failed!\n");
		return CVI_FAILURE;
	}
	SAMPLE_PRT("FBIOPUT_SCREEN_SIZE after(%d,%d)\n", screen_size.width, screen_size.height);

	SAMPLE_PRT("show cursor\n");
	sleep(1); /* 1 second */
	while (point.x_pos <= WIDTH_720P) {
		point.x_pos += 2; /* 2 pos */
		if (ioctl(info->fd, FBIOPUT_SCREEN_ORIGIN_GFBG, &point) < 0) {
			SAMPLE_PRT("set screen original show position failed!\n");
			return CVI_FAILURE;
		}
		// wait a vblank
		if (ioctl(info->fd, FBIOGET_VER_BLANK_GFBG, NULL) < 0) {
			SAMPLE_PRT("wait a vblank failed!\n");
			return CVI_FAILURE;
		}
	}
	while (point.x_pos > 0) {
		point.x_pos -= 2; /* 2 pos */
		if (ioctl(info->fd, FBIOPUT_SCREEN_ORIGIN_GFBG, &point) < 0) {
			SAMPLE_PRT("set screen original show position failed!\n");
			return CVI_FAILURE;
		}
		// wait a vblank
		if (ioctl(info->fd, FBIOGET_VER_BLANK_GFBG, NULL) < 0) {
			SAMPLE_PRT("wait a vblank failed!\n");
			return CVI_FAILURE;
		}
	}
	while (point.y_pos <= HEIGHT_1280P) {
		point.y_pos += 2; /* 2 pos */
		if (ioctl(info->fd, FBIOPUT_SCREEN_ORIGIN_GFBG, &point) < 0) {
			SAMPLE_PRT("set screen original show position failed!\n");
			return CVI_FAILURE;
		}
		// wait a vblank
		if (ioctl(info->fd, FBIOGET_VER_BLANK_GFBG, NULL) < 0) {
			SAMPLE_PRT("wait a vblank failed!\n");
			return CVI_FAILURE;
		}
	}
	while (point.y_pos > 0) {
		point.y_pos -= 2; /* 2 pos */
		if (ioctl(info->fd, FBIOPUT_SCREEN_ORIGIN_GFBG, &point) < 0) {
			SAMPLE_PRT("set screen original show position failed!\n");
			return CVI_FAILURE;
		}
		// wait a vblank
		if (ioctl(info->fd, FBIOGET_VER_BLANK_GFBG, NULL) < 0) {
			SAMPLE_PRT("wait a vblank failed!\n");
			return CVI_FAILURE;
		}
	}
	SAMPLE_PRT("move the cursor\n");
	sleep(1);
	return CVI_SUCCESS;
}

static CVI_S32 TDEFileToBuffer(const CVI_CHAR *filename, CVI_VOID *buffer, SIZE_S *pstSize)
{
	FILE *fp;
	CVI_U32 i;
	CVI_S32 s32Ret = CVI_SUCCESS, s32len;

	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file, %s, error\n", filename);
		return CVI_FAILURE;
	}

	for (i = 0; i < pstSize->u32Height; i++) {
		s32len = fread(buffer, pstSize->u32Width, 4, fp);
		if (s32len <= 0) {
			SAMPLE_PRT("fread data(%d) error\n", i);
			s32Ret = CVI_FAILURE;
			break;
		}
		buffer += pstSize->u32Width * 4;
	}

	fclose(fp);

	return s32Ret;
}

static CVI_S32 TDE_Rotate(TDE_ROTATE_ANGLE_E enRotateAngle, struct fb_fix_screeninfo *fix, CVI_U8 *show_screen)
{
	CVI_S32 s32Ret = CVI_FAILURE;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc;
	TDE_SURFACE_S stDst;
	CVI_U64 u64PhyAddrSrc = 0;
	CVI_VOID *pVirAddrSrc;
	CVI_S32 s32PixelSize = 4;
	SIZE_S stSizeIn = { HEIGHT_1280P, WIDTH_720P };
	CVI_CHAR *filename_in = TDE_DEFAULT_FILE_IN;

	/************************************************
	 Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		SAMPLE_PRT("CVI_TDE_BeginJob failed!\n");
		goto exit2;
	}
	stSrc.enColorFmt = PIXEL_FORMAT_ARGB_8888;
	stSrc.u32Width = ALIGN(stSizeIn.u32Width, TDE_ALIGN);
	stSrc.u32Height = ALIGN(stSizeIn.u32Height, TDE_ALIGN);
	stSrc.u32Stride = stSrc.u32Width * s32PixelSize;

	stDst.enColorFmt = PIXEL_FORMAT_ARGB_8888;
	stDst.u32Width = stSrc.u32Height;
	stDst.u32Height = stSrc.u32Width;
	stDst.u32Stride = stDst.u32Width * s32PixelSize;

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrSrc, &pVirAddrSrc, "TDE_src_buffer", stSrc.u32Stride * stSrc.u32Height);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	memset(pVirAddrSrc, 0, stSrc.u32Stride * stSrc.u32Height);

	s32Ret = TDEFileToBuffer(filename_in, pVirAddrSrc, &stSizeIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("TDEFileToBuffer failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	CVI_SYS_IonFlushCache(u64PhyAddrSrc, pVirAddrSrc, stSrc.u32Stride * stSrc.u32Height);

	stSrc.u64PhyAddr = u64PhyAddrSrc;
	stDst.u64PhyAddr = fix->smem_start;

	s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc, &stDst, enRotateAngle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_Rotate failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_TRUE, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_EndJob failed!\n");
		goto exit3;
	}

	SAMPLE_PRT("***JOB DONE***\n");

	CVI_SYS_IonInvalidateCache(stDst.u64PhyAddr, show_screen, stDst.u32Stride * stDst.u32Height);

exit3:
	if (u64PhyAddrSrc)
		CVI_SYS_IonFree(u64PhyAddrSrc, pVirAddrSrc);
exit2:
	CVI_TDE_Close();
exit1:
	return s32Ret;
}

static CVI_S32 sample_use_tde(pthread_gfbg_sample_info *info, struct fb_var_screeninfo *var, CVI_U8 *show_screen,
	struct fb_fix_screeninfo *fix)
{
	CVI_S32 s32Ret;

	s32Ret = TDE_Rotate(TDE_ROTATE_90, fix, show_screen);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("TDE_Rotate failed!\n");
		return CVI_FAILURE;
	}

	if (ioctl(info->fd, FBIOPAN_DISPLAY, var) < 0) {
		SAMPLE_PRT("FBIOPAN_DISPLAY failed!\n");
		return CVI_FAILURE;
	}

	sleep(1);

	return CVI_SUCCESS;
}

static CVI_S32 sample_show_bitmap(pthread_gfbg_sample_info *info, CVI_U8 *show_screen, CVI_U32 fix_screen_stride,
				  CVI_U32 byte_per_pixel)
{
	// CVI_S32 ret;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	// CVI_VOID *viraddr = CVI_NULL;

	if (ioctl(info->fd, FBIOGET_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("get variable screen info failed!\n");
		return CVI_FAILURE;
	}

	if (ioctl(info->fd, FBIOGET_FSCREENINFO, &fix) < 0) {
		SAMPLE_PRT("get fix screen info failed!\n");
		return CVI_FAILURE;
	}

	switch (info->ctrlkey) {
	/* 2 means none buffer and just for pan display. */
	case 2:
		(void)fix_screen_stride;
		(void)byte_per_pixel;
		break;
	case 3:
		if (info->use_tde) {
			/* use tde */
			if (sample_use_tde(info, &var, show_screen, &fix) != CVI_SUCCESS) {
				return CVI_FAILURE;
			}
		} else {
			/* move cursor */
			if (sample_move_cursor(info, &var, show_screen) != CVI_SUCCESS) {
				return CVI_FAILURE;
			}
		}
		break;
	default:
	return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sample_show_process(pthread_gfbg_sample_info *info)
{
	CVI_U8 *show_screen = CVI_NULL;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	CVI_U32 fix_screen_stride;
	CVI_BOOL show;

	if (ioctl(info->fd, FBIOGET_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("get variable screen info failed!\n");
		goto ERR1;
	}

	if (ioctl(info->fd, FBIOGET_FSCREENINFO, &fix) < 0) {
		SAMPLE_PRT("get fix screen info failed!\n");
		goto ERR1;
	}

	fix_screen_stride = fix.line_length;
	show_screen = mmap(CVI_NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, info->fd, 0);
	if (show_screen == MAP_FAILED) {
		SAMPLE_PRT("mmap framebuffer failed!\n");
		goto ERR1;
	}

	if (memset(show_screen, 0x0, fix.smem_len) != show_screen) {
		goto ERR2;
	}

	if (sample_time_to_play(info, show_screen, fix_screen_stride) != CVI_SUCCESS) {
		goto ERR2;
	}

	/* 8 for 1 byte */
	if (sample_show_bitmap(info, show_screen, fix_screen_stride, var.bits_per_pixel / 8) != CVI_SUCCESS) {
		goto ERR2;
	}

	munmap(show_screen, fix.smem_len);
	show = CVI_FALSE;
	if (ioctl(info->fd, FBIOPUT_SHOW_GFBG, &show) < 0) {
		SAMPLE_PRT("FBIOPUT_SHOW_GFBG failed!\n");
		close(info->fd);
		return CVI_FAILURE;
	}
	close(info->fd);
	info->fd = -1;
	return CVI_SUCCESS;

ERR2:
	munmap(show_screen, fix.smem_len);
	show_screen = CVI_NULL;
ERR1:
	close(info->fd);
	info->fd = -1;
	return CVI_FAILURE;
}

static CVI_VOID *sample_gfbg_pandisplay(CVI_VOID *data)
{
	pthread_gfbg_sample_info *info = CVI_NULL;
	CVI_CHAR thdname[64]; /* 64 for char length */
	CVI_CHAR file[FILE_LENGTH_MAX] = {0};

	if (data == CVI_NULL) {
		return CVI_NULL;
	}

	info = (pthread_gfbg_sample_info *)data;
	if (snprintf(thdname, 17, "GFBG%d_pandisplay", info->layer) == -1) { /* 17 for char length */
		SAMPLE_PRT("%s:%d:snprintf_s failed.\n", __func__, __LINE__);
		return CVI_NULL;
	}
	prctl(PR_SET_NAME, thdname, 0, 0, 0);

	if (sample_get_file_name(info, file) != CVI_SUCCESS) {
		return CVI_NULL;
	}

	if (sample_init_frame_buffer(info, file) != CVI_SUCCESS) {
		return CVI_NULL;
	}

	if (sample_init_var(info) != CVI_SUCCESS) {
		close(info->fd);
		info->fd = -1;
		return CVI_NULL;
	}

	/* map the physical video memory for user use */
	if (sample_show_process(info) != CVI_SUCCESS) {
		return CVI_NULL;
	}

	SAMPLE_PRT("[end]\n");

	return CVI_NULL;
}


static CVI_S32 sample_init_frame_buffer_ex(pthread_gfbg_sample_info *info, const char *input_file)
{
	cvi_fb_point point = {0, 0};

	/* step 1. open framebuffer device overlay 0 */
	info->fd = open(input_file, O_RDWR, 0);
	if (info->fd < 0) {
		SAMPLE_PRT("open %s failed!\n", input_file);
		return CVI_FAILURE;
	}

	if (ioctl(info->fd, FBIOPUT_SCREEN_ORIGIN_GFBG, &point) < 0) {
		SAMPLE_PRT("set screen original show position failed!\n");
		close(info->fd);
		info->fd = -1;
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sample_init_var_ex(pthread_gfbg_sample_info *info)
{
	struct fb_var_screeninfo var;

	if (ioctl(info->fd, FBIOGET_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("get variable screen info failed!\n");
		return CVI_FAILURE;
	}

	switch (info->color_format) {
	case CVI_FB_FORMAT_ARGB8888:
		var.transp = g_a32;
		var.red = g_r32;
		var.green = g_g32;
		var.blue = g_b32;
		var.bits_per_pixel = 32; /* 32 for 4 byte */
		g_osd_color_fmt = OSD_COLOR_FMT_RGB8888;
		break;
	default:
		var.transp = g_a16;
		var.red = g_r16;
		var.green = g_g16;
		var.blue = g_b16;
		var.bits_per_pixel = 16; /* 16 for 2 byte */
		break;
	}

	var.xres_virtual = WIDTH_720P;
	var.yres_virtual = HEIGHT_1280P;
	var.xres = WIDTH_720P;
	var.yres = HEIGHT_1280P;
	var.activate = FB_ACTIVATE_NOW;

	/* step 5. set the variable screen information */
	if (ioctl(info->fd, FBIOPUT_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("put variable screen info failed!\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sample_init_layer_info(pthread_gfbg_sample_info *info)
{
	cvi_fb_layer_info layer_info = {0};

	switch (info->ctrlkey) {
	case 0: /* 0 case */
		layer_info.buf_mode = CVI_FB_LAYER_BUF_ONE;
		layer_info.mask = CVI_FB_LAYER_MASK_BUF_MODE;
		break;
	case 1: /* 1 case */
		layer_info.buf_mode = CVI_FB_LAYER_BUF_DOUBLE;
		layer_info.mask = CVI_FB_LAYER_MASK_BUF_MODE;
		break;
	default:
		layer_info.buf_mode = CVI_FB_LAYER_BUF_NONE;
		layer_info.mask = CVI_FB_LAYER_MASK_BUF_MODE;
		break;
	}

	if (ioctl(info->fd, FBIOPUT_LAYER_INFO, &layer_info) < 0) {
		SAMPLE_PRT("PUT_LAYER_INFO failed!\n");
		close(info->fd);
		info->fd = -1;
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sample_init_canvas(pthread_gfbg_sample_info *info, cvi_fb_buf *canvas_buf, CVI_VOID **buf,
				  CVI_VOID **viraddr)
{
	CVI_U32 byte_per_pixel;
	cvi_fb_color_format clr_fmt;

	switch (info->color_format) {
	case CVI_FB_FORMAT_ARGB8888:
		byte_per_pixel = 4; /* 4 bytes */
		clr_fmt = CVI_FB_FORMAT_ARGB8888;
		break;
	default:
		byte_per_pixel = 2; /* 2 bytes */
		clr_fmt = CVI_FB_FORMAT_ARGB1555;
		break;
	}

	if (CVI_SYS_IonAlloc(&g_canvas_addr, buf, "user_canvas", WIDTH_720P * HEIGHT_1280P *
		(byte_per_pixel)) == CVI_FAILURE) {
		SAMPLE_PRT("allocate memory (max_w*max_h*%d bytes) failed\n", byte_per_pixel);
		close(info->fd);
		info->fd = -1;
		return CVI_FAILURE;
	}

	canvas_buf->canvas.phys_addr = g_canvas_addr;
	canvas_buf->canvas.height = HEIGHT_1280P;
	canvas_buf->canvas.width = WIDTH_720P;
	canvas_buf->canvas.pitch = WIDTH_720P * (byte_per_pixel);
	canvas_buf->canvas.format = clr_fmt;
	if (memset(*buf, 0x00, canvas_buf->canvas.pitch *
		canvas_buf->canvas.height) != *buf) {
		SAMPLE_PRT("%s:%d:memset failed\n", __func__, __LINE__);
		CVI_SYS_IonFree(g_canvas_addr, *buf);
		g_canvas_addr = 0;
		close(info->fd);
		return CVI_FAILURE;
	}

	/* change bmp */
	if (CVI_SYS_IonAlloc(&g_phyaddr, viraddr, "user_bmp", SAMPLE_IMAGE_WIDTH * SAMPLE_IMAGE_HEIGHT *
		byte_per_pixel) == CVI_FAILURE) {
		SAMPLE_PRT("allocate memory (max_w*max_h*%d bytes) failed\n", byte_per_pixel);
		CVI_SYS_IonFree(g_canvas_addr, *buf);
		g_canvas_addr = 0;
		close(info->fd);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sample_draw_line_by_cpu_ex(pthread_gfbg_sample_info *info, cvi_fb_buf *canvas_buf, CVI_VOID *buf)
{
	CVI_U32 x, y, color;
	CVI_S32 ret;
	cvi_fb_color_format clr_fmt;

	switch (info->color_format) {
	case CVI_FB_FORMAT_ARGB8888:
		clr_fmt = CVI_FB_FORMAT_ARGB8888;
		color = GFBG_RED_8888;
		break;
	default:
		clr_fmt = CVI_FB_FORMAT_ARGB1555;
		color = GFBG_RED_1555;
		break;
	}

	for (y = (HEIGHT_1280P / 2 - 2); y < (HEIGHT_1280P / 2 + 2); y++) { /* 2 alg data */
		for (x = 0; x < WIDTH_720P; x++) {
			if (clr_fmt == CVI_FB_FORMAT_ARGB8888) {
				*((CVI_U32 *)buf + y * WIDTH_720P + x) = color;
			} else {
				*((CVI_U16 *)buf + y * WIDTH_720P + x) = color;
			}
		}
	}

	for (y = 0; y < HEIGHT_1280P; y++) {
		for (x = (WIDTH_720P / 2 - 2); x < (WIDTH_720P / 2 + 2); x++) { /* 2 alg data */
			if (clr_fmt == CVI_FB_FORMAT_ARGB8888) {
				*((CVI_U32 *)buf + y * WIDTH_720P + x) = color;
			} else {
				*((CVI_U16 *)buf + y * WIDTH_720P + x) = color;
			}
		}
	}

	canvas_buf->update_rect.x = 0;
	canvas_buf->update_rect.y = 0;
	canvas_buf->update_rect.width = WIDTH_720P;
	canvas_buf->update_rect.height = HEIGHT_1280P;
	ret = ioctl(info->fd, FBIO_REFRESH, canvas_buf);
	if (ret < 0) {
		SAMPLE_PRT("REFRESH failed!\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sample_time_to_play_ex(pthread_gfbg_sample_info *info, cvi_fb_buf *canvas_buf, CVI_VOID *buf,
				      CVI_VOID *viraddr)
{
	CVI_S32 ret;
	CVI_U32 i;

	SAMPLE_PRT("[begin]\n");
	SAMPLE_PRT("expected:two red line!\n");
	/* time to play */
	for (i = 0; i < SAMPLE_IMAGE_NUM; i++) {
		/* draw two lines by cpu */
		if (sample_draw_line_by_cpu_ex(info, canvas_buf, buf) != CVI_SUCCESS) {
			return CVI_FAILURE;
		}
		sleep(1); /* 1 second */

		(void)viraddr;

		canvas_buf->update_rect.x = 0;
		canvas_buf->update_rect.y = 0;
		canvas_buf->update_rect.width = WIDTH_720P;
		canvas_buf->update_rect.height = HEIGHT_1280P;
		ret = ioctl(info->fd, FBIO_REFRESH, canvas_buf);
		if (ret < 0) {
			SAMPLE_PRT("REFRESH failed!\n");
			return CVI_FAILURE;
		}
		sleep(1); /* 1 second */
	}
	return CVI_SUCCESS;
}

static CVI_VOID *sample_gfbg_refresh(CVI_VOID *data)
{
	CVI_CHAR file[FILE_LENGTH_MAX] = {0};
	cvi_fb_buf canvas_buf;
	CVI_VOID *buf = CVI_NULL;
	CVI_VOID *viraddr = CVI_NULL;
	pthread_gfbg_sample_info *info = CVI_NULL;

	prctl(PR_SET_NAME, "GFBG_REFRESH", 0, 0, 0);
	if (data == CVI_NULL) {
		return CVI_NULL;
	}
	info = (pthread_gfbg_sample_info *)data;

	if (sample_get_file_name(info, file) != CVI_SUCCESS) {
		return CVI_NULL;
	}

	if (sample_init_frame_buffer_ex(info, file) != CVI_SUCCESS) {
		return CVI_NULL;
	}

	if (sample_init_var_ex(info) != CVI_SUCCESS) {
		close(info->fd);
		info->fd = -1;
		return CVI_NULL;
	}

	if (sample_init_layer_info(info) != CVI_SUCCESS) {
		return CVI_NULL;
	}

	if (sample_init_canvas(info, &canvas_buf, &buf, &viraddr) != CVI_SUCCESS) {
		return CVI_NULL;
	}

	if (sample_time_to_play_ex(info, &canvas_buf, buf, viraddr) != CVI_SUCCESS) {
		goto ERR;
	}
	SAMPLE_PRT("[end]\n");
ERR:
	CVI_SYS_IonFree(g_phyaddr, viraddr);
	g_phyaddr = 0;
	CVI_SYS_IonFree(g_canvas_addr, buf);
	g_canvas_addr = 0;
	close(info->fd);
	return CVI_NULL;
}

static CVI_S32 sample_cmap_init(pthread_gfbg_sample_info *info)
{
	struct fb_cmap cmap;

	cmap.start = 0;
	cmap.len = g_colors_len;
	cmap.red = g_cmap_red;
	cmap.green = g_cmap_green;
	cmap.blue = g_cmap_blue;
	cmap.transp = g_cmap_alpha;

	if (ioctl(info->fd, FBIOPUTCMAP, &cmap) < 0) {
		SAMPLE_PRT("put cmap info failed!\n");
		close(info->fd);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

static CVI_S32 sample_get_fix_and_mmap(pthread_gfbg_sample_info *info, struct fb_var_screeninfo *var,
				       struct fb_fix_screeninfo *fix, CVI_VOID **viraddr)
{
	if (info == CVI_NULL || var == CVI_NULL || viraddr == CVI_NULL) {
		return CVI_FAILURE;
	}

	if (ioctl(info->fd, FBIOGET_FSCREENINFO, fix) < 0) {
		SAMPLE_PRT("get fix screen info failed!\n");
		return CVI_FAILURE;
	}

	if (ioctl(info->fd, FBIOGET_VSCREENINFO, var) < 0) {
		SAMPLE_PRT("get variable screen info failed!\n");
		return CVI_FAILURE;
	}

	*viraddr = mmap(CVI_NULL, (info->color_format == CVI_FB_FORMAT_LUT_256) ?
			(SAMPLE_IMAGE_WIDTH * SAMPLE_IMAGE_HEIGHT):(SAMPLE_IMAGE_WIDTH * SAMPLE_IMAGE_HEIGHT / 2),
			PROT_READ | PROT_WRITE, MAP_SHARED, info->fd, 0);
	if (*viraddr == MAP_FAILED) {
		SAMPLE_PRT("mmap failed!\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 sample_gfbg_load_bmp_clut(const char *filename, CVI_VOID *viraddr)
{
	OSD_SURFACE_S Surface;
	OSD_BITMAPFILEHEADER bmpFileHeader;
	OSD_BITMAPINFO bmpInfo;
	CVI_S32 Bpp;
	CVI_U32 i;

	if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
		SAMPLE_PRT("GetBmpInfo err!\n");
		return CVI_FAILURE;
	}

	Bpp = bmpInfo.bmiHeader.biBitCount / 8;
	if (Bpp == 1) {
		if (bmpInfo.bmiHeader.biClrUsed == 0)
			g_colors_len = 1 << bmpInfo.bmiHeader.biBitCount;
		else
			g_colors_len = bmpInfo.bmiHeader.biClrUsed;

		SAMPLE_PRT("load bmp clut length %d.\n", g_colors_len);

		if (g_colors_len > CMAP_LENGTH_MAX) {
			SAMPLE_PRT("Number of indexed palette is over 256.");
			return CVI_FAILURE;
		}

		/* Create the palette */
		for (i = 0; i < g_colors_len; i++) {
			//rgb888 bmp so we set alpha to 0xff
			// g_cmap_alpha[i] = bmpInfo.bmiColors[i].rgbReserved;
			g_cmap_alpha[i] = 0xff;
			g_cmap_red[i] = bmpInfo.bmiColors[i].rgbRed;
			g_cmap_green[i] = bmpInfo.bmiColors[i].rgbGreen;
			g_cmap_blue[i] = bmpInfo.bmiColors[i].rgbBlue;
		}
	}

	if (Bpp == 1) {
		Surface.enColorFmt = OSD_COLOR_FMT_8BIT_MODE;
		CreateSurfaceByBitMap(filename, &Surface, (CVI_U8 *)(viraddr));
	}

	return CVI_SUCCESS;
}

static CVI_S32 sample_init_var_clut(pthread_gfbg_sample_info *info)
{
	struct fb_var_screeninfo var;

	if (ioctl(info->fd, FBIOGET_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("get variable screen info failed!\n");
		return CVI_FAILURE;
	}

	switch (info->color_format) {
	case CVI_FB_FORMAT_LUT_256:
		var.transp = g_a8;
		var.red    = g_r8;
		var.green  = g_g8;
		var.blue   = g_b8;
		var.bits_per_pixel = 8; /* 8 bits per pixel */
		break;
	case CVI_FB_FORMAT_LUT_16:
		var.transp = g_a4;
		var.red    = g_r4;
		var.green  = g_g4;
		var.blue   = g_b4;
		var.bits_per_pixel = 4; /* 4 bits per pixel */
		break;
	default:
		break;
	}

	var.xres_virtual = SAMPLE_IMAGE_WIDTH;
	var.yres_virtual = SAMPLE_IMAGE_HEIGHT; /* 1 for 1buf */
	var.xres = SAMPLE_IMAGE_WIDTH;
	var.yres = SAMPLE_IMAGE_HEIGHT;
	var.activate = FB_ACTIVATE_NOW;

	if (ioctl(info->fd, FBIOPUT_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("put variable screen info failed!\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_VOID *sample_gfbg_clut(void *data)
{
	pthread_gfbg_sample_info *info = CVI_NULL;
	CVI_CHAR thdname[64]; /* 64 for char length */
	CVI_CHAR file[FILE_LENGTH_MAX] = {0};
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	CVI_VOID *viraddr = CVI_NULL;
	CVI_BOOL show;

	if (data == CVI_NULL) {
		return CVI_NULL;
	}

	info = (pthread_gfbg_sample_info *)data;
	if (snprintf(thdname, 17, "GFBG%d_clut", info->layer) == -1) { /* 17 for char length */
		SAMPLE_PRT("%s:%d:snprintf_s failed.\n", __func__, __LINE__);
		return CVI_NULL;
	}
	prctl(PR_SET_NAME, thdname, 0, 0, 0);

	if (sample_get_file_name(info, file) != CVI_SUCCESS) {
		return CVI_NULL;
	}

	if (sample_init_frame_buffer(info, file) != CVI_SUCCESS) {
		return CVI_NULL;
	}

	if (sample_init_var_clut(info) != CVI_SUCCESS) {
		goto ERR1;
	}

	if (sample_get_fix_and_mmap(info, &var, &fix, &viraddr) != CVI_SUCCESS) {
		goto ERR1;
	}

	if (sample_gfbg_load_bmp_clut(SAMPLE_IMAGE2_PATH, viraddr) != CVI_SUCCESS) {
		goto ERR2;
	}

	if (sample_cmap_init(info) != CVI_SUCCESS) {
		goto ERR2;
	}

	show = CVI_TRUE;
	if (ioctl(info->fd, FBIOPUT_SHOW_GFBG, &show) < 0) {
		SAMPLE_PRT("FBIOPUT_SHOW_GFBG failed!\n");
		goto ERR2;
	}

	sleep(4);

ERR2:
	munmap(viraddr, (info->color_format == CVI_FB_FORMAT_LUT_256) ?
	       (SAMPLE_IMAGE_WIDTH * SAMPLE_IMAGE_HEIGHT) :
	       (SAMPLE_IMAGE_WIDTH * SAMPLE_IMAGE_HEIGHT) / 2);
ERR1:
	close(info->fd);
	info->fd = -1;
	SAMPLE_PRT("[end]\n");
	return CVI_NULL;
}

static CVI_S32 sample_gfbg_standard_mode(CVI_S32 index, CVI_BOOL use_tde, CVI_BOOL compress)
{
	CVI_S32 ret = CVI_SUCCESS;
	pthread_gfbg_sample_info info0 = {0};

	info0.layer = GRAPHICS_LAYER_G0;
	info0.fd = -1;
	info0.ctrlkey = (index == 0) ? 2 : 3; /* 2 none buffer pan display / 3 cursor or tde case*/
	info0.compress = compress; /* compress opened or not */
	info0.color_format = CVI_FB_FORMAT_ARGB8888;
	info0.use_tde = use_tde;

	if (pthread_create(&g_gfbg_thread, 0, sample_gfbg_pandisplay, (CVI_VOID *)(&info0)) != 0) {
		SAMPLE_PRT("start gfbg thread0 failed!\n");
	}

	sample_gfbg_to_exit();

	return ret;
}

static CVI_S32 sample_gfbg_none_buf_mode(void)
{
	CVI_S32 ret = CVI_SUCCESS;
	pthread_gfbg_sample_info info0;

	info0.layer = GRAPHICS_LAYER_G0;
	info0.fd = -1;
	info0.ctrlkey = 3; /* 3: none buffer */
	info0.compress = CVI_FALSE;
	info0.color_format = CVI_FB_FORMAT_ARGB1555;
	if (pthread_create(&g_gfbg_thread, 0, sample_gfbg_refresh, (void *)(&info0)) != 0) {
		SAMPLE_PRT("start gfbg thread failed!\n");
	}

	sample_gfbg_to_exit();

	return ret;
}

static CVI_S32 sample_gfbg_clut_mode(void)
{
	CVI_S32 ret = CVI_SUCCESS;
	pthread_gfbg_sample_info info0;

	info0.layer =  GRAPHICS_LAYER_G0;
	info0.fd  = -1;
	info0.ctrlkey = 2; /* 2 0buffer pan display */
	info0.compress = CVI_FALSE;
	info0.color_format = CVI_FB_FORMAT_LUT_256;
	if (pthread_create(&g_gfbg_thread, 0, sample_gfbg_clut, (void *)(&info0)) != 0) {
		SAMPLE_PRT("start gfbg thread failed!\n");
	}

	sample_gfbg_to_exit();

	return ret;
}

static CVI_S32 sample_choose_the_case(char **argv)
{
	CVI_S32 ret = CVI_FAILURE;
	CVI_CHAR ch;

	ch = *(argv[1]);

	if (ch == '0') {
		SAMPLE_PRT("\nindex 0 selected.\n");
		ret = sample_gfbg_standard_mode(0, 0, 0);
	} else if (ch == '1') {
		SAMPLE_PRT("\nindex 1 selected.\n");
		ret = sample_gfbg_standard_mode(1, 0, 0);
	} else if (ch == '2') {
		SAMPLE_PRT("\nindex 2 selected.\n");
		ret = sample_gfbg_none_buf_mode();
	} else if (ch == '3') {
		SAMPLE_PRT("\nindex 3 selected.\n");
		ret = sample_gfbg_clut_mode();
	} else if (ch == '4') {
		SAMPLE_PRT("\nindex 4 selected.\n");
		ret = sample_gfbg_standard_mode(1, 1, 0);
	} else if (ch == '5') {
		SAMPLE_PRT("\nindex 5 selected.\n");
		ret = sample_gfbg_standard_mode(0, 0, 1);
	} else {
		SAMPLE_PRT("index invalid! please try again.\n");
		sample_gfbg_usage1(argv[0]);
		return CVI_FAILURE;
	}

	if (ret == CVI_SUCCESS) {
		SAMPLE_PRT("program exit normally!\n");
	} else {
		SAMPLE_PRT("program exit abnormally!\n");
	}

	return ret;
}

int main(int argc, char *argv[])
{
	CVI_S32 ret;
	VO_DEV VoDev = 0;

	if ((argc != 2) || (strlen(argv[1]) != 1)) {
		SAMPLE_PRT("index invalid! please try again.\n");
		sample_gfbg_usage1(argv[0]);
		return CVI_FAILURE;
	}

	sample_sys_signal(&sample_gfbg_handle_sig);

	/*enable vo*/
	ret = vo_sys_init();
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("vo_sys_init failed\n");
		return CVI_FAILURE;
	}

	ret = vo_init_by_fmt(vo_file.enPixelFormat, VoDev);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("vo_init_by_fmt failed\n");
		goto exit1;
	}

	ret = vo_send_frame(&vo_file, VoDev);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("vo_ut_send_frame failed\n");
		goto exit2;
	}

	ret = sample_choose_the_case(argv);
	if (ret != CVI_SUCCESS) {
		goto exit2;
	}

exit2:
	ret = vo_deinit();
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("vo_deinit failed\n");
	}
exit1:
	ret = vo_sys_deinit();
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("vo_sys_deinit failed\n");
		return CVI_FAILURE;
	}

	return ret;
}
