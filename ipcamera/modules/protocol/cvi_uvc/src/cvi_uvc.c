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
bool g_bPushVencData = false;

int cvi_uvc_stream_send_data(void *data)
{
    CVI_U32 i = 0;
    VENC_PACK_S *pstData = CVI_NULL;
    unsigned char *s = CVI_NULL;
    unsigned int data_len = 0;
    unsigned int copy_size = 0;
    VENC_STREAM_S * pstStream = (VENC_STREAM_S *) data;

    if(false == s_stUVCCtx.bRun) {
        return -1;
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

    if (!fnode)
    {
        //printf("free queue null\n");

        // printf("free_queue:\n");
        // debug_dump_queue(uvc_cache->free_queue);
        // printf("ok_queue:\n");
        // debug_dump_queue(uvc_cache->ok_queue);
        // printf("free_queue:\n");
        // debug_dump_queue(uvc_cache->free_queue);

        // get_node_from_queue(uvc_cache->ok_queue, &fnode);

        return CVI_SUCCESS;
    }

    fnode->used = 0;
    // printf("pstStream->u32PackCount = %d\n", pstStream->u32PackCount);
    for (i = 0; i < pstStream->u32PackCount; ++i)
    {
        pstData = &pstStream->pstPack[i];
        s = pstData->pu8Addr + pstData->u32Offset;
        data_len = pstData->u32Len - pstData->u32Offset;
        if(data_len < (fnode->length - fnode->used)){
            copy_size = data_len;
        }else{
            printf("data_len=%d, (fnode->length - fnode->used)=%d\n", data_len, (fnode->length - fnode->used));
            copy_size = (fnode->length - fnode->used);
        }
        // copy_size = data_len < (fnode->length - fnode->used) ? data_len : (fnode->length - fnode->used);

        if (copy_size > 0)
        {
            memcpy(fnode->mem + fnode->used, s, copy_size);
            fnode->used += copy_size;
        }
    }
    // printf("fnode->used = %d\n", fnode->used);

    put_node_to_queue(uvc_cache->ok_queue, fnode);

    return CVI_SUCCESS;
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
            break;
        } else if (ret == 0) {
            printf("Timeout Do Nothing\n");
            if (false != g_bPushVencData) {
                g_bPushVencData = false;
            }
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
    if (false == s_stUVCCtx.bRun) {
        printf("UVC not run\n");
        return 0;
    }

    s_stUVCCtx.bRun = false;
    pthread_join(s_stUVCCtx.TskId, NULL);

    return UVC_GADGET_DeviceClose();
}

UVC_CONTEXT_S *UVC_GetCtx(void) { return &s_stUVCCtx; }

void app_uvc_exit(void)
{
    if(!s_uvc_init){
        printf("uvc not init\n");
        return;
    }

	if (UVC_Stop() != 0) {
		printf("UVC_Stop Failed !");
	}
	if (UVC_Deinit() != 0) {
		printf("UVC_Deinit Failed !");
	}
    destroy_uvc_cache();
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