// #include "cvi_sys.h"
#include "ive.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

int cpu_ref(const int channels, IVE_SRC_IMAGE_S *src1, IVE_SRC_IMAGE_S *src2, IVE_DST_IMAGE_S *dst);

int main(int argc, char **argv) {
  if (argc != 4) {
    printf("Incorrect loop value. Usage: %s <file_name1> <file_name2> <loop in value (1-1000)>\n",
           argv[0]);
    return CVI_FAILURE;
  }

  // Create instance
  // CVI_SYS_Init();
  IVE_HANDLE handle = CVI_IVE_CreateHandle();
  printf("BM Kernel init.\n");

  const char *file_name1 = argv[1];  //"/mnt/data/admin1_data/alios_test/a.jpg";
  const char *file_name2 = argv[2];  //"/mnt/data/admin1_data/alios_test/b.jpg";//argv[2];
  int total_run = atoi(argv[3]);

  // Read image from file. CVI_IVE_ReadImage will do the flush for you.
  IVE_IMAGE_TYPE_E img_type =
      IVE_IMAGE_TYPE_U8C1;  // IVE_IMAGE_TYPE_S8C3_PLANAR;//IVE_IMAGE_TYPE_S8C1
  IVE_IMAGE_S src1 = CVI_IVE_ReadImage(handle, file_name1, img_type);
  IVE_IMAGE_S src2 = CVI_IVE_ReadImage(handle, file_name2, img_type);
  int nChannels = 1;
  int width = src1.u16Width;
  int height = src1.u16Height;
  IVE_DST_IMAGE_S dst;
  CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, width, height);

  IVE_SUB_CTRL_S iveSubCtrl;
  iveSubCtrl.enMode = IVE_SUB_MODE_ABS_CLIP;  // IVE_SUB_MODE_NORMAL;//IVE_SUB_MODE_ABS;
  printf("Run TPU Sub,mode:%d,loopnum:%d\n", iveSubCtrl.enMode, total_run);
  struct timeval t0, t1;
  gettimeofday(&t0, NULL);
  int ret = CVI_SUCCESS;
  for (int i = 0; i < total_run; i++) {
    ret = CVI_IVE_Sub(handle, &src1, &src2, &dst, &iveSubCtrl, 0);
  }
  gettimeofday(&t1, NULL);
  unsigned long elapsed_tpu =
      ((t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec) / total_run;
  if (ret != CVI_SUCCESS) {
    printf("cvi sub failed\n");
    return ret;
  }
  CVI_IVE_BufRequest(handle, &src1);
  CVI_IVE_BufRequest(handle, &src2);
  CVI_IVE_BufRequest(handle, &dst);
  ret = cpu_ref(nChannels, &src1, &src2, &dst);

  if (ret == CVI_SUCCESS) {
    printf("equality check ok\n");
  } else {
    printf("equality check fail\n");
  }
  printf("TPU avg time %lu\n", elapsed_tpu);

  // Free memory, instance
  CVI_SYS_FreeI(handle, &src1);
  CVI_SYS_FreeI(handle, &src2);
  CVI_SYS_FreeI(handle, &dst);
  CVI_IVE_DestroyHandle(handle);
  // CVI_SYS_Exit();

  return ret;
}

int cpu_ref(const int channels, IVE_SRC_IMAGE_S *src1, IVE_SRC_IMAGE_S *src2,
            IVE_DST_IMAGE_S *dst) {
  int ret = CVI_SUCCESS;
  CVI_U8 *src1_ptr = src1->pu8VirAddr[0];
  CVI_U8 *src2_ptr = src2->pu8VirAddr[0];
  CVI_U8 *dst_ptr = dst->pu8VirAddr[0];
  for (size_t i = 0; i < channels * src1->u16Width * src1->u16Height; i++) {
    int res = abs((int)src1_ptr[i] - (int)src2_ptr[i]);
    if (res > 128) res = 128;
    if (res != dst_ptr[i]) {
      printf("[%zu] %d - %d = TPU %d, CPU %d\n", i, src1_ptr[i], src2_ptr[i], dst_ptr[i], (int)res);
      ret = CVI_FAILURE;
      break;
    }
  }
  return ret;
}
