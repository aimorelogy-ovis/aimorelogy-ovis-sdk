#!/bin/sh
${CVI_SHOPTS}
#
# Start to insert kernel modules
#
insmod /mnt/system/ko/cv180x_sys.ko
#insmod /mnt/system/ko/cv180x_base.ko
insmod /mnt/system/ko/cv180x_clock_cooling.ko
insmod /mnt/system/ko/cv180x_tpu.ko
insmod /mnt/system/ko/cvi_ipcm.ko

echo 3 > /proc/sys/vm/drop_caches

exit $?
