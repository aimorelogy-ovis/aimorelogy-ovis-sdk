#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "errno.h"
#include <sys/prctl.h>

#include "cvi_uvc.h"
#include "cvi_uvc_gadget.h"
#include "cvi_system.h"
#include "frame_cache.h"
#include "cvi_ae.h"
#include "cvi_venc.h"

#ifdef SUPPORT_AI_TRACK
#include "track.h"
#endif

#define     CVI_KOMOD_PATH              "/mnt/system/ko"
#define     CVI_UVC_SCRIPTS_PATH        "/etc"

#define     VI_FPS                      25
#define     SLOW_FPS                    15
#define     UVC_DIAG_FRAME_COUNT        120

static bool s_uvc_init = false;
/** UVC Stream Context */
typedef struct tagUVC_STREAM_CONTEXT_S {
    CVI_UVC_DEVICE_CAP_S stDeviceCap;
    CVI_UVC_DATA_SOURCE_S stDataSource;
    UVC_STREAM_ATTR_S stStreamAttr; /**<stream attribute, update by uvc driver */
    bool bVcapsStart;
    bool bVpssStart;
    bool bVencStart;
    bool bFirstFrame;
    bool bInited;
} UVC_STREAM_CONTEXT_S;
static UVC_STREAM_CONTEXT_S s_stUVCStreamCtx;

/** UVC Context */
static UVC_CONTEXT_S s_stUVCCtx = {.bRun = false, .bPCConnect = false, .TskId = (pthread_t)-1, .Tsk2Id = (pthread_t)-1};
static bool g_bPushVencData = false;
static pthread_mutex_t g_stUVCStreamMutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned int uvc_debug_checksum(const unsigned char *data, size_t length)
{
    unsigned int checksum = 2166136261U;

    for (size_t i = 0; i < length; ++i) {
        checksum ^= data[i];
        checksum *= 16777619U;
    }

    return checksum;
}

static bool uvc_debug_jpeg_valid(const unsigned char *data, size_t length)
{
    size_t start;

    if (length < 4 || data[0] != 0xff || data[1] != 0xd8) {
        return false;
    }

    start = length > 64 ? length - 64 : 2;
    for (size_t i = length - 1; i > start; --i) {
        if (data[i - 1] == 0xff && data[i] == 0xd9) {
            return true;
        }
    }

    return false;
}

static void uvc_debug_dump_frame(const frame_node_t *node)
{
    char path[64];
    FILE *file;

    snprintf(path, sizeof(path), "/tmp/uvc-diag-%u.jpg",
        node->debug_sequence % UVC_DIAG_FRAME_COUNT);
    file = fopen(path, "wb");
    if (file == NULL) {
        printf("UVC DIAG unable to open %s: %s\n", path, strerror(errno));
        return;
    }

    if (fwrite(node->mem, 1, node->used, file) != node->used) {
        printf("UVC DIAG unable to write %s: %s\n", path, strerror(errno));
    }
    fclose(file);
}

void cvi_uvc_stream_set_enabled(bool enabled)
{
    pthread_mutex_lock(&g_stUVCStreamMutex);
    g_bPushVencData = enabled && s_stUVCCtx.bRun;
    if (!g_bPushVencData) {
        clear_ok_queue();
    }
    pthread_mutex_unlock(&g_stUVCStreamMutex);
}

/* The caller holds g_stUVCStreamMutex until the cache node is returned. */
static int cvi_uvc_stream_copy_data(void *data)
{
    static unsigned long long invalid_jpeg_count;
    CVI_U32 i = 0;
    VENC_PACK_S *pstData = CVI_NULL;
    unsigned char *s = CVI_NULL;
    unsigned int data_len = 0;
    size_t frame_size = 0;
    VENC_STREAM_S * pstStream = (VENC_STREAM_S *) data;

    if ((pstStream == CVI_NULL) || (pstStream->pstPack == CVI_NULL) ||
        (pstStream->u32PackCount == 0)) {
        return CVI_SUCCESS;
    }

    for (i = 0; i < pstStream->u32PackCount; ++i) {
        pstData = &pstStream->pstPack[i];
        if (pstData->u32Offset > pstData->u32Len) {
            printf("invalid UVC VENC pack, offset=%u len=%u\n",
                pstData->u32Offset, pstData->u32Len);
            return CVI_SUCCESS;
        }
        frame_size += pstData->u32Len - pstData->u32Offset;
    }

    if ((frame_size == 0) || (frame_size > CACHE_MEM_SIZE)) {
        printf("drop UVC frame, size=%zu max=%u\n", frame_size,
            (unsigned int)CACHE_MEM_SIZE);
        return CVI_SUCCESS;
    }
#ifdef VENC_SAVE_FILE
    static int first_time = 0;
    static int frame_cnt = 0;
    static unsigned char* stream_buf;
    static int buf_len = 0;
    static FILE* pFile = NULL; 
    if(first_time == 0){
        first_time = 1;
        char outputFileName[256] = {0};
        snprintf(outputFileName, 256, "venc.%s", "h264");
        pFile = fopen(outputFileName, "wb");
        if (pFile == NULL) {
            printf("open file err, %s\n", outputFileName);
            return -1;
        }

        stream_buf = (unsigned char*)malloc(2 * 1024 * 1024);
        if(stream_buf == NULL){
            printf("alloc stream_buf is err\n");
            return -1;
        }
        printf("venc create_stream_file done\n");
    }

    for (i = 0; i < pstStream->u32PackCount; ++i)
    {
        printf("frame_cnt = %d\n", frame_cnt);
        printf("buf_len = %d\n", buf_len);
        if(buf_len < 1 * 1024 * 1024){
            VENC_PACK_S *ppack;
            ppack = &pstStream->pstPack[i];
            memcpy(stream_buf + buf_len, ppack->pu8Addr + ppack->u32Offset, ppack->u32Len - ppack->u32Offset);
            buf_len += ppack->u32Len - ppack->u32Offset;
            // fwrite(ppack->pu8Addr + ppack->u32Offset,
            //     ppack->u32Len - ppack->u32Offset, 1, pFile);
        }
        else{
            if(pFile){
                fwrite(stream_buf, buf_len, 1, pFile);
                fclose(pFile);
                printf("close venc file\n");
                free(stream_buf);
                pFile = NULL;
            }
        }
    }
    frame_cnt++;
#endif

    uvc_cache_t *uvc_cache = uvc_cache_get();
    frame_node_t *fnode = CVI_NULL;

    if (uvc_cache)
    {
        // printf("ok_queue:\n");
        // debug_dump_queue(uvc_cache->ok_queue);
        get_node_from_queue(uvc_cache->free_queue, &fnode);
    }

    if (!uvc_cache) {
        return CVI_SUCCESS;
    }

    if (!fnode)
    {
        /* Do not copy frames faster than USB can consume them. The pending
         * queue is already bounded and drained to the newest frame by the
         * consumer, so copying another full MJPEG frame here only burns CPU. */
        return CVI_SUCCESS;
    }

    fnode->used = 0;
    // printf("pstStream->u32PackCount = %d\n", pstStream->u32PackCount);
    for (i = 0; i < pstStream->u32PackCount; ++i)
    {
        pstData = &pstStream->pstPack[i];
        s = pstData->pu8Addr + pstData->u32Offset;
        data_len = pstData->u32Len - pstData->u32Offset;
        memcpy(fnode->mem + fnode->used, s, data_len);
        fnode->used += data_len;
    }

    if (!uvc_debug_jpeg_valid(fnode->mem, fnode->used)) {
        invalid_jpeg_count++;
        if (invalid_jpeg_count == 1 || (invalid_jpeg_count % 30) == 0) {
            printf("UVC: drop invalid MJPEG frame seq=%u size=%u total=%llu.\n",
                pstStream->u32Seq, fnode->used, invalid_jpeg_count);
        }
        fnode->used = 0;
        put_node_to_queue(uvc_cache->free_queue, fnode);
        return CVI_SUCCESS;
    }

    if (access("/tmp/uvc-diag", F_OK) == 0) {
        fnode->debug_sequence = pstStream->u32Seq;
        fnode->debug_checksum = uvc_debug_checksum(fnode->mem, fnode->used);
        fnode->debug_jpeg_valid = 1;

        if (!fnode->debug_jpeg_valid || (pstStream->u32Seq % 30) == 0) {
            printf("UVC DIAG producer seq=%u size=%u packs=%u jpeg=%u checksum=%08x\n",
                fnode->debug_sequence, fnode->used, pstStream->u32PackCount,
                fnode->debug_jpeg_valid, fnode->debug_checksum);
        }
        uvc_debug_dump_frame(fnode);
    } else {
        fnode->debug_checksum = 0;
        fnode->debug_sequence = pstStream->u32Seq;
        fnode->debug_jpeg_valid = 0;
    }
    // printf("fnode->used = %d\n", fnode->used);

    if (put_node_to_queue(uvc_cache->ok_queue, fnode) != 0) {
        fnode->used = 0;
        put_node_to_queue(uvc_cache->free_queue, fnode);
    }

    return CVI_SUCCESS;
}

int cvi_uvc_stream_send_data(void *data)
{
    int ret = CVI_SUCCESS;

    /* Stream-off waits for an in-flight copy before releasing the cache. */
    pthread_mutex_lock(&g_stUVCStreamMutex);
    if (g_bPushVencData) {
        ret = cvi_uvc_stream_copy_data(data);
    }
    pthread_mutex_unlock(&g_stUVCStreamMutex);

    return ret;
}

int32_t UVC_STREAM_ReqIDR(void) {
    // TODO: implement
    return 0;
}



static void *UVC_CheckTask(void *pvArg) {
    int32_t ret = 0;
    prctl(PR_SET_NAME, "cvitask_uvc", 0, 0, 0);
    while (s_stUVCCtx.bRun) {
        ret = UVC_GADGET_DeviceCheck();

        if (ret < 0) {
            printf("UVC_GADGET_DeviceCheck %x\n", ret);
            cvi_uvc_stream_set_enabled(false);
            break;
        }
        //usleep(50 * 1000);
    }


    return NULL;
}
#if 0
static int32_t UVC_LoadMod(void) {
    static bool first = true;
    if(first == false) {
        return 0;
    }
    first = false;
    printf("Uvc insmod ko successfully!");
    // cvi_insmod(CVI_KOMOD_PATH"/videobuf2-memops.ko", NULL);
    cvi_system("echo 449 >/sys/class/gpio/export");
    cvi_system("echo 450 >/sys/class/gpio/export");

    cvi_system("echo \"out\" >/sys/class/gpio/gpio449/direction");
    cvi_system("echo \"out\" >/sys/class/gpio/gpio450/direction");

    cvi_system("echo 0 >/sys/class/gpio/gpio449/value");
    cvi_system("echo 1 >/sys/class/gpio/gpio450/value");

    cvi_insmod(CVI_KOMOD_PATH"/usbcore.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/dwc2.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/configfs.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/libcomposite.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/videobuf2-vmalloc.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/usb_f_uvc.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/u_audio.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/usb_f_uac1.ko", NULL);
    cvi_system("echo device > /proc/cviusb/otg_role");
    cvi_system(CVI_UVC_SCRIPTS_PATH"/run_usb.sh probe uvc");
    cvi_system(CVI_UVC_SCRIPTS_PATH"/ConfigUVC.sh");
    cvi_system(CVI_UVC_SCRIPTS_PATH"/run_usb.sh start");
    cvi_system("devmem 0x030001DC 32 0x8844");
    return 0;
}
#endif

int32_t UVC_Init(const CVI_UVC_DEVICE_CAP_S *pstCap, const CVI_UVC_DATA_SOURCE_S *pstDataSrc,
                 CVI_UVC_BUFFER_CFG_S *pstBufferCfg) {

    // UVC_LoadMod();

    s_stUVCStreamCtx.stDeviceCap = *pstCap;
    s_stUVCStreamCtx.stDataSource = *pstDataSrc;
    UVC_GADGET_Init(pstCap, pstBufferCfg->u32BufSize);

    // TODO: Do we need handle CVI_UVC_BUFFER_CFG_S?
    return 0;
}

int32_t UVC_Deinit(void) {
    // UVC_UnLoadMod(); // TODO, Not work right now

    return 0;
}

int32_t UVC_Start(const char *pDevPath) {

    if (false == s_stUVCCtx.bRun) {
        strcpy(s_stUVCCtx.szDevPath, pDevPath);

        if (UVC_GADGET_DeviceOpen(pDevPath)) {
            printf("UVC_GADGET_DeviceOpen Failed!");
            return -1;
        }

        s_stUVCCtx.bPCConnect = false;
        s_stUVCCtx.bRun = true;

        // pthread_attr_t pthread_attr;
        // pthread_attr_init(&pthread_attr);
        // struct sched_param param;
        // param.sched_priority = 90;
        // pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
        // pthread_attr_setschedparam(&pthread_attr, &param);
        // pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
        // int policy;

        if (pthread_create(&s_stUVCCtx.TskId, NULL, UVC_CheckTask, NULL)) {
            printf("UVC_CheckTask create thread failed!\n");
            s_stUVCCtx.bRun = false;
            return -1;
        }

        if (UVC_GADGET_DeviceConnect() != 0) {
            printf("UVC_GADGET_DeviceConnect Failed!\n");
            s_stUVCCtx.bRun = false;
            pthread_cancel(s_stUVCCtx.TskId);
            pthread_join(s_stUVCCtx.TskId, NULL);
            UVC_GADGET_DeviceClose();
            return -1;
        }

        // usleep(100*1000);
        // //获取当前线程的调度策略和参数
        // if(pthread_getschedparam(s_stUVCCtx.TskId, &policy, &param) != 0){
        //     printf("pthread_getschedparam failed!\n");
        //     return -1;
        // }
        // //修改优先级
        // param.sched_priority = 95;
        // printf("param.sched_priority:%d\n", param.sched_priority);
        // //设置新的调度策略和参数
        // if(pthread_setschedparam(s_stUVCCtx.TskId, policy, &param) != 0){
        //     printf("pthread_setschedparam failed!\n");
        //     return -1;
        // }

        // if (pthread_create(&s_stUVCCtx.TskId, NULL, UVC_CheckTask, NULL)) {
        //     printf("UVC_CheckTask create thread failed!\n");
        //     s_stUVCCtx.bRun = false;
        //     return -1;
        // }
        printf("UVC_CheckTask create thread successful\n");
    } else {
        printf("UVC already started\n");
    }

    return 0;
}

int32_t UVC_Stop(void) {
    int join_ret;

    if (false == s_stUVCCtx.bRun) {
        printf("UVC not run\n");
        return 0;
    }

    s_stUVCCtx.bRun = false;
    cvi_uvc_stream_set_enabled(false);
    printf("UVC: waiting for event thread to stop.\n");
    join_ret = pthread_join(s_stUVCCtx.TskId, NULL);
    if (join_ret != 0) {
        printf("UVC: event thread stop failed: %s (%d).\n",
            strerror(join_ret), join_ret);
        return -1;
    }
    s_stUVCCtx.TskId = (pthread_t)-1;
    printf("UVC: event thread stopped.\n");

    return UVC_GADGET_DeviceClose();
}

UVC_CONTEXT_S *UVC_GetCtx(void) { return &s_stUVCCtx; }

void app_uvc_exit(void)
{
    if (!s_uvc_init) {
        printf("uvc not init\n");
        return;
    }

    if (UVC_Stop() != 0) {
        printf("UVC_Stop Failed !");
        return;
    }
    if (UVC_Deinit() != 0) {
        printf("UVC_Deinit Failed !");
    }
    destroy_uvc_cache();
    s_uvc_init = false;
}

int app_uvc_init(void)
{
    char uvc_devname[32] = "/dev/video0";
    if(access(uvc_devname, F_OK) != 0){
		printf("file %s not found\n", uvc_devname);
		return -1;
	}

	CVI_UVC_DEVICE_CAP_S stDeviceCap = {0};
    CVI_UVC_DATA_SOURCE_S stDataSource = {0};
    CVI_UVC_BUFFER_CFG_S stBuffer = {0};
    stDataSource.AcapHdl = 0;
    stDataSource.VcapHdl = 0;
    stDataSource.VencHdl = 0;
    stDataSource.VprocChnId = 0;
    stDataSource.VprocHdl = 0;

    create_uvc_cache();

    if (UVC_Init(&stDeviceCap, &stDataSource, &stBuffer) != 0) {
        printf("UVC_Init Failed !");
        goto failed;
    }

    if (UVC_Start(uvc_devname) != 0) {
        printf("UVC_Start Failed !");
        goto failed;
    }

    s_uvc_init = true;
	return 0;
failed:
	return -1;
}
