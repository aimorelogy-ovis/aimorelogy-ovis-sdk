/* GStreamer
 * Copyright (C) 2020 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstcviivebackground
 *
 * The cviivebackground element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! cviivebackground ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include "gstcviivebackground.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cvi_tracer.h>

GST_DEBUG_CATEGORY_STATIC (gst_cvi_ive_background_debug_category);
#define GST_CAT_DEFAULT gst_cvi_ive_background_debug_category

/* prototypes */


static void gst_cvi_ive_background_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_cvi_ive_background_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_cvi_ive_background_dispose (GObject * object);
static void gst_cvi_ive_background_finalize (GObject * object);

static gboolean gst_cvi_ive_background_start (GstBaseTransform * trans);
static gboolean gst_cvi_ive_background_stop (GstBaseTransform * trans);
static gboolean gst_cvi_ive_background_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info);
static GstFlowReturn gst_cvi_ive_background_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe);
static GstFlowReturn gst_cvi_ive_background_transform_frame_ip (GstVideoFilter * filter,
    GstVideoFrame * frame);

enum
{
  PROP_0
};

/* pad templates */

/* FIXME: add/remove formats you can handle */
#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE("{ I420 }")

/* FIXME: add/remove formats you can handle */
#define VIDEO_SINK_CAPS \
    GST_VIDEO_CAPS_MAKE("{ I420 }")

static GstCaps *gst_cvi_ive_background_transform_caps(GstBaseTransform * btrans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstCaps *result;
  GstStructure *st;
  GstCapsFeatures *f;
  gint i, n;
  printf("[cviivebackground] transform caps.\n");
  GST_DEBUG_OBJECT (btrans, "Transformed %" GST_PTR_FORMAT, caps);
  GST_DEBUG_OBJECT (btrans, "Filter %" GST_PTR_FORMAT, filter);
  result = gst_caps_new_empty();
  n = gst_caps_get_size(caps);
  // printf("Size of Caps: %d\n", n);

  for (i = 0; i < n; i++)
  {
    st = gst_caps_get_structure(caps, i);
    f = gst_caps_get_features(caps, i);

    /* If this is already expressed by the existing caps
     * skip this structure */
    if (i > 0 && gst_caps_is_subset_structure_full (result, st, f))
      continue;

    st = gst_structure_copy (st);
    /* Only remove format info for the cases when we can actually convert */
    if (!gst_caps_features_is_any (f)
        && gst_caps_features_is_equal (f,
            GST_CAPS_FEATURES_MEMORY_SYSTEM_MEMORY)) {
      gst_structure_set (st,
        "width",  GST_TYPE_INT_RANGE, 1, 2048,
        "height", GST_TYPE_INT_RANGE, 1, 2048, NULL);

      /* if pixel aspect ratio, make a range of it */
      if (gst_structure_has_field (st, "pixel-aspect-ratio")) {
        gst_structure_set (st, "pixel-aspect-ratio",
        GST_TYPE_FRACTION_RANGE, 1, 2048, 2048, 1, NULL);
      }
      gst_structure_remove_fields (st, "format", "colorimetry", "chroma-site", NULL);
    }

    gst_caps_append_structure_full (result, st, gst_caps_features_copy (f));
  }
  GST_DEBUG_OBJECT (btrans, "Into %" GST_PTR_FORMAT, result);
  // GST_DEBUG_OBJECT (btrans, "Transformed %" GST_PTR_FORMAT "Into %"
  //     GST_PTR_FORMAT, caps, result);
  return result;
}

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstCviIveBackground, gst_cvi_ive_background, GST_TYPE_VIDEO_FILTER,
  GST_DEBUG_CATEGORY_INIT (gst_cvi_ive_background_debug_category, "cviivebackground", 0,
  "debug category for cviivebackground element"));

static void
gst_cvi_ive_background_class_init (GstCviIveBackgroundClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SINK_CAPS)));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_cvi_ive_background_set_property;
  gobject_class->get_property = gst_cvi_ive_background_get_property;
  gobject_class->dispose = gst_cvi_ive_background_dispose;
  gobject_class->finalize = gst_cvi_ive_background_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_cvi_ive_background_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_cvi_ive_background_stop);
  base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_cvi_ive_background_transform_caps);
  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_cvi_ive_background_set_info);
  video_filter_class->transform_frame = GST_DEBUG_FUNCPTR (gst_cvi_ive_background_transform_frame);
  video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR (gst_cvi_ive_background_transform_frame_ip);

}

static void
gst_cvi_ive_background_init (GstCviIveBackground *cviivebackground)
{
  CVI_SYS_TraceBegin("cvi_ive_init");
  cviivebackground->bk_handle = malloc(sizeof(GST_CVI_IVE_BACKGROUND_HANDLE_S));
  cviivebackground->bk_handle->handle = CVI_IVE_CreateHandle();
  cviivebackground->bk_handle->count = 0;
  cviivebackground->bk_handle->i_count = 0;
  CVI_SYS_TraceEnd();
}

void
gst_cvi_ive_background_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstCviIveBackground *cviivebackground = GST_CVI_IVE_BACKGROUND (object);

  GST_DEBUG_OBJECT (cviivebackground, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_cvi_ive_background_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstCviIveBackground *cviivebackground = GST_CVI_IVE_BACKGROUND (object);

  GST_DEBUG_OBJECT (cviivebackground, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_cvi_ive_background_dispose (GObject * object)
{
  GstCviIveBackground *cviivebackground = GST_CVI_IVE_BACKGROUND (object);

  GST_DEBUG_OBJECT (cviivebackground, "dispose");

  /* clean up as possible.  may be called multiple times */
  if (cviivebackground->bk_handle != NULL) {
    GST_CVI_IVE_BACKGROUND_HANDLE_S *bk_handle = cviivebackground->bk_handle;
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->src[0]);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->src[1]);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->tmp);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->andframe[0]);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->andframe[1]);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->dst);
    CVI_IVE_DestroyHandle(bk_handle->handle);

    free(cviivebackground->bk_handle);
    cviivebackground->bk_handle = NULL;
  }

  G_OBJECT_CLASS (gst_cvi_ive_background_parent_class)->dispose (object);
}

void
gst_cvi_ive_background_finalize (GObject * object)
{
  GstCviIveBackground *cviivebackground = GST_CVI_IVE_BACKGROUND (object);

  GST_DEBUG_OBJECT (cviivebackground, "finalize");

  /* clean up object here */
  if (cviivebackground->bk_handle != NULL) {
    GST_CVI_IVE_BACKGROUND_HANDLE_S *bk_handle = cviivebackground->bk_handle;
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->src[0]);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->src[1]);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->tmp);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->andframe[0]);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->andframe[1]);
    CVI_SYS_FreeI(bk_handle->handle, &bk_handle->dst);
    CVI_IVE_DestroyHandle(bk_handle->handle);

    free(cviivebackground->bk_handle);
    cviivebackground->bk_handle = NULL;
  }

  G_OBJECT_CLASS (gst_cvi_ive_background_parent_class)->finalize (object);
}

static gboolean
gst_cvi_ive_background_start (GstBaseTransform * trans)
{
  GstCviIveBackground *cviivebackground = GST_CVI_IVE_BACKGROUND (trans);

  GST_DEBUG_OBJECT (cviivebackground, "start");

  return TRUE;
}

static gboolean
gst_cvi_ive_background_stop (GstBaseTransform * trans)
{
  GstCviIveBackground *cviivebackground = GST_CVI_IVE_BACKGROUND (trans);

  GST_DEBUG_OBJECT (cviivebackground, "stop");

  return TRUE;
}

static gboolean
gst_cvi_ive_background_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstCviIveBackground *cviivebackground = GST_CVI_IVE_BACKGROUND (filter);

  GST_CVI_IVE_BACKGROUND_HANDLE_S *bk_handle = cviivebackground->bk_handle;
  gint src_width = GST_VIDEO_INFO_WIDTH(in_info);
  gint src_height = GST_VIDEO_INFO_HEIGHT(in_info);
  CVI_SYS_TraceBegin("Init buffer");
  printf("src w, h %d, %d\n", src_width, src_height);
  CVI_IVE_CreateImage(bk_handle->handle, &bk_handle->src[0], IVE_IMAGE_TYPE_U8C1, src_width, src_height);
  CVI_IVE_CreateImage(bk_handle->handle, &bk_handle->src[1], IVE_IMAGE_TYPE_U8C1, src_width, src_height);
  CVI_IVE_CreateImage(bk_handle->handle, &bk_handle->tmp, IVE_IMAGE_TYPE_U8C1, src_width, src_height);
  CVI_IVE_CreateImage(bk_handle->handle, &bk_handle->andframe[0], IVE_IMAGE_TYPE_U8C1, src_width, src_height);
  CVI_IVE_CreateImage(bk_handle->handle, &bk_handle->andframe[1], IVE_IMAGE_TYPE_U8C1, src_width, src_height);
  CVI_IVE_CreateImage(bk_handle->handle, &bk_handle->dst, IVE_IMAGE_TYPE_U8C1, src_width, src_height);
  CVI_SYS_TraceEnd();
  GST_DEBUG_OBJECT (cviivebackground, "set_info");
  return TRUE;
}

#include <time.h>
static void run_background_subtraction(GstCviIveBackground *cviivebackground, guchar *in_buf,
                                       guchar *out_buf) {
  CVI_SYS_TraceBegin("bk_sub");
  GST_CVI_IVE_BACKGROUND_HANDLE_S *bk_handle = cviivebackground->bk_handle;
  guint total_sz = bk_handle->src[bk_handle->count].u16Height *
                   bk_handle->src[bk_handle->count].u16Stride[0];
  memcpy(bk_handle->src[bk_handle->count].pu8VirAddr[0], in_buf, total_sz);
  CVI_IVE_BufFlush(bk_handle->handle, &bk_handle->src[bk_handle->count]);
  if (bk_handle->i_count > 0) {
    // Sub - threshold - dilate
    IVE_SUB_CTRL_S iveSubCtrl;
    iveSubCtrl.enMode = IVE_SUB_MODE_ABS;
    CVI_IVE_Sub(bk_handle->handle, &bk_handle->src[bk_handle->count],
                &bk_handle->src[1 - bk_handle->count], &bk_handle->tmp, &iveSubCtrl, 0);

    IVE_THRESH_CTRL_S iveTshCtrl;
    iveTshCtrl.enMode = IVE_THRESH_MODE_BINARY;
    iveTshCtrl.u8MinVal = 0;
    iveTshCtrl.u8MaxVal = 255;
    iveTshCtrl.u8LowThr = 35;
    CVI_IVE_Thresh(bk_handle->handle, &bk_handle->tmp, &bk_handle->tmp, &iveTshCtrl, 0);

    IVE_DILATE_CTRL_S stDilateCtrl;
    memset(stDilateCtrl.au8Mask, 1, 25);
    CVI_IVE_Dilate(bk_handle->handle, &bk_handle->tmp, &bk_handle->andframe[bk_handle->count],
                   &stDilateCtrl, 0);

    if (bk_handle->i_count > 1) {
      // And two dilated images
      CVI_IVE_And(bk_handle->handle, &bk_handle->andframe[bk_handle->count],
                  &bk_handle->andframe[1 - bk_handle->count], &bk_handle->dst, 0);
      CVI_IVE_And(bk_handle->handle, &bk_handle->src[bk_handle->count], &bk_handle->dst,
                  &bk_handle->dst, 0);
      CVI_IVE_BufRequest(bk_handle->handle, &bk_handle->dst);
      memcpy(out_buf, bk_handle->dst.pu8VirAddr[0], total_sz);
    }
  }
  bk_handle->count = 1 - bk_handle->count;

  if ( bk_handle->i_count < 2) {
    bk_handle->i_count++;
  }
  CVI_SYS_TraceEnd();
}

/* transform */
static GstFlowReturn
gst_cvi_ive_background_transform_frame (GstVideoFilter * filter, GstVideoFrame * inframe,
    GstVideoFrame * outframe)
{
  int height_in = GST_VIDEO_FRAME_HEIGHT(inframe);
  int height_out = GST_VIDEO_FRAME_HEIGHT(outframe);
  GstCviIveBackground *cviivebackground = GST_CVI_IVE_BACKGROUND(filter);
  if (height_in == height_out) {
    guchar *in_buf = GST_VIDEO_FRAME_PLANE_DATA(inframe, 0);
    guchar *out_buf = GST_VIDEO_FRAME_PLANE_DATA(outframe, 0);
    for (int i = 1; i < 3; i++) {
      guchar *in_frame = GST_VIDEO_FRAME_PLANE_DATA(inframe, i);
      guchar *out_frame = GST_VIDEO_FRAME_PLANE_DATA(outframe, i);
      int width_s = GST_VIDEO_FRAME_PLANE_STRIDE(outframe, i);
      int height = GST_VIDEO_FRAME_COMP_HEIGHT(outframe, i);
      memcpy(out_frame, in_frame, width_s * height);
    }
    run_background_subtraction(cviivebackground, in_buf, out_buf);
  } else if (height_in * 2 == height_out) {
    guchar *in_buf = GST_VIDEO_FRAME_PLANE_DATA(inframe, 0);
    guchar *out_buf = GST_VIDEO_FRAME_PLANE_DATA(outframe, 0);
    for (int i = 1; i < 3; i++) {
      guchar *in_frame = GST_VIDEO_FRAME_PLANE_DATA(inframe, i);
      guchar *out_frame = GST_VIDEO_FRAME_PLANE_DATA(outframe, i);
      int width_s = GST_VIDEO_FRAME_PLANE_STRIDE(outframe, i);
      int height = GST_VIDEO_FRAME_COMP_HEIGHT(inframe, i);
      memcpy(out_frame, in_frame, width_s * height);
    }
    run_background_subtraction(cviivebackground, in_buf, out_buf);
    for (int i = 0; i < 3; i++) {
      guchar *in_frame = GST_VIDEO_FRAME_PLANE_DATA (inframe, i);
      guchar *out_frame = GST_VIDEO_FRAME_PLANE_DATA (outframe, i);
      int width_s = GST_VIDEO_FRAME_PLANE_STRIDE(outframe, i);
      int height = GST_VIDEO_FRAME_COMP_HEIGHT(inframe, i);
      out_frame += width_s * height;
      memcpy(out_frame, in_frame, width_s * height);
    }
  }


  GST_DEBUG_OBJECT (cviivebackground, "transform_frame");

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_cvi_ive_background_transform_frame_ip (GstVideoFilter * filter, GstVideoFrame * frame)
{
  GstCviIveBackground *cviivebackground = GST_CVI_IVE_BACKGROUND (filter);
  guchar *in_buf = GST_VIDEO_FRAME_PLANE_DATA (frame, 0);
  run_background_subtraction(cviivebackground, in_buf, in_buf);

  GST_DEBUG_OBJECT (cviivebackground, "transform_frame_ip");

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "cviivebackground", GST_RANK_NONE,
      GST_TYPE_CVI_IVE_BACKGROUND);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "cvi_ive"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "cvi_ive_background"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "NA"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    cviivebackground,
    "IVE background subtraction",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

