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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_CVI_IVE_BACKGROUND_H_
#define _GST_CVI_IVE_BACKGROUND_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

#include "ive.h"
G_BEGIN_DECLS

#define GST_TYPE_CVI_IVE_BACKGROUND   (gst_cvi_ive_background_get_type())
#define GST_CVI_IVE_BACKGROUND(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_CVI_IVE_BACKGROUND,GstCviIveBackground))
#define GST_CVI_IVE_BACKGROUND_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_CVI_IVE_BACKGROUND,GstCviIveBackgroundClass))
#define GST_IS_CVI_IVE_BACKGROUND(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_CVI_IVE_BACKGROUND))
#define GST_IS_CVI_IVE_BACKGROUND_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_CVI_IVE_BACKGROUND))

typedef struct _GstCviIveBackground GstCviIveBackground;
typedef struct _GstCviIveBackgroundClass GstCviIveBackgroundClass;

typedef struct GST_CVI_IVE_BACKGROUND_HANDLE {
  IVE_HANDLE handle;
  IVE_SRC_IMAGE_S src[2], tmp, andframe[2];
  IVE_DST_IMAGE_S dst;
  int count;
  int i_count;
} GST_CVI_IVE_BACKGROUND_HANDLE_S;

struct _GstCviIveBackground
{
  GstVideoFilter base_cviivebackground;
  GST_CVI_IVE_BACKGROUND_HANDLE_S *bk_handle;
};

struct _GstCviIveBackgroundClass
{
  GstVideoFilterClass base_cviivebackground_class;
};

GType gst_cvi_ive_background_get_type (void);

G_END_DECLS

#endif
