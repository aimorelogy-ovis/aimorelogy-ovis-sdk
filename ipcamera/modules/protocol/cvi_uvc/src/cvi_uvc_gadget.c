#include <errno.h>
#include <fcntl.h>
#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "cvi_uvc_gadget.h"
#include "uvc.h" // TODO, move this into cvi_uvc.h
#include "frame_cache.h"

/* Enable debug prints. */
#undef ENABLE_BUFFER_DEBUG
// #define ENABLE_BUFFER_DEBUG
#undef ENABLE_USB_REQUEST_DEBUG

#ifndef CLEAR
#define CLEAR(x) memset(&(x), 0, sizeof(x))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef clamp
#define clamp(val, min, max)                   \
    ({                                         \
        typeof(val) __val = (val);             \
        typeof(min) __min = (min);             \
        typeof(max) __max = (max);             \
        (void)(&__val == &__min);              \
        (void)(&__val == &__max);              \
        __val = __val < __min ? __min : __val; \
        __val > __max ? __max : __val;         \
    })
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(a[0])))
#endif

#ifndef pixfmtstr
#define pixfmtstr(x) (x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, ((x) >> 24) & 0xff
#endif
/*
 * The UVC webcam gadget kernel driver (g_webcam.ko) supports changing
 * the Brightness attribute of the Processing Unit (PU). by default. If
 * the underlying video capture device supports changing the Brightness
 * attribute of the image being acquired (like the Virtual Video, VIVI
 * driver), then we should route this UVC request to the respective
 * video capture device.
 *
 * Incase, there is no actual video capture device associated with the
 * UVC gadget and we wish to use this application as the final
 * destination of the UVC specific requests then we should return
 * pre-cooked (static) responses to GET_CUR(BRIGHTNESS) and
 * SET_CUR(BRIGHTNESS) commands to keep command verifier test tools like
 * UVC class specific test suite of USBCV, happy.
 *
 * Note that the values taken below are in sync with the VIVI driver and
 * must be changed for your specific video capture device. These values
 * also work well in case there in no actual video capture device.
 */
#define PU_BRIGHTNESS_MIN_VAL 0
#define PU_BRIGHTNESS_MAX_VAL 255
#define PU_BRIGHTNESS_STEP_SIZE 1
#define PU_BRIGHTNESS_DEFAULT_VAL 127

#define MAX_BITSTREAM_BUFFER_SIZE (2 * 1024 * 1024)
#define MJPG_PAYLOAD_SIZE 1024
#define H264_PAYLOAD_SIZE 512
#define H265_PAYLOAD_SIZE 1024

/* ---------------------------------------------------------------------------
 * Generic stuff
 */

/* IO methods supported */
enum io_method {
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

/* Buffer representing one video frame */
struct buffer {
    struct v4l2_buffer buf;
    void *start;
    size_t length;
};

/* ---------------------------------------------------------------------------
 * UVC specific stuff
 */

struct uvc_frame_info {
    uint32_t width;
    uint32_t height;
    uint32_t intervals[8];
};

struct uvc_format_info {
    uint32_t fcc;
    const struct uvc_frame_info *frames;
};

static const struct uvc_frame_info uvc_frames_mjpeg[] = {
    // {
    //     2560,
    //     1440,
    //     {400000, 0},
    // },
    {
        1920,
        1080,
        {333333, 0},
    },
    // {
    //     1280,
    //     720,
    //     {333333, 0},
    // },
    // {
    //     640,
    //     360,
    //     {333333, 0},
    // },
    {
        0,
        0,
        {
            0,
        },
    },
};

static const struct uvc_frame_info uvc_frames_h264[] = {
    // {
    //     2560,
    //     1440,
    //     {400000, 0},
    // },
    {
        1920,
        1080,
        {333333, 0},
    },
    // {
    //     1280,
    //     720,
    //     {400000, 0},
    // },
    // {
    //     640,
    //     360,
    //     {400000, 0},
    // },
    {
        0,
        0,
        {
            0,
        },
    },
};

static const struct uvc_frame_info uvc_frames_h265[] = {
    // {
    //     2560,
    //     1440,
    //     {400000, 0},
    // },
    {
        1920,
        1080,
        {333333, 0},
    },
    // {
    //     1280,
    //     720,
    //     {400000, 0},
    // },
    // {
    //     640,
    //     360,
    //     {400000, 0},
    // },
    {
        0,
        0,
        {
            0,
        },
    },
};

// TODO, move into parameters
static const struct uvc_format_info uvc_formats[] = {
    {V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg},
    {V4L2_PIX_FMT_H264, uvc_frames_h264},
    {V4L2_PIX_FMT_HEVC, uvc_frames_h265},
};

/* ---------------------------------------------------------------------------
 * V4L2 and UVC device instances
 */

/* Represents a UVC based video output device */
typedef struct tagUVC_DEVICE_CTX_S {
    /* uvc device specific */
    int uvc_fd;
    int is_streaming;
    int run_standalone;
    char uvc_devname[128];

    /* uvc control request specific */

    struct uvc_streaming_control probe;
    struct uvc_streaming_control commit;
    int control;
    struct uvc_request_data request_error_code;
    uint32_t brightness_val;

    /* uvc buffer specific */
    enum io_method io;
    struct buffer *mem;
    struct buffer *dummy_buf;
    uint32_t nbufs;
    uint32_t fcc;
    uint32_t width;
    uint32_t height;
    uint32_t fps;

    uint32_t bulk;
    uint8_t color;
    uint32_t imgsize;

    /* USB speed specific */
    int mult;
    int burst;
    int maxpkt;
    enum usb_device_speed speed;

    /* uvc specific flags */
    int first_buffer_queued;
    int uvc_shutdown_requested;

    /* uvc buffer queue and dequeue counters */
    unsigned long long int qbuf_count;
    unsigned long long int dqbuf_count;

    /* v4l2 device hook */
    struct v4l2_device *vdev;
} UVC_DEVICE_CTX_S;
static UVC_DEVICE_CTX_S s_stUVCDevCtx;

/* Represents a V4L2 based video capture device */
struct v4l2_device {
    /* v4l2 device specific */
    int v4l2_fd;
    int is_streaming;
    char *v4l2_devname;

    /* v4l2 buffer specific */
    enum io_method io;
    struct buffer *mem;
    uint32_t nbufs;

    /* v4l2 buffer queue and dequeue counters */
    unsigned long long int qbuf_count;
    unsigned long long int dqbuf_count;

    /* uvc device hook */
    UVC_DEVICE_CTX_S *udev;
};


#define WAITED_NODE_SIZE (3)
static frame_node_t *__waited_node[WAITED_NODE_SIZE];
static void clear_waited_node()
{
    int i = 0;
    uvc_cache_t *uvc_cache = uvc_cache_get();

    for (i = 0; i < WAITED_NODE_SIZE; i++)
    {
        if ((__waited_node[i] != 0) && uvc_cache)
        {
            put_node_to_queue(uvc_cache->free_queue, __waited_node[i]);
            __waited_node[i] = 0;
        }
    }
}

/* forward declarations */
static int uvc_video_stream(UVC_DEVICE_CTX_S *dev, int enable);

/* ---------------------------------------------------------------------------
 * V4L2 streaming related
 */
static int v4l2_reqbufs_mmap(struct v4l2_device *dev, int nbufs) {
    struct v4l2_requestbuffers req;
    uint32_t i = 0;
    int ret;

    CLEAR(req);

    req.count = nbufs;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(dev->v4l2_fd, VIDIOC_REQBUFS, &req);
    if (ret < 0) {
        if (ret == -EINVAL)
            printf("V4L2: does not support memory mapping\n");
        else
            printf("V4L2: VIDIOC_REQBUFS error %s (%d).\n", strerror(errno), errno);
        goto err;
    }

    if (!req.count) return 0;

    if (req.count < 2) {
        printf("V4L2: Insufficient buffer memory.\n");
        ret = -EINVAL;
        goto err;
    }

    /* Map the buffers. */
    dev->mem = calloc(req.count, sizeof dev->mem[0]);
    if (!dev->mem) {
        printf("V4L2: Out of memory\n");
        ret = -ENOMEM;
        goto err;
    }

    for (i = 0; i < req.count; ++i) {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        ret = ioctl(dev->v4l2_fd, VIDIOC_QUERYBUF, &(dev->mem[i].buf));
        if (ret < 0) {
            printf(
                "V4L2: VIDIOC_QUERYBUF failed for buf %d: "
                "%s (%d).\n",
                i, strerror(errno), errno);
            ret = -EINVAL;
            goto err_free;
        }

        dev->mem[i].start =
            mmap(NULL /* start anywhere */, dev->mem[i].buf.length, PROT_READ | PROT_WRITE /* required */,
                 MAP_SHARED /* recommended */, dev->v4l2_fd, dev->mem[i].buf.m.offset);

        if (MAP_FAILED == dev->mem[i].start) {
            printf("V4L2: Unable to map buffer %u: %s (%d).\n", i, strerror(errno), errno);
            dev->mem[i].length = 0;
            ret = -EINVAL;
            goto err_free;
        }

        dev->mem[i].length = dev->mem[i].buf.length;
        printf("V4L2: Buffer %u mapped at address %p.\n", i, dev->mem[i].start);
    }

    dev->nbufs = req.count;
    printf("V4L2: %u buffers allocated.\n", req.count);

    return 0;

err_free:
    free(dev->mem);
err:
    return ret;
}

static int v4l2_reqbufs_userptr(struct v4l2_device *dev, int nbufs) {
    struct v4l2_requestbuffers req;
    int ret;

    CLEAR(req);

    req.count = nbufs;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->v4l2_fd, VIDIOC_REQBUFS, &req);
    if (ret < 0) {
        if (ret == -EINVAL)
            printf("V4L2: does not support user pointer i/o\n");
        else
            printf("V4L2: VIDIOC_REQBUFS error %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    dev->nbufs = req.count;
    printf("V4L2: %u buffers allocated.\n", req.count);

    return 0;
}

static int v4l2_reqbufs(struct v4l2_device *dev, int nbufs) {
    int ret = 0;

    switch (dev->io) {
        case IO_METHOD_MMAP:
            ret = v4l2_reqbufs_mmap(dev, nbufs);
            break;

        case IO_METHOD_USERPTR:
            ret = v4l2_reqbufs_userptr(dev, nbufs);
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

static int v4l2_qbuf_mmap(struct v4l2_device *dev) {
    uint32_t i;
    int ret;

    for (i = 0; i < dev->nbufs; ++i) {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        ret = ioctl(dev->v4l2_fd, VIDIOC_QBUF, &(dev->mem[i].buf));
        if (ret < 0) {
            printf("V4L2: VIDIOC_QBUF failed : %s (%d).\n", strerror(errno), errno);
            return ret;
        }

        dev->qbuf_count++;
    }

    return 0;
}

static int v4l2_qbuf(struct v4l2_device *dev) {
    int ret = 0;

    switch (dev->io) {
        case IO_METHOD_MMAP:
            ret = v4l2_qbuf_mmap(dev);
            break;

        case IO_METHOD_USERPTR:
            /* Empty. */
            ret = 0;
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

/* ---------------------------------------------------------------------------
 * V4L2 generic stuff
 */
static int v4l2_set_ctrl(struct v4l2_device *dev, int new_val, int ctrl) {
    struct v4l2_queryctrl queryctrl;
    struct v4l2_control control;
    int ret;

    CLEAR(queryctrl);

    switch (ctrl) {
        case V4L2_CID_BRIGHTNESS:
            queryctrl.id = V4L2_CID_BRIGHTNESS;
            ret = ioctl(dev->v4l2_fd, VIDIOC_QUERYCTRL, &queryctrl);
            if (-1 == ret) {
                if (errno != EINVAL)
                    printf(
                        "V4L2: VIDIOC_QUERYCTRL"
                        " failed: %s (%d).\n",
                        strerror(errno), errno);
                else
                    printf(
                        "V4L2_CID_BRIGHTNESS is not"
                        " supported: %s (%d).\n",
                        strerror(errno), errno);

                return ret;
            } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                printf("V4L2_CID_BRIGHTNESS is not supported.\n");
                ret = -EINVAL;
                return ret;
            } else {
                CLEAR(control);
                control.id = V4L2_CID_BRIGHTNESS;
                control.value = new_val;

                ret = ioctl(dev->v4l2_fd, VIDIOC_S_CTRL, &control);
                if (-1 == ret) {
                    printf("V4L2: VIDIOC_S_CTRL failed: %s (%d).\n", strerror(errno), errno);
                    return ret;
                }
            }
            printf("V4L2: Brightness control changed to value = 0x%x\n", new_val);
            break;

        default:
            /* TODO: We don't support any other controls. */
            return -EINVAL;
    }

    return 0;
}

static int v4l2_start_capturing(struct v4l2_device *dev) {
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret;

    ret = ioctl(dev->v4l2_fd, VIDIOC_STREAMON, &type);
    if (ret < 0) {
        printf("V4L2: Unable to start streaming: %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    printf("V4L2: Starting video stream.\n");

    return 0;
}

static int v4l2_stop_capturing(struct v4l2_device *dev) {
    enum v4l2_buf_type type;
    int ret;

    switch (dev->io) {
        case IO_METHOD_MMAP:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            ret = ioctl(dev->v4l2_fd, VIDIOC_STREAMOFF, &type);
            if (ret < 0) {
                printf("V4L2: VIDIOC_STREAMOFF failed: %s (%d).\n", strerror(errno), errno);
                return ret;
            }

            break;
        default:
            /* Nothing to do. */
            break;
    }

    return 0;
}

/* ---------------------------------------------------------------------------
 * UVC generic stuff
 */

static int uvc_video_set_format(UVC_DEVICE_CTX_S *dev) {
    struct v4l2_format fmt;
    int ret;

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = dev->width;
    fmt.fmt.pix.height = dev->height;
    fmt.fmt.pix.pixelformat = dev->fcc;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

	if (s_stUVCDevCtx.io == IO_METHOD_MMAP)
		fmt.fmt.pix.sizeimage = dev->width * dev->height * 3 / 2;
    else if (dev->fcc == V4L2_PIX_FMT_MJPEG || dev->fcc == V4L2_PIX_FMT_H264 || dev->fcc == V4L2_PIX_FMT_HEVC || dev->fcc == V4L2_PIX_FMT_YUYV)
        fmt.fmt.pix.sizeimage = dev->imgsize;

    printf("[%s] fmt.fmt.pix.sizeimage: %u, dev->imgsize: %u\n", __func__, fmt.fmt.pix.sizeimage,
            dev->imgsize);

    ret = ioctl(dev->uvc_fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        printf("UVC: Unable to set format %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    printf("UVC: Setting format to: %c%c%c%c %ux%u\n", pixfmtstr(dev->fcc), dev->width, dev->height);

    return 0;
}

static void UVC_VideoDisable(UVC_DEVICE_CTX_S *dev) { }

static int uvc_video_stream(UVC_DEVICE_CTX_S *dev, int enable) {
    int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    int ret;

    if (!enable) {
        ret = ioctl(dev->uvc_fd, VIDIOC_STREAMOFF, &type);
        if (ret < 0) {
            printf("UVC: VIDIOC_STREAMOFF failed: %s (%d).\n", strerror(errno), errno);
            return ret;
        }

        printf("UVC: Stopping video stream.\n");
        UVC_VideoDisable(dev);
        printf("UVC_VideoDisable done.\n"); 

        // int policy;
        // struct sched_param param;
        // // 获取当前线程的调度策略和参数
        // pthread_getschedparam(pthread_self(), &policy, &param);
        // printf("Current thread priority: %d\n", param.sched_priority);

        // // 修改优先级
        // param.sched_priority = 95; // 设置新优先级
        // if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) != 0) {
        //     printf("pthread_setschedparam err\n");
        // }

        // // 再次获取并打印新的优先级
        // pthread_getschedparam(pthread_self(), &policy, &param);
        // printf("New thread priority: %d\n", param.sched_priority);

        return 0;
    }

    ret = ioctl(dev->uvc_fd, VIDIOC_STREAMON, &type);
    if (ret < 0) {
        printf("UVC: Unable to start streaming %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    printf("UVC: Starting video stream.\n");

    // int policy;
    // struct sched_param param;
    // // 获取当前线程的调度策略和参数
    // pthread_getschedparam(pthread_self(), &policy, &param);
    // printf("Current thread priority: %d\n", param.sched_priority);

    // // 修改优先级
    // param.sched_priority = 0; // 设置新优先级
    // if (pthread_setschedparam(pthread_self(), SCHED_OTHER, &param) != 0) {
    //     printf("pthread_setschedparam err\n");
    // }
    // param.sched_priority = 80; // 设置新优先级
    // if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) != 0) {
    //     printf("pthread_setschedparam err\n");
    // }

    // 再次获取并打印新的优先级
    // pthread_getschedparam(pthread_self(), &policy, &param);
    // printf("New thread priority: %d\n", param.sched_priority);

    dev->uvc_shutdown_requested = 0;

    return 0;
}

static int uvc_uninit_device(UVC_DEVICE_CTX_S *dev) {
    uint32_t i;
    int ret;

    switch (dev->io) {
        case IO_METHOD_MMAP:
            for (i = 0; i < dev->nbufs; ++i) {
                ret = munmap(dev->mem[i].start, dev->mem[i].length);
                if (ret < 0) {
                    printf("UVC: munmap failed\n");
                    return ret;
                }
            }

            free(dev->mem);
            break;

        case IO_METHOD_USERPTR:
        default:
            // if (dev->run_standalone) {
            //     for (i = 0; i < dev->nbufs; ++i) free(dev->dummy_buf[i].start);

            //     free(dev->dummy_buf);
            // }
            break;
    }

    return 0;
}

static int32_t UVC_DeviceOpen(const char *devname, UVC_DEVICE_CTX_S *dev) {
    struct v4l2_capability cap;
    int fd;
    int ret = -EINVAL;

    fd = open(devname, O_RDWR | O_NONBLOCK);
    if (fd == -1) {
        printf("UVC: device open failed: %s (%d).\n", strerror(errno), errno);
        return ret;
    }
    strcpy(dev->uvc_devname, devname);

    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0) {
        printf("UVC: unable to query uvc device: %s (%d)\n", strerror(errno), errno);
        goto err;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
        printf("UVC: %s is no video output device\n", devname);
        goto err;
    }

    printf("uvc device is %s on bus %s\n", cap.card, cap.bus_info);

    dev->uvc_fd = fd;
    printf("uvc open succeeded, file descriptor = %d\n", dev->uvc_fd);

    CLEAR(__waited_node);

    return 0;

err:
    close(fd);
    return ret;
}

static inline CVI_UVC_STREAM_FORMAT_E UVC_FCC_TO_STREAM_FORMAT(uint32_t fcc) {
    switch (fcc) {
        case V4L2_PIX_FMT_YUYV:
            return CVI_UVC_STREAM_FORMAT_YUV420;
        case V4L2_PIX_FMT_MJPEG:
            return CVI_UVC_STREAM_FORMAT_MJPEG;
        case V4L2_PIX_FMT_H264:
            return CVI_UVC_STREAM_FORMAT_H264;
        case V4L2_PIX_FMT_HEVC:
            return CVI_UVC_STREAM_FORMAT_H265;
        default:
            return CVI_UVC_STREAM_FORMAT_MJPEG;
    }
}

static void UVC_VideoEnable(UVC_DEVICE_CTX_S *dev) {
    UVC_STREAM_ATTR_S stStreamAttr;
    memset(&stStreamAttr, 0, sizeof(UVC_STREAM_ATTR_S));
    stStreamAttr.enFormat = UVC_FCC_TO_STREAM_FORMAT(dev->fcc);
    stStreamAttr.u32Width = dev->width;
    stStreamAttr.u32Height = dev->height;
    stStreamAttr.u32Fps = dev->fps;
    // stStreamAttr.u32BitRate =  H264_BITRATE;
}

/* ---------------------------------------------------------------------------
 * UVC streaming related
 */

static void uvc_video_fill_buffer(UVC_DEVICE_CTX_S *dev, struct v4l2_buffer* buf)
{
    int retry_count = 0;
    buf->bytesused = 0;

    // printf("buf->index = %d\n", buf->index);
    // struct timeval tTimeVal;
    // gettimeofday(&tTimeVal, NULL);
    // struct tm *tTM = localtime(&tTimeVal.tv_sec);
    // printf("%02d:%02d.%03ld.%03ld\n",
    //     tTM->tm_min, tTM->tm_sec,
    //     tTimeVal.tv_usec / 1000, tTimeVal.tv_usec % 1000);

    switch (dev->fcc)
    {
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_HEVC:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_YUV420:
    {
        uvc_cache_t *uvc_cache = uvc_cache_get();
        frame_node_t *node = 0;
        frame_queue_t *q = 0, *fq = 0;

retry:
        if (uvc_cache)
        {
            // printf("ok_queue:\n");
            // debug_dump_queue(uvc_cache->ok_queue);
            q  = uvc_cache->ok_queue;
            fq = uvc_cache->free_queue;
            get_node_from_queue(q, &node);
        }

        if ((__waited_node[buf->index] != 0) && uvc_cache)
        {
            // memset(__waited_node[buf->index]->mem, 0, __waited_node[buf->index]->length);
            put_node_to_queue(fq, __waited_node[buf->index]);
            __waited_node[buf->index] = 0;
        }

        if (node != 0)
        {
            buf->bytesused = node->used;
            buf->m.userptr = (unsigned long)node->mem;
            buf->length = node->length;
            __waited_node[buf->index] = node;
        }
        else if (retry_count++ < 1000)
        {
            // the perfect solution is using locker and waiting queue's notify.
            // but here just only simply used usleep method and try again.
            // it works fine now.

            // printf("dump ok\n");
            // debug_dump_queue(uvc_cache->ok_queue);
            // printf("dump free\n");
            // debug_dump_queue(uvc_cache->free_queue);
            // printf("retry===== (%d)\n", retry_count);
            usleep(10*1000);
            goto retry;
        }
        else{
            printf("retry failed\n");
        }
        // printf("retry_count = %d, node %d\n", retry_count, node != 0);
    }
        break;
    default:
        printf("dev->fcc = %d\n", dev->fcc);
        break;
    }


}

static int uvc_video_process(UVC_DEVICE_CTX_S *dev) {
    struct v4l2_buffer ubuf;
    struct v4l2_buffer vbuf;
    uint32_t i;
    int ret;
    /*
     * Return immediately if UVC video output device has not started
     * streaming yet.
     */
    if (!dev->is_streaming) {
        usleep(1000);
        return 0;
    }
    /* Prepare a v4l2 buffer to be dequeued from UVC domain. */
    CLEAR(ubuf);

    ubuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    switch (dev->io) {
        case IO_METHOD_MMAP:
            ubuf.memory = V4L2_MEMORY_MMAP;
            break;

        case IO_METHOD_USERPTR:
        default:
            ubuf.memory = V4L2_MEMORY_USERPTR;
            break;
    }
    if (dev->run_standalone) {
        //struct timeval perf_t0, perf_t1;
        //unsigned long use_time = 0;

        /* UVC stanalone setup. */
        ret = ioctl(dev->uvc_fd, VIDIOC_DQBUF, &ubuf);
        if (ret < 0) return ret;

        // ubuf.bytesused = s_payload_size[ubuf.index];
        // ubuf.length = s_payload_size[ubuf.index];
        // printf("bytesused:%d, length:%d\n", ubuf.bytesused, ubuf.length);

        dev->dqbuf_count++;
#ifdef ENABLE_BUFFER_DEBUG
        printf("DeQueued buffer at UVC side = %d\n", ubuf.index);
#endif
        // gettimeofday(&perf_t0, NULL);
        // usleep(200 * 1000);
        uvc_video_fill_buffer(dev, &ubuf);
        // gettimeofday(&perf_t1, NULL);
        // use_time = (perf_t1.tv_sec - perf_t0.tv_sec) * 1000 + (perf_t1.tv_usec - perf_t0.tv_usec) / 1000;
        // printf("======= %s use_time: %lu ms =======\n", "uvc_video_fill_buffer", use_time);
        ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &ubuf);
        if (ret < 0) {
            printf("UVC: Unable to queue buffer: %s (%d).\n", strerror(errno), errno);
            return ret;
        }

        dev->qbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
        printf("ReQueueing buffer at UVC side = %d\n", ubuf.index);
#endif
    } else {
        /* UVC - V4L2 integrated path. */

        /*
         * Return immediately if V4L2 video capture device has not
         * started streaming yet or if QBUF was not called even once on
         * the UVC side.
         */
        if (!dev->vdev->is_streaming || !dev->first_buffer_queued) return 0;

        /*
         * Do not dequeue buffers from UVC side until there are atleast
         * 2 buffers available at UVC domain.
         */
        if (!dev->uvc_shutdown_requested)
            if ((dev->dqbuf_count + 1) >= dev->qbuf_count) return 0;

        /* Dequeue the spent buffer from UVC domain */
        ret = ioctl(dev->uvc_fd, VIDIOC_DQBUF, &ubuf);
        if (ret < 0) {
            printf("UVC: Unable to dequeue buffer: %s (%d).\n", strerror(errno), errno);
            return ret;
        }

        if (dev->io == IO_METHOD_USERPTR)
            for (i = 0; i < dev->nbufs; ++i)
                if (ubuf.m.userptr == (unsigned long)dev->vdev->mem[i].start &&
                    ubuf.length == dev->vdev->mem[i].length)
                    break;

        dev->dqbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
        printf("DeQueued buffer at UVC side=%d\n", ubuf.index);
#endif

        /*
         * If the dequeued buffer was marked with state ERROR by the
         * underlying UVC driver gadget, do not queue the same to V4l2
         * and wait for a STREAMOFF event on UVC side corresponding to
         * set_alt(0). So, now all buffers pending at UVC end will be
         * dequeued one-by-one and we will enter a state where we once
         * again wait for a set_alt(1) command from the USB host side.
         */
        if (ubuf.flags & V4L2_BUF_FLAG_ERROR) {
            dev->uvc_shutdown_requested = 1;
            printf(
                "UVC: Possible USB shutdown requested from "
                "Host, seen during VIDIOC_DQBUF\n");
            return 0;
        }

        /* Queue the buffer to V4L2 domain */
        CLEAR(vbuf);

        vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vbuf.memory = V4L2_MEMORY_MMAP;
        vbuf.index = ubuf.index;

        ret = ioctl(dev->vdev->v4l2_fd, VIDIOC_QBUF, &vbuf);
        if (ret < 0) return ret;

        dev->vdev->qbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
        printf("ReQueueing buffer at V4L2 side = %d\n", vbuf.index);
#endif
    }

    return 0;
}

static int uvc_video_qbuf_mmap(UVC_DEVICE_CTX_S *dev) {
    uint32_t i;
    int ret;

    for (i = 0; i < dev->nbufs; ++i) {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        /* UVC standalone setup. */
        if (dev->run_standalone) uvc_video_fill_buffer(dev, &(dev->mem[i].buf));

        ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &(dev->mem[i].buf));
        if (ret < 0) {
            printf("UVC: VIDIOC_QBUF failed : %s (%d).\n", strerror(errno), errno);
            return ret;
        }

        dev->qbuf_count++;
    }

    return 0;
}

static int uvc_video_qbuf_userptr(UVC_DEVICE_CTX_S *dev) {
    uint32_t i;
    int ret;

    /* UVC standalone setup. */
    if (dev->run_standalone) {
        for (i = 0; i < dev->nbufs; ++i) {
            struct v4l2_buffer buf;

            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
            buf.memory = V4L2_MEMORY_USERPTR;
            // buf.m.userptr = (unsigned long)dev->dummy_buf[i].start;
            // buf.length = dev->dummy_buf[i].length;
            buf.index = i;
            // buf.bytesused = dev->dummy_buf[i].length;

            uvc_video_fill_buffer(dev, &buf);

            ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &buf);
            if (ret < 0) {
                printf("UVC: VIDIOC_QBUF failed : %s (%d).\n", strerror(errno), errno);
                return ret;
            }

            dev->qbuf_count++;
        }
    }

    return 0;
}

static int uvc_video_qbuf(UVC_DEVICE_CTX_S *dev) {
    int ret = 0;

    switch (dev->io) {
        case IO_METHOD_MMAP:
            ret = uvc_video_qbuf_mmap(dev);
            break;

        case IO_METHOD_USERPTR:
            ret = uvc_video_qbuf_userptr(dev);
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

static int uvc_video_reqbufs_mmap(UVC_DEVICE_CTX_S *dev, int nbufs) {
    struct v4l2_requestbuffers rb;
    uint32_t i;
    int ret;

    CLEAR(rb);

    rb.count = nbufs;
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0) {
        if (ret == -EINVAL)
            printf("UVC: does not support memory mapping\n");
        else
            printf("UVC: Unable to allocate buffers: %s (%d).\n", strerror(errno), errno);
        goto err;
    }

    if (!rb.count) return 0;

    if (rb.count < 2) {
        printf("UVC: Insufficient buffer memory.\n");
        ret = -EINVAL;
        goto err;
    }

    /* Map the buffers. */
    dev->mem = calloc(rb.count, sizeof dev->mem[0]);
    if (!dev->mem) {
        printf("UVC: Out of memory\n");
        ret = -ENOMEM;
        goto err;
    }

    for (i = 0; i < rb.count; ++i) {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        ret = ioctl(dev->uvc_fd, VIDIOC_QUERYBUF, &(dev->mem[i].buf));
        if (ret < 0) {
            printf(
                "UVC: VIDIOC_QUERYBUF failed for buf %d: "
                "%s (%d).\n",
                i, strerror(errno), errno);
            ret = -EINVAL;
            goto err_free;
        }
        dev->mem[i].start =
            mmap(NULL /* start anywhere */, dev->mem[i].buf.length, PROT_READ | PROT_WRITE /* required */,
                 MAP_SHARED /* recommended */, dev->uvc_fd, dev->mem[i].buf.m.offset);

        if (MAP_FAILED == dev->mem[i].start) {
            printf("UVC: Unable to map buffer %u: %s (%d).\n", i, strerror(errno), errno);
            dev->mem[i].length = 0;
            ret = -EINVAL;
            goto err_free;
        }

        dev->mem[i].length = dev->mem[i].buf.length;
        printf("UVC: Buffer %u mapped at address %p.\n", i, dev->mem[i].start);
    }

    dev->nbufs = rb.count;
    printf("UVC: %u buffers allocated.\n", rb.count);

    return 0;

err_free:
    free(dev->mem);
err:
    return ret;
}

static int uvc_video_reqbufs_userptr(UVC_DEVICE_CTX_S *dev, int nbufs) {
    struct v4l2_requestbuffers rb;
    int ret;

    CLEAR(rb);

    rb.count = nbufs;
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0) {
        if (ret == -EINVAL)
            printf("UVC: does not support user pointer i/o\n");
        else
            printf("UVC: VIDIOC_REQBUFS error %s (%d).\n", strerror(errno), errno);
        goto err;
    }

    if (!rb.count) return 0;

    dev->nbufs = rb.count;
    printf("UVC: %u buffers allocated, per size:%d.\n", rb.count, sizeof dev->dummy_buf[0]);
    return 0;

err:
    return ret;
}

static int uvc_video_reqbufs(UVC_DEVICE_CTX_S *dev, int nbufs) {
    int ret = 0;

    switch (dev->io) {
        case IO_METHOD_MMAP:
            ret = uvc_video_reqbufs_mmap(dev, nbufs);
            break;

        case IO_METHOD_USERPTR:
            ret = uvc_video_reqbufs_userptr(dev, nbufs);
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

/*
 * This function is called in response to either:
 * 	- A SET_ALT(interface 1, alt setting 1) command from USB host,
 * 	  if the UVC gadget supports an ISOCHRONOUS video streaming endpoint
 * 	  or,
 *
 *	- A UVC_VS_COMMIT_CONTROL command from USB host, if the UVC gadget
 *	  supports a BULK type video streaming endpoint.
 */
static int uvc_handle_streamon_event(UVC_DEVICE_CTX_S *dev) {
    int ret;

    ret = uvc_video_reqbufs(dev, dev->nbufs);
    if (ret < 0) {
        printf("[%s] after uvc_video_reqbufs\n", __func__);
        goto err;
    }
    if (!dev->run_standalone) {
        /* UVC - V4L2 integrated path. */
        if (IO_METHOD_USERPTR == dev->vdev->io) {
            /*
             * Ensure that the V4L2 video capture device has already
             * some buffers queued.
             */
            ret = v4l2_reqbufs(dev->vdev, dev->vdev->nbufs);
            if (ret < 0) goto err;
        }

        ret = v4l2_qbuf(dev->vdev);
        if (ret < 0) goto err;

        /* Start V4L2 capturing now. */
        ret = v4l2_start_capturing(dev->vdev);
        if (ret < 0) goto err;

        dev->vdev->is_streaming = 1;
    }

    /* Common setup. */

    /* Queue buffers to UVC domain and start streaming. */
    ret = uvc_video_qbuf(dev);
    if (ret < 0) {
        printf("[%s] after uvc_video_qbuf\n", __func__);
        goto err;
    }

    if (dev->run_standalone) {
        uvc_video_stream(dev, 1);
        dev->first_buffer_queued = 1;
        dev->is_streaming = 1;
    }

    return 0;

err:
    return ret;
}

/* ---------------------------------------------------------------------------
 * UVC Request processing
 */

static void uvc_fill_streaming_control(UVC_DEVICE_CTX_S *dev, struct uvc_streaming_control *ctrl, int iframe,
                                       int iformat) {
    const struct uvc_format_info *format;
    const struct uvc_frame_info *frame;
    uint32_t nframes;

    printf("[%s] before iframe:%d, iformat:%d\n", __FUNCTION__, iframe, iformat);

    if (iformat < 0) iformat = ARRAY_SIZE(uvc_formats) + iformat;
    if (iformat < 0 || iformat >= (int)ARRAY_SIZE(uvc_formats)) return;
    format = &uvc_formats[iformat];

    nframes = 0;
    while (format->frames[nframes].width != 0) ++nframes;

    if (iframe < 0) iframe = nframes + iframe;
    if (iframe < 0 || iframe >= (int)nframes) return;
    frame = &format->frames[iframe];

    printf("[%s] after iframe:%d, iformat:%d\n", __FUNCTION__, iframe, iformat);

    memset(ctrl, 0, sizeof *ctrl);

    ctrl->bmHint = 1;
    ctrl->bFormatIndex = iformat + 1;
    ctrl->bFrameIndex = iframe + 1;
    ctrl->dwFrameInterval = frame->intervals[0];
    switch (format->fcc) {
        case V4L2_PIX_FMT_YUYV:
            // ctrl->dwMaxVideoFrameSize = frame->width * frame->height * 2;
            ctrl->dwMaxVideoFrameSize = MAX_BITSTREAM_BUFFER_SIZE;
            break;
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_HEVC:
            ctrl->dwMaxVideoFrameSize = MAX_BITSTREAM_BUFFER_SIZE;
            break;
    }

    /* TODO: the UVC maxpayload transfer size should be filled
     * by the driver.
     */
    if (!dev->bulk)
        ctrl->dwMaxPayloadTransferSize = (dev->maxpkt) * (dev->mult + 1) * (dev->burst + 1);
    else
        ctrl->dwMaxPayloadTransferSize = ctrl->dwMaxVideoFrameSize;

    ctrl->bmFramingInfo = 3;
    ctrl->bPreferedVersion = 1;
    ctrl->bMaxVersion = 1;
}

static void uvc_events_process_standard(UVC_DEVICE_CTX_S *dev, struct usb_ctrlrequest *ctrl,
                                        struct uvc_request_data *resp) {
    printf("standard request\n");
    (void)dev;
    (void)ctrl;
    (void)resp;
}

static void uvc_events_process_control(UVC_DEVICE_CTX_S *dev, uint8_t req, uint8_t cs, uint8_t entity_id,
                                       uint8_t len, struct uvc_request_data *resp) {
    switch (entity_id) {
        case 0:
            switch (cs) {
                case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
                    /* Send the request error code last prepared. */
                    resp->data[0] = dev->request_error_code.data[0];
                    resp->length = dev->request_error_code.length;
                    break;

                default:
                    /*
                     * If we were not supposed to handle this
                     * 'cs', prepare an error code response.
                     */
                    dev->request_error_code.data[0] = 0x06;
                    dev->request_error_code.length = 1;
                    break;
            }
            break;

        /* Camera terminal unit 'UVC_VC_INPUT_TERMINAL'. */
        case 1:
            switch (cs) {
                /*
                 * We support only 'UVC_CT_AE_MODE_CONTROL' for CAMERA
                 * terminal, as our bmControls[0] = 2 for CT. Also we
                 * support only auto exposure.
                 */
                case UVC_CT_AE_MODE_CONTROL:
                    switch (req) {
                        case UVC_SET_CUR:
                            /* Incase of auto exposure, attempts to
                             * programmatically set the auto-adjusted
                             * controls are ignored.
                             */
                            resp->data[0] = 0x01;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;

                        case UVC_GET_INFO:
                            /*
                             * TODO: We support Set and Get requests, but
                             * don't support async updates on an video
                             * status (interrupt) endpoint as of
                             * now.
                             */
                            resp->data[0] = 0x03;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;

                        case UVC_GET_CUR:
                        case UVC_GET_DEF:
                        case UVC_GET_RES:
                            /* Auto Mode â€“ auto Exposure Time, auto Iris. */
                            resp->data[0] = 0x02;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        default:
                            /*
                             * We don't support this control, so STALL the
                             * control ep.
                             */
                            resp->length = -EL2HLT;
                            /*
                             * For every unsupported control request
                             * set the request error code to appropriate
                             * value.
                             */
                            dev->request_error_code.data[0] = 0x07;
                            dev->request_error_code.length = 1;
                            break;
                    }
                    break;

                default:
                    /*
                     * We don't support this control, so STALL the control
                     * ep.
                     */
                    resp->length = -EL2HLT;
                    /*
                     * If we were not supposed to handle this
                     * 'cs', prepare a Request Error Code response.
                     */
                    dev->request_error_code.data[0] = 0x06;
                    dev->request_error_code.length = 1;
                    break;
            }
            break;

        /* processing unit 'UVC_VC_PROCESSING_UNIT' */
        case 2:
            switch (cs) {
                /*
                 * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
                 * Unit, as our bmControls[0] = 1 for PU.
                 */
                case UVC_PU_BRIGHTNESS_CONTROL:
                    switch (req) {
                        case UVC_SET_CUR:
                            resp->data[0] = 0x0;
                            resp->length = len;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_MIN:
                            resp->data[0] = PU_BRIGHTNESS_MIN_VAL;
                            resp->length = 2;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_MAX:
                            resp->data[0] = PU_BRIGHTNESS_MAX_VAL;
                            resp->length = 2;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_CUR:
                            resp->length = 2;
                            memcpy(&resp->data[0], &dev->brightness_val, resp->length);
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_INFO:
                            /*
                             * We support Set and Get requests and don't
                             * support async updates on an interrupt endpt
                             */
                            resp->data[0] = 0x03;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request, set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_DEF:
                            resp->data[0] = PU_BRIGHTNESS_DEFAULT_VAL;
                            resp->length = 2;
                            /*
                             * For every successfully handled control
                             * request, set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_RES:
                            resp->data[0] = PU_BRIGHTNESS_STEP_SIZE;
                            resp->length = 2;
                            /*
                             * For every successfully handled control
                             * request, set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        default:
                            /*
                             * We don't support this control, so STALL the
                             * default control ep.
                             */
                            resp->length = -EL2HLT;
                            /*
                             * For every unsupported control request
                             * set the request error code to appropriate
                             * code.
                             */
                            dev->request_error_code.data[0] = 0x07;
                            dev->request_error_code.length = 1;
                            break;
                    }
                    break;

                default:
                    /*
                     * We don't support this control, so STALL the control
                     * ep.
                     */
                    resp->length = -EL2HLT;
                    /*
                     * If we were not supposed to handle this
                     * 'cs', prepare a Request Error Code response.
                     */
                    dev->request_error_code.data[0] = 0x06;
                    dev->request_error_code.length = 1;
                    break;
            }

            break;

        default:
            /*
             * If we were not supposed to handle this
             * 'cs', prepare a Request Error Code response.
             */
            dev->request_error_code.data[0] = 0x06;
            dev->request_error_code.length = 1;
            break;
    }

    printf("control request (req %02x cs %02x) entity_id:%d\n", req, cs, entity_id);
}

static void uvc_events_process_streaming(UVC_DEVICE_CTX_S *dev, uint8_t req, uint8_t cs,
                                         struct uvc_request_data *resp) {
    struct uvc_streaming_control *ctrl;

    printf("streaming request (req %02x cs %02x)\n", req, cs);

    if (cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL) return;

    ctrl = (struct uvc_streaming_control *)&resp->data;
    resp->length = sizeof *ctrl;

    switch (req) {
        case UVC_SET_CUR:
            dev->control = cs;
            resp->length = 34;
            break;

        case UVC_GET_CUR:
            if (cs == UVC_VS_PROBE_CONTROL)
                memcpy(ctrl, &dev->probe, sizeof *ctrl);
            else
                memcpy(ctrl, &dev->commit, sizeof *ctrl);
            break;

        case UVC_GET_MIN:
        case UVC_GET_MAX:
        case UVC_GET_DEF:
            uvc_fill_streaming_control(dev, ctrl, req == UVC_GET_MAX ? -1 : 0, req == UVC_GET_MAX ? -1 : 0);
            break;

        case UVC_GET_RES:
            CLEAR(ctrl);
            break;

        case UVC_GET_LEN:
            resp->data[0] = 0x00;
            resp->data[1] = 0x22;
            resp->length = 2;
            break;

        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;
    }
    if (dev->bulk && cs == UVC_VS_COMMIT_CONTROL)
        uvc_handle_streamon_event(dev);
}

static void uvc_events_process_class(UVC_DEVICE_CTX_S *dev, struct usb_ctrlrequest *ctrl,
                                     struct uvc_request_data *resp) {
    if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE) return;

    switch (ctrl->wIndex & 0xff) {
        case UVC_INTF_CONTROL:
            uvc_events_process_control(dev, ctrl->bRequest, ctrl->wValue >> 8, ctrl->wIndex >> 8,
                                       ctrl->wLength, resp);
            break;

        case UVC_INTF_STREAMING:
            uvc_events_process_streaming(dev, ctrl->bRequest, ctrl->wValue >> 8, resp);
            break;

        default:
            break;
    }
}
static void uvc_events_process_setup(UVC_DEVICE_CTX_S *dev, struct usb_ctrlrequest *ctrl,
                                     struct uvc_request_data *resp) {
    dev->control = 0;

#ifdef ENABLE_USB_REQUEST_DEBUG
    printf(
        "\nbRequestType %02x bRequest %02x wValue %04x wIndex %04x "
        "wLength %04x\n",
        ctrl->bRequestType, ctrl->bRequest, ctrl->wValue, ctrl->wIndex, ctrl->wLength);
#endif
    switch (ctrl->bRequestType & USB_TYPE_MASK) {
        case USB_TYPE_STANDARD:
            uvc_events_process_standard(dev, ctrl, resp);
            break;

        case USB_TYPE_CLASS:
            uvc_events_process_class(dev, ctrl, resp);
            break;

        default:
            break;
    }
}

static int uvc_events_process_control_data(UVC_DEVICE_CTX_S *dev, uint8_t cs, uint8_t entity_id,
                                           struct uvc_request_data *data) {
    switch (entity_id) {
        /* Processing unit 'UVC_VC_PROCESSING_UNIT'. */
        case 2:
            switch (cs) {
                /*
                 * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
                 * Unit, as our bmControls[0] = 1 for PU.
                 */
                case UVC_PU_BRIGHTNESS_CONTROL:
                    memcpy(&dev->brightness_val, data->data, data->length);
                    /* UVC - V4L2 integrated path. */
                    if (!dev->run_standalone)
                        /*
                         * Try to change the Brightness attribute on
                         * Video capture device. Note that this try may
                         * succeed or end up with some error on the
                         * video capture side. By default to keep tools
                         * like USBCV's UVC test suite happy, we are
                         * maintaining a local copy of the current
                         * brightness value in 'dev->brightness_val'
                         * variable and we return the same value to the
                         * Host on receiving a GET_CUR(BRIGHTNESS)
                         * control request.
                         *
                         * FIXME: Keeping in view the point discussed
                         * above, notice that we ignore the return value
                         * from the function call below. To be strictly
                         * compliant, we should return the same value
                         * accordingly.
                         */
                        v4l2_set_ctrl(dev->vdev, dev->brightness_val, V4L2_CID_BRIGHTNESS);
                    break;

                default:
                    break;
            }

            break;

        default:
            break;
    }

    printf("Control Request data phase (cs %02x entity %02x)\n", cs, entity_id);

    return 0;
}

static int uvc_events_process_data(UVC_DEVICE_CTX_S *dev, struct uvc_request_data *data) {
    struct uvc_streaming_control *target;
    struct uvc_streaming_control *ctrl;
    const struct uvc_format_info *format;
    const struct uvc_frame_info *frame;
    const uint32_t *interval;
    uint32_t iformat, iframe;
    uint32_t nframes;
    uint32_t *val = (uint32_t *)data->data;
    int ret;

    switch (dev->control) {
        case UVC_VS_PROBE_CONTROL:
            printf("setting probe control, length = %d\n", data->length);
            target = &dev->probe;
            break;

        case UVC_VS_COMMIT_CONTROL:
            printf("setting commit control, length = %d\n", data->length);
            target = &dev->commit;
            break;

        default:
            printf("setting unknown control, length = %d\n", data->length);

            /*
             * As we support only BRIGHTNESS control, this request is
             * for setting BRIGHTNESS control.
             * Check for any invalid SET_CUR(BRIGHTNESS) requests
             * from Host. Note that we support Brightness levels
             * from 0x0 to 0x10 in a step of 0x1. So, any request
             * with value greater than 0x10 is invalid.
             */
            if (*val > PU_BRIGHTNESS_MAX_VAL) {
                return -EINVAL;
            } else {
                ret = uvc_events_process_control_data(dev, UVC_PU_BRIGHTNESS_CONTROL, 2, data);
                if (ret < 0) goto err;

                return 0;
            }
    }

    ctrl = (struct uvc_streaming_control *)&data->data;
    iformat = clamp((uint32_t)ctrl->bFormatIndex, 1U, (uint32_t)ARRAY_SIZE(uvc_formats));
    printf("[%s:%d] bFormatIndex:%d, iformat:%d\n", __FUNCTION__, __LINE__, ctrl->bFormatIndex, iformat);
    format = &uvc_formats[iformat - 1];

    nframes = 0;
    while (format->frames[nframes].width != 0) ++nframes;

    iframe = clamp((uint32_t)ctrl->bFrameIndex, 1U, nframes);
    printf("[%s:%d] bFrameIndex:%d, iframe:%d\n", __FUNCTION__, __LINE__, ctrl->bFrameIndex, iframe);
    frame = &format->frames[iframe - 1];
    interval = frame->intervals;

    while (interval[0] < ctrl->dwFrameInterval && interval[1]) ++interval;

    dev->width = frame->width;
    dev->height = frame->height;
    printf("width:%d height:%d\n", dev->width, dev->height);
    printf("format->fcc:%d, expect: %d\n", format->fcc, V4L2_PIX_FMT_H264);
    target->bFormatIndex = iformat;
    target->bFrameIndex = iframe;
    switch (format->fcc) {
        case V4L2_PIX_FMT_YUYV:
            // target->dwMaxVideoFrameSize = frame->width * frame->height * 2;
            target->dwMaxVideoFrameSize = MAX_BITSTREAM_BUFFER_SIZE;
            break;
        case V4L2_PIX_FMT_MJPEG:
            if (dev->imgsize == 0) printf("WARNING: MJPEG requested and no image loaded.\n");
            dev->imgsize = MJPG_PAYLOAD_SIZE;
            target->dwMaxVideoFrameSize = MAX_BITSTREAM_BUFFER_SIZE;
            break;
        case V4L2_PIX_FMT_H264:
            if (dev->imgsize == 0) printf("WARNING: H264 requested and no image loaded.\n");
            dev->imgsize = H264_PAYLOAD_SIZE;
            target->dwMaxVideoFrameSize = MAX_BITSTREAM_BUFFER_SIZE;
            break;
        case V4L2_PIX_FMT_HEVC:
            if (dev->imgsize == 0) printf("WARNING: H265 requested and no image loaded.\n");
            dev->imgsize = H265_PAYLOAD_SIZE;
            target->dwMaxVideoFrameSize = MAX_BITSTREAM_BUFFER_SIZE;
            break;
    }
    target->dwFrameInterval = *interval;

    if (dev->control == UVC_VS_COMMIT_CONTROL) {
        dev->fcc = format->fcc;
        dev->width = frame->width;
        dev->height = frame->height;
        dev->fps = 10000000 / target->dwFrameInterval;
        uvc_video_set_format(dev);
        UVC_VideoEnable(dev);
    }

    return 0;

err:
    return ret;
}

static void uvc_events_process(UVC_DEVICE_CTX_S *dev) {
    struct v4l2_event v4l2_event;
    struct uvc_event *uvc_event = (void *)&v4l2_event.u.data;
    struct uvc_request_data resp;
    int ret;

    ret = ioctl(dev->uvc_fd, VIDIOC_DQEVENT, &v4l2_event);
    if (ret < 0) {
        printf("VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno), errno);
        return;
    }

    memset(&resp, 0, sizeof resp);
    resp.length = -EL2HLT;

    switch (v4l2_event.type) {
        case UVC_EVENT_CONNECT:
            printf("[%s]: UVC_EVENT_CONNECT\n", __func__);
            return;

        case UVC_EVENT_DISCONNECT:
            dev->uvc_shutdown_requested = 1;
            printf("[%s]: UVC_EVENT_DISCONNECT\n", __func__);
            printf(
                "UVC: Possible USB shutdown requested from "
                "Host, seen via UVC_EVENT_DISCONNECT\n");
            // UVC_VideoDisable(dev);
            if (dev->is_streaming) {
                uvc_video_stream(dev, 0);
                uvc_uninit_device(dev);
                uvc_video_reqbufs(dev, 0);
                dev->is_streaming = 0;
                dev->first_buffer_queued = 0;
            }
            return;

        case UVC_EVENT_SETUP:
            uvc_events_process_setup(dev, &uvc_event->req, &resp);
            break;

        case UVC_EVENT_DATA:
            ret = uvc_events_process_data(dev, &uvc_event->data);
            if (ret < 0) break;
            return;

        case UVC_EVENT_STREAMON:
            if (!dev->bulk) {
                // clear_ok_queue();
                // clear_waited_node();
                uvc_handle_streamon_event(dev);
            }
            return;

        case UVC_EVENT_STREAMOFF:
            /* Stop V4L2 streaming... */
            if (!dev->run_standalone && dev->vdev->is_streaming) {
                /* UVC - V4L2 integrated path. */
                v4l2_stop_capturing(dev->vdev);
                clear_waited_node();
                dev->vdev->is_streaming = 0;
            }

            /* ... and now UVC streaming.. */
            if (dev->is_streaming) {
                uvc_video_stream(dev, 0);
                uvc_uninit_device(dev);
                uvc_video_reqbufs(dev, 0);
                dev->is_streaming = 0;
                dev->first_buffer_queued = 0;
            }

            return;
    }

    ret = ioctl(dev->uvc_fd, UVCIOC_SEND_RESPONSE, &resp);
    if (ret < 0) {
        printf("UVCIOC_S_EVENT failed: %s (%d)\n", strerror(errno), errno);
        return;
    }
}

static void uvc_events_init(UVC_DEVICE_CTX_S *dev) {
    struct v4l2_event_subscription sub;
    uint32_t payload_size = 0;

    switch (dev->fcc) {
        case V4L2_PIX_FMT_YUYV:
            // payload_size = dev->width * dev->height * 2;
            // break;
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_HEVC:
            payload_size = dev->imgsize;
            break;
    }

    uvc_fill_streaming_control(dev, &dev->probe, 0, 0);
    uvc_fill_streaming_control(dev, &dev->commit, 0, 0);

    if (dev->bulk) {
        /* FIXME Crude hack, must be negotiated with the driver. */
        dev->probe.dwMaxPayloadTransferSize = dev->commit.dwMaxPayloadTransferSize = payload_size;
    }

    memset(&sub, 0, sizeof sub);
    sub.type = UVC_EVENT_SETUP;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DATA;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMON;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMOFF;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_CONNECT;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DISCONNECT;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
}

static void uvc_close(UVC_DEVICE_CTX_S *dev) { close(dev->uvc_fd); }

int32_t UVC_GADGET_DeviceCheck(void) {
    fd_set fdsu;

    FD_ZERO(&fdsu);

    /* We want both setup and data events on UVC interface.. */
    FD_SET(s_stUVCDevCtx.uvc_fd, &fdsu);

    fd_set efds = fdsu;
    fd_set dfds = fdsu;

    int ret;
    ret = select(s_stUVCDevCtx.uvc_fd + 1, NULL, &dfds, &efds, NULL);
    if (-1 == ret) {
        printf("select error %d, %s\n", errno, strerror(errno));
        if (EINTR == errno) return ret;
    }

    if (0 == ret) {
        printf("select timeout\n");
        return ret;
    }

    if (FD_ISSET(s_stUVCDevCtx.uvc_fd, &efds)) {
        uvc_events_process(&s_stUVCDevCtx);
    }
    if (FD_ISSET(s_stUVCDevCtx.uvc_fd, &dfds)) {
        // struct timeval tTimeVal;
        // gettimeofday(&tTimeVal, NULL);
        // struct tm *tTM = localtime(&tTimeVal.tv_sec);
        // printf("%02d:%02d.%03ld.%03ld === start\n",
        //     tTM->tm_min, tTM->tm_sec,
        //     tTimeVal.tv_usec / 1000, tTimeVal.tv_usec % 1000);

        uvc_video_process(&s_stUVCDevCtx);
        // printf("%02d:%02d.%03ld.%03ld *** end\n",
        //     tTM->tm_min, tTM->tm_sec,
        //     tTimeVal.tv_usec / 1000, tTimeVal.tv_usec % 1000);
    }

    return ret;
}

int32_t UVC_GADGET_Init(const CVI_UVC_DEVICE_CAP_S *pstDevCaps, u_int32_t u32MaxFrameSize) {
    int bulk_mode = 0;

    /* Frame format/resolution related params. */
    int default_format = 0;     /* V4L2_PIX_FMT_YUYV */
    int default_resolution = 0; /* VGA 360p */
    int nbufs = WAITED_NODE_SIZE;              /* Ping-Pong buffers */
    /* USB speed related params */
    int mult = 2;
    int burst = 0;
    int maxp = 1024;

    enum usb_device_speed speed = USB_SPEED_SUPER; /* High-Speed */
    enum io_method uvc_io_method = IO_METHOD_USERPTR;

    /* Set parameters as passed by user. */
    s_stUVCDevCtx.width = (default_resolution == 0) ? 2560 : 1280;
    s_stUVCDevCtx.height = (default_resolution == 0) ? 1440 : 720;

    if (default_format == 0) {
        s_stUVCDevCtx.imgsize = s_stUVCDevCtx.width * s_stUVCDevCtx.height * 2;
        s_stUVCDevCtx.fcc = V4L2_PIX_FMT_YUYV;
    } else if (default_format == 1) {
        s_stUVCDevCtx.imgsize = s_stUVCDevCtx.width * s_stUVCDevCtx.height;
        s_stUVCDevCtx.fcc = V4L2_PIX_FMT_MJPEG;
    } else {
        s_stUVCDevCtx.imgsize = s_stUVCDevCtx.width * s_stUVCDevCtx.height;
        s_stUVCDevCtx.fcc = V4L2_PIX_FMT_H264;
    }

    s_stUVCDevCtx.io = uvc_io_method;
    s_stUVCDevCtx.bulk = bulk_mode;
    s_stUVCDevCtx.nbufs = nbufs;
    s_stUVCDevCtx.mult = mult;
    s_stUVCDevCtx.burst = burst;
    s_stUVCDevCtx.speed = speed;
    s_stUVCDevCtx.run_standalone = 1;

    switch (speed) {
        case USB_SPEED_FULL:
            /* Full Speed. */
            if (bulk_mode)
                s_stUVCDevCtx.maxpkt = 64;
            else
                s_stUVCDevCtx.maxpkt = 1023;
            break;

        case USB_SPEED_HIGH:
            /* High Speed. */
            if (bulk_mode)
                s_stUVCDevCtx.maxpkt = 512;
            else
                s_stUVCDevCtx.maxpkt = 1024;
            break;

        case USB_SPEED_SUPER:
        default:
            /* Super Speed. */
            if (bulk_mode)
                s_stUVCDevCtx.maxpkt = 1024;
            else
                s_stUVCDevCtx.maxpkt = 1024;
            break;
    }

    if (maxp) s_stUVCDevCtx.maxpkt = maxp;

    s_stUVCDevCtx.uvc_fd = -1;

    return 0;
}

int32_t UVC_GADGET_DeviceOpen(const char *pDevPath) {
    if (s_stUVCDevCtx.uvc_fd == -1) {
        if (UVC_DeviceOpen(pDevPath, &s_stUVCDevCtx)) {
            return -1;
        }
    }

    uvc_events_init(&s_stUVCDevCtx);

    return 0;
}

int32_t UVC_GADGET_DeviceClose(void) {
    if (s_stUVCDevCtx.is_streaming) {
        uvc_video_stream(&s_stUVCDevCtx, 0);
        printf("[%s] uvc_video_stream done\n", __FUNCTION__);
        uvc_uninit_device(&s_stUVCDevCtx);
        printf("[%s] uvc_uninit_device done\n", __FUNCTION__);
        uvc_video_reqbufs(&s_stUVCDevCtx, 0);
        printf("[%s] uvc_video_reqbufs done\n", __FUNCTION__);
        s_stUVCDevCtx.is_streaming = 0;
    }

    uvc_close(&s_stUVCDevCtx);
    printf("[%s] uvc_close done\n", __FUNCTION__);

    return 0;
}
